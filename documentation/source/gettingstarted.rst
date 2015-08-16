Overview
********

AutoFlight's GUI is divided into a main panel used to display the video stream and two sidebars containing buttons (to connect to the drone, launch the scripting environment, etc.) and indicators to show the real time sensor data.

Basic Usage
===========

.. warning::

   By using AutoFlight you agree that I'm not responsible for any damage you might cause to your drone while using this program. This is a beta version and software failures may very well happen!

Connecting to the drone
-----------------------

To start flying your drone, make sure you are already connected to it via WiFi, just like you would connect to any other wireless router. Then, just click the button in the upper right corner ("Connect to drone"). In a few seconds the real time video stream and sensor data should appear.

.. note::

	In the AutoFlight 1.0 beta version, there are problems when reconnecting for a second time in one session. Please make sure you have a stable WiFi connection to your drone before launching AutoFlight and clicking "Connect to drone". Also, when you lose the WiFi connection, you will need to **restart AutoFlight before reconnecting**.

.. note::

	Please make sure you are not connected to any other network besides your drone's.

Head-Up Display
---------------

You can switch to a head-up display mode by pressing :kbd:`F5` or going into :menuselection:`View --> Head-Up Display` option. Now the sensor data should be presented as an overlay on top of the video stream. Press F5 again to exit this mode.

Flying with a Joystick / Gamepad / Keyboard
-------------------------------------------

Arming/disarming the drone
++++++++++++++++++++++++++

AutoFlight will refuse to send commands to the drone unless it is **armed**. To arm the drone and start flying, you will have to press :kbd:`shift` + :kbd:`alt` + :kbd:`Y`. To disarm the drone and disallow takeoff, press :kbd:`shift` + :kbd:`alt` + :kbd:`Y` again.

To arm the drone using a controller, hold your altitude and pitch control at minimum for around 4 seconds until a confirmation that your drone has been armed appears.

The status indicator on the right sidebar shows whether the drone is armed or not.

Flying with a Joystick / Gamepad
++++++++++++++++++++++++++++++++

You can configure your joystick over the :menuselection:`Edit --> Controller Configuration` menu - it should be pretty straightforward.
If you don't have a controller, you can fly with your keyboard, too. The next section shows how.

.. note::

    I have successfully used the XBox One controller, the Logitech Extreme 3D Pro joystick and several generic, unbranded controllers.
	My gamepad and joysticks work flawlessy, but if your do not, I've heard from users that `MotioninJoy <http://www.motioninjoy.com/>`_ can solve the problems.


Flying with the Keyboard
++++++++++++++++++++++++

No configuration is needed. Just use the commands described in the table below.

.. image:: _images/gettingstarted/af_controls.svg
   :width: 820px


+-------------------------------------------+---------------------------------------+---------------------------------------------+-------------+
| Drone Commands                                                                    | AutoFlight Commands                                       |
+===========================================+=======================================+=============================================+=============+
| Take Off / Land                           | :kbd:`T`                              | Take Picture                                | :kbd:`P`    |
+-------------------------------------------+---------------------------------------+---------------------------------------------+-------------+
| Switch camera (Front/Bottom)              | :kbd:`V`                              | Start/Stop recording video                  | :kbd:`R`    |
+-------------------------------------------+---------------------------------------+---------------------------------------------+-------------+
| Flip                                      | 2x :kbd:`F`                           | Toggle HUD                                  | :kbd:`F5`   |
+-------------------------------------------+---------------------------------------+---------------------------------------------+-------------+
| Emergency                                 | 2x :kbd:`Y`                           | Start/Stop recording sensor data            | :kbd:`N`    |
+-------------------------------------------+---------------------------------------+---------------------------------------------+-------------+
| Up | Rotate left | Down | Rotate right    | :kbd:`I` :kbd:`J` :kbd:`K` :kbd:`L`   |                                                           |
+-------------------------------------------+---------------------------------------+---------------------------------------------+-------------+
| Forward | Left | Backward | Right         | :kbd:`W` :kbd:`A` :kbd:`S` :kbd:`D`   |                                                           |
+-------------------------------------------+---------------------------------------+---------------------------------------------+-------------+

Important warnings and known issues
===================================

*This program is still in beta, which means that it is not yet stable and complete enough to be considered production-quality software. Also, you should keep in mind that I can not take responsability for broken drones and you should use this program at your own risk. (However, should AutoFlight crash while flying, under normal circumstances the drone would hover and descend to an altitude of 1m.)*

The controller configuration is not checked automatically (yet), so you should confirm that you haven't assigned the same button/axis to multiple actions.

Some features like the image processor are not implemented yet but may be shown in the menus.

A few AutoScript functions are not implemented yet (see in-program AutoScript documentation).

The WiFi indicator works only for the Bebop drone as the AR.Drone 2.0 does not correctly report its signal strength.

There seem to be problems with the 3D map view not adjusting the view correctly (the virtual camera does not follow the drone indicator as it should).

**If the main panel doesn't show the AutoFlight logo and you are unable to see the live video stream or the head-up display, you should make sure that you have at least OpenGL version 2. When running the program in VirtualBox (or other virtual environments) this might be a problem.**

Miscellaneous
=============

Photos/Video
------------

**AR.Drone 2.0 only:**
Photos and recorded videos are saved in your home folder, under a new folder called AutoFlightSaves (e.g. in ``C:\Users\your_username\AutoFlightSaves`` on Windows 7).

**Bebop only:**
Photos and recorded videos are saved on the Bebop's memory. To download them, go into :menuselection:`Tools --> Download media stored on Bebop`.

Drone configuration
-------------------

Go into the :menuselection:`Drone --> Flight Settings` menu to change the on-board flight parameters of the drone (max. roll/pitch angles, max. height, etc.).

For the Bebop drone, you can also configure the video/picture settings in the :menuselection:`Drone --> Video/picture settings` menu.

MAVLink?
--------

As the Bebop drone has integrated GPS, it would be useful to be able to control it with widely used software such as `QGroundControl <http://qgroundcontrol.org/>`_. However, these programs use `MAVLink <http://qgroundcontrol.org/mavlink/start>`_ for communication, which the Bebop does not support.

AutoFlight will automatically relay the Bebop's navigation data, converting back and forth between Parrot's proprietary communication protocols and MAVLink. So go ahead and install `QGroundControl <http://qgroundcontrol.org/>`_, connect with the default UDP link and you should start receiving some basic Bebop navigation data and position information directly inside QGroundControl!

.. note::

	This feature is still under heavy development. Right now, AutoFlight only sends MAVLink packets and ignores any requests, so Waypoints and other commands that will make this feature useful in the future are **not implemented yet**.
