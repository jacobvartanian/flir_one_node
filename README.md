# FLIR One Interfacing Node

This package is used to open a connection to the FLIR one camera and publish data from it to various ROS topics.

This guide assumes that [ROS](https://www.ros.org/) has been installed and setup on the user's system

Package has been tested using `ROS Melodic` running on `Ubuntu 18.04`

## 1. Allow access to USB device

The FLIR One has a vendor ID of `0x09CB` and a porduct ID of `0x1996`

To allow the package to access data from this, create a new `rules` file

```
sudo nano /etc/udev/rules.d/51-usb-flir-one.rules
```

Add the following text to this file

```
SUBSYSTEMS=="usb", ATTRS{idVendor}=="09cb", ATTRS{idProduct}=="1996", MODE:="0666"
```

Restart the system

## 2. Create and run the ROS package

Ensure that this git repository is contained within the `src` folder of the `catkin_ws` directory.

Run `catkin_make` from within the `catkin_ws` directory.

Ensure the package can be run by executing the following command from the `catkin_ws` directory:

```
source devel/setup.bash
```

Ensure an instance of ROS is running on the machine

```
roscore
```

Ensuring the camera is connected and powered on, run the ROS package

```
rosrun flir_one_node flir_one_node
```

The image can be viewed using RQT (or similar) on the following topics:

```
/camera_flir_node/rgb/image_raw
/camera_flir_node/ir_8b/image_raw
/camera_flir_node/ir_16b/image_raw
```