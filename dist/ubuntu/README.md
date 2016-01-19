# AutoFlight for Ubuntu/Linux

AutoFlight has been compiled and tested on Ubuntu 14.04 LTS, but should work on newer versions and Ubuntu/Debian derivatives.

## Prerequisites

AutoFlight ships with its own OpenCV 3.0 and FFmpeg libraries. However, you will still need to install some Boost libraries, Qt 5, x264, Python 3 and NumPy in order to use it.

On Ubuntu, you can install these dependencies by running the following command:

```
sudo apt-get install python3 python3-numpy libx264-142 qtbase5 libqt5webkit5 libqt5opengl5 qtmultimedia5-dev libboost-program-options1.55.0 libboost-system1.55.0 libboost-python1.55.0 libboost-filesystem1.55.0 libboost-thread1.55.0 libboost-chrono1.55.0 libboost-date-time1.55.0 libboost-timer1.55.0 libboost-log1.55.0 libyaml-cpp0.5
```

## Running AutoFlight

To run AutoFlight, just execute the ``AutoFlight-ARDrone2.sh`` or ``AutoFlight-Bebop.sh`` script, depending on the drone you want to use.

## Getting started

You should visit [electronics.kitchen/gettingstarted](http://electronics.kitchen/gettingstarted) for important information before using AutoFlight.
