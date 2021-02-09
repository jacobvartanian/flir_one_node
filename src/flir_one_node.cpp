/*!
 * @file flir_one_node.cpp
 * @brief
 *         Main module to initialise the ROS node.
 *
 */         
/*!
 *  @addtogroup flir_one_node main module documentation
 *  @{
 */         
/* MODULE flir_one_node */

#include <signal.h>
#include "control.h"

/*!
 * @brief Method to handle segmentation faults
*/
void sigsegv_handler(int sig)
{
  signal(SIGSEGV, SIG_DFL);
  ROS_ERROR("Segmentation fault, stopping camera.");
  ros::shutdown();                      // stop the main loop
}

/*!
 * @brief Main method to initialise the ros node and start the main thread
*/
int main(int argc, char **argv)
{
  // Initialise this ROS node
  ros::init(argc, argv, "camera_flir_node");
  ros::NodeHandle nh;

  signal(SIGSEGV, &sigsegv_handler);

  // Start the threads contained in the control class
  std::shared_ptr<Control> gc(new Control(nh));
  std::thread dataCollectorThread(&Control::dataCollectorThread,gc);
  std::thread dataPublisherThread(&Control::dataPublisherThread,gc);

  // Handle callbacks
  ros::spin();

  // Clean up everything, shutdown ROS and rejoin the thread
  ros::shutdown();
  dataCollectorThread.join();
  dataPublisherThread.join();

  return 0;
}
