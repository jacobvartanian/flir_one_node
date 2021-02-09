/*! @file
 *
 *  @brief File containing the main control threads used in this program:
 *
 *  This file contains the control threads used in this program to handle passing data to
 *  and from this ROS node.
 * 
 */
/*!
 * @addtogroup Control_module Control module documentation
 * @{
 */

#include "control.h"

Control::Control(ros::NodeHandle nh):
  nh_(nh),
  it_(nh)
{
  // Initialise publishers
  ir_pub_ = it_.advertise("flirone/images/ir_16b", 1);
  rgb_pub_ = it_.advertise("flirone/images/rgb_jpg", 1);
  status_pub_ = it_.advertise("flirone/status", 1);
  ros::NodeHandle pn("~");
}

Control::~Control()
{
  
}

void Control::dataPublisherThread(void)
{
  /**
  * The below loop runs until ros is shutdown
  */
  ImageData ir_image;
  ImageData rgb_image;
  while (ros::ok())
  {
    bool ir_image_ok = driver_flir_.getLatestImage(ir_image);
    bool rgb_image_ok = driver_flir_.getLatestImage(rgb_image);
    // Publish image
    if((ir_image_ok) && (rgb_image_ok))
    {
      // TODO
      // image_pub_.publish(cv_image.toImageMsg());
    }
    // This delay slows the loop down for the sake of readability
    std::this_thread::sleep_for (std::chrono::milliseconds(IMAGE_PUBLISH_THREAD_DELAY_MS));
  }
  std::cout << __func__ << " thread terminated" << std::endl;
}

void Control::dataCollectorThread(void)
{
  /**
  * The below loop runs until ros is shutdown
  */

  while (ros::ok())
  {
    // TODO Collect the Data

    // This delay slows the loop down for the sake of readability
    std::this_thread::sleep_for (std::chrono::milliseconds(MAIN_THREAD_DELAY_MS));
  }
  std::cout << __func__ << " thread terminated" << std::endl;
}