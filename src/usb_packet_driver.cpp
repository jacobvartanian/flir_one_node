#include <boost/format.hpp>
#include "usb_packet_driver.h"

namespace usb_packet_driver
{
  UsbPacketDriver::UsbPacketDriver(int vendor_id, int product_id):
    is_ok_(true),
    states_(INIT),
    context_(NULL),
    vendor_id_(vendor_id),
    product_id_(product_id)
  {
  }

  UsbPacketDriver::~UsbPacketDriver()
  {
  }

  void UsbPacketDriver::shutdown()
  {
    ROS_INFO("Shutting down");
    if (devh_)
    {
      libusb_reset_device(devh_);
      libusb_close(devh_);
      ROS_INFO("Device closed");
    }
    libusb_exit(NULL);
    is_ok_ = false;
  }

  bool UsbPacketDriver::ok(void)
  {
    return is_ok_;
  }

  bool UsbPacketDriver::get_next_packet(std::vector<unsigned char> &packet)
  {
    packet_queue_.mtx.lock();
    if (!packet_queue_.data.empty())
    {
      buffer next_data = packet_queue_.data.front();
      unsigned int data_array_size = sizeof(next_data.data);
      packet.insert(packet.end(), &next_data.data[0], &next_data.data[data_array_size]);
      packet_queue_.data.pop();
      packet_queue_.mtx.unlock();
      return true;
    }
    packet_queue_.mtx.unlock();
    return false;
  }

  void UsbPacketDriver::print_bulk_result(const char ep[],char EP_error[], int r, int actual_length, unsigned char buf[])
  {
    time_t now1;
    int i;

    now1 = time(NULL);
    if (r < 0)
    {
      if (strcmp(EP_error, libusb_error_name(r))!=0)
      {
        strcpy(EP_error, libusb_error_name(r));
        fprintf(stderr, "\n: %s >>>>>>>>>>>>>>>>>bulk transfer (in) %s: %s\n", ctime(&now1), ep , libusb_error_name(r));
        sleep(1);
      }
      //return 1;
    } else
    {
      ROS_INFO("\n: %s bulk read EP %s, actual length %d\nHEX:\n",ctime(&now1), ep ,actual_length);
    }
  }

  void UsbPacketDriver::read(const char ep[],char EP_error[], int r, int actual_length, unsigned char buf[])
  {
    // reset buffer if the new chunk begins with magic bytes or the buffer size limit is exceeded
    if  ((strncmp (( const char *)buf, ( const char *)magic_byte_,4)==0 ) || ((usb_buffer_.pointer + actual_length) >= BUF85SIZE))
    {
      usb_buffer_.pointer = 0;
    }

    memmove(usb_buffer_.data+usb_buffer_.pointer, buf, actual_length);
    usb_buffer_.pointer=usb_buffer_.pointer+actual_length;

    if  ((strncmp((const char *) usb_buffer_.data, (const char *) magic_byte_,4)!=0))
    {
      usb_buffer_.pointer = 0;
      ROS_DEBUG("Reset buffer because of bad Magic Byte!");
      return;
    }

    uint32_t FrameSize = usb_buffer_.data[8] + (usb_buffer_.data[9] << 8) + (usb_buffer_.data[10] << 16) + (usb_buffer_.data[11] << 24);

    if ((FrameSize+28) > (usb_buffer_.pointer))
    {
      // wait for next chunk
      ROS_DEBUG("wait for next chunk");
      return;
    }
    packet_queue_.mtx.lock();
    packet_queue_.data.push(usb_buffer_);
    packet_queue_.mtx.unlock();
  }

