#ifndef DRIVER_FLIR_H
#define DRIVER_FLIR_H

#include <boost/thread/mutex.hpp>

#include <vector>

#include <ros/ros.h>
#include <camera_info_manager/camera_info_manager.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/fill_image.h>

#include "usb_packet_driver.h"

/** @file

    @brief ROS driver interface for UEYE-compatible USB digital cameras.

*/

#define VENDOR_ID 0x09cb
#define PRODUCT_ID 0x1996

namespace driver_flir
{

  class DriverFlir
  {
  public:
    DriverFlir();
    ~DriverFlir();
    bool get_latest_images(cv::Mat &image_ir, cv::Mat &image_rgb);
    void init();
    void shutdown();
    bool ok();
    void run();

  private:
    const unsigned char magic_byte_[4] = {0xEF,0xBE,0x00,0x00};
    usb_packet_driver::UsbPacketDriver usb_driver_;
  };
};

#endif // DRIVER_FLIR_H