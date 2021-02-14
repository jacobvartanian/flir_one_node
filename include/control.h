/*! @file
 *
 *  @brief File containing the main control threads used in this program:
 *
 *  This file contains the control threads used in this program to handle passing data to
 *  and from this ROS node.
 * 
 */

#ifndef CONTROL_H
#define CONTROL_H

#include "ros/ros.h"

//ROS-OpenCV Tools for Image Manipulation
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>

#include <sstream>
#include <iostream>
#include <string>

#include <thread>
#include <chrono>
#include <deque>
#include <mutex>
#include <random>
#include <atomic>

#include "driver_flir.h"

#define DATA_POLLING_THREAD_DELAY_MS 100
#define IMAGE_PUBLISH_THREAD_DELAY_MS 50

class Control
{
  public:
    /**
     * @brief Constructor for Control class
     * 
     * @param nh ROS node handler
    */
    Control(ros::NodeHandle nh);

    /**
     * @brief Destructor for Control class
    */
    ~Control();

    /**
     * @brief Thread to collect the data from the FLIR One Camera
    */
    void dataCollectorThread(void);

    /**
     * @brief Thread to continously publish the data image to ROS
    */
    void dataPublisherThread(void);

  private:
    ros::NodeHandle nh_;
    image_transport::ImageTransport it_;
    image_transport::Publisher ir_pub_;
    image_transport::Publisher rgb_pub_;
    ros::Publisher status_pub_;

    driver_flir::DriverFlir driver_flir_;
};

#endif // CONTROL_H