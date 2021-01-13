# Using Video for Linux 2 (V4L2)

In the case where ROS is not available, the video data from the FLIR One can be streamed to Linux by running the appropriate drivers. This can be useful for testing and evaluation purposes.

## Testing Conditions

This procedure was tested on the following machines and operating systems:

1. DELL XPS 13 (with UEFI Secure Boot)
```
Kernel: 5.4.0-60-generic x86_64
bits: 64
Desktop: Gnome 3.28.4
Distro: Ubuntu 18.04.5 LTS
```

2. Lattepanda SBC
```
Ubuntu 16.04
```

## Device Setup

Install the following packages using the following

```
$ sudo apt-get install g++
$ sudo apt-get install libusb-1.0-0-dev
$ sudo apt-get install linux-generic
$ sudo apt-get install mplayer
```

Install `v4l2loopback` from [github](https://github.com/umlaeute/v4l2loopback). Follow the instructions in this repository to install.

To check that `v4l2loopback` has installed correctly, run the following:

```
$ sudo modprobe v4l2loopback
```

This will create a new virtual video device for the physical video devices connected to your system.

If an error message is received, refer to the troubleshooting steps in the `v4l2loopback` documentation. This will likely occur on UEFI secure boot devices, in which case, the module should be built [via DKMS](https://github.com/umlaeute/v4l2loopback#DKMS).

When working with DKMS, you need to be superuser through either of the following:

```
$ su
$ sudo -i
```

## Install

The first step is to create new dummy video devices for the FLIR One camera for the three image streams.

Assign a virtual video device to each existing video device and check what devices are connected to the system:

```
$ sudo modprobe v4l2loopback
$ v4l2-ctl --list-devices
```

You should see a list of devices and an associated video. You can also list the virtual video devices used as follows:

```
$ ls /dev/video*
```

Create the three new virtual video devices using the following command:

```
$ sudo modprobe v4l2loopback devices=3
```

Alternatively, you can specify the video number and name with the following

```
$ sudo modprobe v4l2loopback video_nr=<N>,<N+1>,<N+2> card_label="<Device name N>","<Device name N+1>","<Device name N+2>"
```

where N is the next available virtual video device.

For example, the following device has an internal webcam and two external USB cameras. Listing the devices gave the following result:

```
$ v4l2-ctl --list-devices
Integrated_Webcam_HD: Integrate (usb-0000:00:14.0-5):
	/dev/video0
	/dev/video1
	/dev/video2
	/dev/video3

USB2.0 UVC PC Camera: USB2.0 UV (usb-0000:3c:00.0-2.3.2.1.1.1):
	/dev/video4
	/dev/video5

Microsoft® LifeCam Studio(TM): (usb-0000:3f:00.0-1.2):
	/dev/video6
	/dev/video7
```

As there are eight devices already (0-7), three new additional devices will be created as video8, video9 and video10:

```
$ sudo modprobe v4l2loopback devices=3
$ v4l2-ctl --list-devices
Dummy video device (0x0000) (platform:v4l2loopback-000):
	/dev/video8

Dummy video device (0x0001) (platform:v4l2loopback-001):
	/dev/video9

Dummy video device (0x0002) (platform:v4l2loopback-002):
	/dev/video10

Integrated_Webcam_HD: Integrate (usb-0000:00:14.0-5):
	/dev/video0
	/dev/video1
	/dev/video2
	/dev/video3

USB2.0 UVC PC Camera: USB2.0 UV (usb-0000:3c:00.0-2.3.2.1.1.1):
	/dev/video4
	/dev/video5

Microsoft® LifeCam Studio(TM): (usb-0000:3f:00.0-1.2):
	/dev/video6
	/dev/video7
```

If not all dummy videos were created, you can try and reset v4l2loopback, which is loaded on boot (may occur especially if multiple video devices are connected). Recreate the devices again as follows:

```
$ sudo rmmod v4l2loopback
$ sudo modprobe v4l2loopback devices=3
$ v4l2-ctl --list-devices
```

The source code of [flirone.c](src/flirone.c) needs to then be modified to assign the video streams to one of these virtual devices (lines 41-51). Only the string for the video device name needs to be changed

```
#define VIDEO_DEVICE0 "/dev/video8"  // gray scale thermal image
#define FRAME_WIDTH0  160
#define FRAME_HEIGHT0 120

#define VIDEO_DEVICE1 "/dev/video9" // color visible image
#define FRAME_WIDTH1  640
#define FRAME_HEIGHT1 480

#define VIDEO_DEVICE2 "/dev/video10" // colorized thermal image
#define FRAME_WIDTH2  160
#define FRAME_HEIGHT2 128
```

Compile the code by running

```
$ make
```

## Run

Power on the FLIR One device and connect it to your machine. It is recommended to use a connection system that supports USB3.0 or higher

Wait until the FLIR One has completely powered on, and the Green LED is blinking on the power button.

Run the FLIR driver with a palette from the [palletes](palettes) folder. For example:

```
$ ./flirone palettes/7.raw 
```

While the above script is running, open a new terminal window and run:

```
$ mplayer tv:// -tv driver=v4l2:device=/dev/<videoN>
```

where videN is one of the three video devices you created earlier, for example:

```
$ mplayer tv:// -tv driver=v4l2:device=/dev/video8
```

You should now see a stream of the video from the FLIR One.

## References

<https://www.eevblog.com/forum/thermal-imaging/question-about-flir-one-for-android/msg832208/#msg832208>

<https://github.com/umlaeute/v4l2loopback>

<https://github.com/fnoop/flirone-v4l2>

<https://news.ycombinator.com/item?id=22805925>