  int UsbPacketDriver::poll_data(void)
  {
 	  unsigned char data[2]={0,0}; // dummy data
    int r;

    switch (states_)
    {
      /* Flir config
      01 0b 01 00 01 00 00 00 c4 d5
      0 bmRequestType = 01
      1 bRequest = 0b
      2 wValue 0001 type (H) index (L)    stop=0/start=1 (Alternate Setting)
      4 wIndex 01                         interface 1/2
      5 wLength 00
      6 Data 00 00

      libusb_control_transfer (*dev_handle, bmRequestType, bRequest, wValue,  wIndex, *data, wLength, timeout)
      */

      case INIT:
        ROS_INFO("stop interface 2 FRAME\n");
        r = libusb_control_transfer(devh_,1,0x0b,0,2,data,0,100);
        if (r < 0)
        {
          ROS_ERROR("Control Out error %d\n", r);
          error_code_ = r;
          states_ = ERROR;
        } else
        {
          states_ = INIT_1;
        }
        break;

      case INIT_1:
        ROS_INFO("stop interface 1 FILEIO\n");
        r = libusb_control_transfer(devh_,1,0x0b,0,1,data,0,100);
        if (r < 0)
        {
          ROS_ERROR("Control Out error %d\n", r);
          error_code_ = r;
          states_ = ERROR;
        } else
        {
          states_ = INIT_2;
        }
        break;

      case INIT_2:
        ROS_INFO("\nstart interface 1 FILEIO\n");
        r = libusb_control_transfer(devh_,1,0x0b,1,1,data,0,100);
        if (r < 0)
        {
          ROS_ERROR("Control Out error %d\n", r);
          error_code_ = r;
          states_ = ERROR;
        } else
        {
          states_ = ASK_ZIP;
        }
        break;

      case ASK_ZIP:
      {
        ROS_INFO("\nask for CameraFiles.zip on EP 0x83:\n");

        int transferred = 0;
        char my_string[128];

        //--------- write string: {"type":"openFile","data":{"mode":"r","path":"CameraFiles.zip"}}
        int length = 16;
        unsigned char my_string2[16]={0xcc,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x41,0x00,0x00,0x00,0xF8,0xB3,0xF7,0x00};
        ROS_INFO("\nEP 0x02 to be sent Hexcode: %i Bytes[",length);
        int i;
        for (i = 0; i < length; i++)
        {
          ROS_INFO(" %02x", my_string2[i]);
        }
        ROS_INFO(" ]\n");

        r = libusb_bulk_transfer(devh_, 2, my_string2, length, &transferred, 0);
        if(r == 0 && transferred == length) {
          ROS_INFO("\nWrite successful!");
        } else
        {
          ROS_ERROR("\nError in write! res = %d and transferred = %d\n", r, transferred);
          states_ = ERROR;
          break;
        }

        strcpy(my_string,"{\"type\":\"openFile\",\"data\":{\"mode\":\"r\",\"path\":\"CameraFiles.zip\"}}");

        length = strlen(my_string)+1;
        ROS_INFO("\nEP 0x02 to be sent: %s", my_string);

        // avoid error: invalid conversion from ‘char*’ to ‘unsigned char*’ [-fpermissive]
        unsigned char *my_string1 = (unsigned char*)my_string;
        //my_string1 = (unsigned char*)my_string;

        r = libusb_bulk_transfer(devh_, 2, my_string1, length, &transferred, 0);
        if(r == 0 && transferred == length)
        {
          ROS_INFO("\nWrite successful!");
          ROS_INFO("\nSent %d bytes with string: %s\n", transferred, my_string);
        } else
        {
          ROS_ERROR("\nError in write! res = %d and transferred = %d\n", r, transferred);
          states_ = ERROR;
          break;
        }

        //--------- write string: {"type":"readFile","data":{"streamIdentifier":10}}
        length = 16;
        unsigned char my_string3[16]={0xcc,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x33,0x00,0x00,0x00,0xef,0xdb,0xc1,0xc1};
        ROS_INFO("\nEP 0x02 to be sent Hexcode: %i Bytes[",length);
        for (i = 0; i < length; i++)
        {
          ROS_INFO(" %02x", my_string3[i]);
        }
        ROS_INFO(" ]\n");

        r = libusb_bulk_transfer(devh_, 2, my_string3, length, &transferred, 0);
        if(r == 0 && transferred == length)
        {
          ROS_INFO("\nWrite successful!");
        } else
        {
          ROS_ERROR("\nError in write! res = %d and transferred = %d\n", r, transferred);
          states_ = ERROR;
          break;
        }

        //strcpy(  my_string, "{\"type\":\"setOption\",\"data\":{\"option\":\"autoFFC\",\"value\":true}}");
        strcpy(my_string,"{\"type\":\"readFile\",\"data\":{\"streamIdentifier\":10}}");
        length = strlen(my_string)+1;
        ROS_INFO("\nEP 0x02 to be sent %i Bytes: %s", length, my_string);

        // avoid error: invalid conversion from ‘char*’ to ‘unsigned char*’ [-fpermissive]
        my_string1 = (unsigned char*)my_string;

        r = libusb_bulk_transfer(devh_, 2, my_string1, length, &transferred, 0);
        if(r == 0 && transferred == length)
        {
          ROS_INFO("\nWrite successful!");
          ROS_INFO("\nSent %d bytes with string: %s\n", transferred, my_string);
        } else
        {
          ROS_ERROR("\nError in write! res = %d and transferred = %d\n", r, transferred);
          states_ = ERROR;
          break;
        }

        // go to next state
        states_ = ASK_VIDEO;
      }
      break;

      case ASK_VIDEO:
        ROS_INFO("\nAsk for video stream, start EP 0x85:\n");

        r = libusb_control_transfer(devh_,1,0x0b,1,2,data, 2,200);
        if (r < 0)
        {
          ROS_ERROR("Control Out error %d\n", r);
          error_code_ = r;
          states_ = ERROR;
        } else
        {
          states_ = POOL_FRAME;
        }
      break;

      case POOL_FRAME:
        // endless loop
        // poll Frame Endpoints 0x85
        // don't change timeout=100ms
        r = libusb_bulk_transfer(devh_, 0x85, usb_buffer_.data, sizeof(usb_buffer_.data), &actual_length_, 200);
        switch(r){
          case LIBUSB_ERROR_TIMEOUT:
            ROS_ERROR("LIBUSB_ERROR_TIMEOUT");
            break;
          case LIBUSB_ERROR_PIPE:
            ROS_ERROR("LIBUSB_ERROR_PIPE");
            break;
          case LIBUSB_ERROR_OVERFLOW:
            ROS_ERROR("LIBUSB_ERROR_OVERFLOW");
            break;
          case LIBUSB_ERROR_NO_DEVICE:
            ROS_ERROR("LIBUSB_ERROR_NO_DEVICE");
            states_ = ERROR;
            break;
        }
        if (actual_length_ > 0)
        {
          ROS_DEBUG("Actual frame length %d", actual_length_);
          read("0x85",EP85_error_, r, actual_length_, usb_buffer_.data);
        }
        break;

      case ERROR:
        is_ok_ = false;
        break;
    }

    // poll Endpoints 0x81, 0x83
    r = libusb_bulk_transfer(devh_, 0x81, usb_buffer_.data, sizeof(usb_buffer_.data), &actual_length_, 10);
    print_bulk_result("0x81",EP81_error_, r, actual_length_, usb_buffer_.data);

    r = libusb_bulk_transfer(devh_, 0x83, usb_buffer_.data, sizeof(usb_buffer_.data), &actual_length_, 10);
    print_bulk_result("0x83",EP83_error_, r, actual_length_, usb_buffer_.data);
  }

