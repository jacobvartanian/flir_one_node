/*! @file
 *
 *  @brief Class to connect to and collect USB data
 *
 *  This class establishes a USB connection to a device and loads data
 *  received from it in a buffer as defined by a packet header
 * 
 */

#define BUF85SIZE 1048576
#include <libusb.h>
#include <queue>
#include <ros/ros.h>

struct buffer {
  int pointer;
  unsigned char data[BUF85SIZE];
};

namespace usb_packet_driver
{

  class UsbPacketDriver
  {
  public:

    UsbPacketDriver(int vendor_id, int product_id, unsigned char * magic_byte);
    ~UsbPacketDriver();
    bool get_next_packet(unsigned char * packet);

    void poll_data(void);
    void usb_setup(void);
    void shutdown(void);

    bool ok();

  private:
    void read(char ep[],char EP_error[], int r, int actual_length, unsigned char buf[]);
    void print_bulk_result(char ep[],char EP_error[], int r, int actual_length, unsigned char buf[]);

    libusb_context *context_;
    struct libusb_device_handle *devh_;

   	char EP81_error_[50];
    char EP83_error_[50];
    char EP85_error_[50];
    int actual_length_;
    buffer usb_buffer_;
    std::queue<buffer> packet_queue_;

    enum states_t {INIT, INIT_1, INIT_2, ASK_ZIP, ASK_VIDEO, POOL_FRAME, ERROR};
    states_t states_;

    enum setup_states_t {SETUP_INIT, SETUP_LISTING, SETUP_FIND, SETUP_SET_CONF, SETUP_CLAIM_INTERFACE_0,  SETUP_CLAIM_INTERFACE_1,  SETUP_CLAIM_INTERFACE_2, SETUP_ALL_OK, SETUP_ERROR};
    setup_states_t setup_states_;

    int error_code_;

    bool is_ok_;

    int vendor_id_;
    int product_id_;
    unsigned char *magic_byte_;
  };
};
