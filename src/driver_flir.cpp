#include <boost/format.hpp>
#include <opencv2/highgui.hpp>

#include "driver_flir.h"

namespace driver_flir
{

  DriverFlir::DriverFlir():
    usb_driver_(VENDOR_ID, PRODUCT_ID)
  {
  }

  DriverFlir::~DriverFlir()
  {
  }

  void DriverFlir::init()
  {
    usb_driver_.usb_setup(magic_byte_);
  }

  void DriverFlir::shutdown()
  {
    usb_driver_.shutdown();
  }

  bool DriverFlir::ok()
  {
    return usb_driver_.ok();
  }

  void DriverFlir::run()
  {
    usb_driver_.poll_data();
  }

  bool DriverFlir::get_latest_images(cv::Mat &image_ir, cv::Mat &image_rgb)
  {
    std::vector<unsigned char> packet;
    bool packet_available = usb_driver_.get_next_packet(packet);
    if (packet_available)
    {
      uint32_t FrameSize   = packet[ 8] + (packet[ 9] << 8) + (packet[10] << 16) + (packet[11] << 24);
      uint32_t ThermalSize = packet[12] + (packet[13] << 8) + (packet[14] << 16) + (packet[15] << 24);
      uint32_t JpgSize     = packet[16] + (packet[17] << 8) + (packet[18] << 16) + (packet[19] << 24);
      uint32_t StatusSize  = packet[20] + (packet[21] << 8) + (packet[22] << 16) + (packet[23] << 24);

      unsigned short pix[160*120];
      int v;
      for (uint8_t y = 0; y < 120; ++y)
      {
        for (uint8_t x = 0; x < 160; ++x)
        {
          if (x<80)
          {
            v = packet[2*(y * 164 + x) +32]+256*packet[2*(y * 164 + x) +33];
          } else
          {
            v = packet[2*(y * 164 + x) +32+4]+256*packet[2*(y * 164 + x) +33+4];
          }
          pix[y * 160 + x] = v;
        }
      }

      cv::Mat im16 = cv::Mat (120, 160, CV_16UC1, pix);
      image_ir = im16;

      cv::Mat rawRgb = cv::Mat(1, JpgSize, CV_8UC1, &packet[28+ThermalSize]);
      cv::Mat decodedImage  =  cv::imdecode(rawRgb, CV_LOAD_IMAGE_COLOR);
      cv::cvtColor(decodedImage, decodedImage, cv::COLOR_BGR2RGB);
      image_rgb = decodedImage;

      return true;
    } else
    {
      return false;
    }
  }
};