  void UsbPacketDriver::usb_setup(const unsigned char * magic_byte)
  {
    magic_byte_ = magic_byte;
    int r;
    states_ = INIT;
    setup_states_t setup_states = SETUP_INIT;
    while ((setup_states != SETUP_ERROR) && (setup_states != SETUP_ALL_OK))
    {
      switch (setup_states)
      {
        case SETUP_INIT:
          if (libusb_init(&context_) < 0)
          {
            ROS_ERROR("Failed to initialise libusb");
            setup_states = SETUP_ERROR;
          } else
          {
            ROS_INFO("Successfully initialised libusb");
            setup_states = SETUP_FIND;
            setup_states = SETUP_LISTING;
          }
          break;

        case SETUP_LISTING:
          {
            int r = 0;
            libusb_device_handle *dev_handle = NULL   ;
            libusb_device        **devs               ;
            int count = libusb_get_device_list(context_, &devs);

            for (size_t idx = 0; idx < count; ++idx)
            {
              libusb_device *device = devs[idx];
              libusb_device_descriptor desc = {0};

              r = libusb_get_device_descriptor(device, &desc);
              assert(r == 0);

              ROS_DEBUG("Vendor:Device = %04x:%04x", desc.idVendor, desc.idProduct);
            }
            libusb_free_device_list(devs, 1); //free the list, unref the devices in it
            setup_states = SETUP_FIND;
          }
          break;

        case SETUP_FIND:
          devh_ = libusb_open_device_with_vid_pid(context_, vendor_id_, product_id_);
          if (devh_ == NULL)
          {
            ROS_ERROR_STREAM("Could not find/open device. devh : " << devh_);
            setup_states = SETUP_ERROR;
          } else
          {
            ROS_INFO("Successfully find the Flir One G2 device");
            setup_states = SETUP_SET_CONF;
          }
          break;

        case SETUP_SET_CONF:
          ROS_INFO("Setting Configuration");
          r = libusb_set_configuration(devh_, 3);
          if (r < 0)
          {
            ROS_ERROR("libusb_set_configuration error %d", r);
            setup_states = SETUP_ERROR;
          } else
          {
            ROS_INFO("Successfully set usb configuration 3");
            setup_states = SETUP_CLAIM_INTERFACE_0;
          }
          break;

        case SETUP_CLAIM_INTERFACE_0:
          r = libusb_claim_interface(devh_, 0);
          if (r < 0)
          {
            ROS_ERROR("libusb_claim_interface 0 error %d", r);
            setup_states = SETUP_ERROR;
          } else
          {
            ROS_INFO("Successfully claimed interface 1");
            setup_states = SETUP_CLAIM_INTERFACE_1;
          }
          break;

        case SETUP_CLAIM_INTERFACE_1:
          r = libusb_claim_interface(devh_, 1);
          if (r < 0)
          {
            ROS_ERROR("libusb_claim_interface 1 error %d", r);
            setup_states = SETUP_ERROR;
          } else
          {
            ROS_INFO("Successfully claimed interface 1");
            setup_states = SETUP_CLAIM_INTERFACE_2;
          }
          break;

        case SETUP_CLAIM_INTERFACE_2:
          r = libusb_claim_interface(devh_, 2);
          if (r < 0)
          {
            ROS_ERROR("libusb_claim_interface 2 error %d", r);
            setup_states = SETUP_ERROR;
          } else
          {
            ROS_INFO("Successfully claimed interface 2");
            setup_states = SETUP_ALL_OK;
          }
          break;
      }
    }

    if (setup_states == SETUP_ERROR)
    {
     	states_ = ERROR;
      is_ok_ = false;
    } else
    {
      is_ok_ = true;
    }
  }
};
