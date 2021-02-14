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
  // status_pub_ = it_.advertise("flirone/status", 1);
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
  cv::Mat ir_image;
  cv::Mat rgb_image;
  while (ros::ok())
  {
    bool image_ok = driver_flir_.get_latest_images(ir_image, rgb_image);
    // Publish image
    if(image_ok)
    {
      ros::Time stamp = ros::Time::now();
      cv_bridge::CvImage out_msg;
      out_msg.header.frame_id = "flir";
      out_msg.header.stamp = stamp;
      out_msg.encoding = sensor_msgs::image_encodings::TYPE_16UC1;
      out_msg.image = ir_image;
      ir_pub_.publish(out_msg.toImageMsg());

      std_msgs::Header header;
      header.frame_id = "flir";
      header.stamp = stamp;
      
      sensor_msgs::ImagePtr msg = cv_bridge::CvImage(header,"rgb8", rgb_image).toImageMsg();
      msg->encoding = "rgb8" ;
      rgb_pub_.publish(msg);
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
    ROS_INFO("Initialising Connection");
    driver_flir_.init();
    while (ros::ok() && driver_flir_.ok())
    {
      driver_flir_.run();

      // This delay slows the loop down for the sake of readability
      std::this_thread::sleep_for (std::chrono::milliseconds(DATA_POLLING_THREAD_DELAY_MS));
    }
    driver_flir_.shutdown();
    if (ros::ok())
    {
      ROS_INFO("Retrying...");
    }
    std::this_thread::sleep_for (std::chrono::milliseconds(3000));
  }
  std::cout << __func__ << " thread terminated" << std::endl;
}