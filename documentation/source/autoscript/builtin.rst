**************************
Basic AutoScript Functions
**************************

AutoScript has some basic functions for accessing drone navigation data and controlling flight. On this page, these built-in functions are documented.

.. warning::

   In AutoFlight 1.0, the AutoScript API has changed and is not compatible with earlier versions of AutoFlight.


They are divided into two modules named ``basicctl`` and ``imgproc``. The ``imgproc`` module contains functions for retrieving and processing images, while the ``basicctl`` module is used for most other simple commands.

For more advanced and high-level commands, please use the ``dronectl`` package.

Exceptions
==========

All the functions in this module will throw a :class:`RuntimeError` when the drone is not connected, not armed (and arming is necessary for the desired operation), an unsupported action is executed or an unknown error occurrs.

.. note::

   Currently :class:`RuntimeError` is used but it is likely that in the future a custom exception class will be created.

Basic control and navigation data retrieval
===========================================

.. function:: basicctl.takeoff()

   Sends a take off command to the drone. This will only send the command and continue immediately, so you'll probably want to wait until the drone has fully taken off.


.. function:: basicctl.land()

   Sends a land command to the drone.


.. function:: basicctl.move(pitch, roll, gaz, yaw)

   Moves the drone by setting the pitch and roll angles as well as vertical and rotational speed. Values will be clipped at the maximum allowable (see :func:`basicctl.limits`).

   .. note:: This function will cause the drone to move with the specified parameters for an infinite amount of time. You will need to call the ``hover()`` command to stop it.

   :param pitch: Pitch angle in radians (positive is backward, negative is forward)
   :param roll: Roll angle in radians (positive is right, negative is left)
   :param gaz: Vertical speed in m/s (positive is up, negative is down)
   :param yaw: Rotational speed in rad/s (positive is clockwise, negative is counterclockwise)


.. function:: basicctl.move_rel(pitch, roll, gaz, yaw)

   Moves the drone. The parameters are multipliers of the allowed maximum (see :func:`basicctl.limits`), and have to be in the range from -1.0 (corresponding to the maximum tilt in one direction) to 1.0 (corresponding to the maximum in the other direction).

   .. note:: This function will cause the drone to move with the specified parameters for an infinite amount of time. You will need to call the ``hover()`` command to stop it.

   :param pitch: Pitch angle (**-1.0**: full speed in **forward** direction; **1.0**: full speed in **backward** direction)
   :param roll: Roll angle (**-1.0**: full angle to **left** hand side; **1.0**: full angle to **right** hand side)
   :param gaz: Vertical speed (**-1.0**: full speed **down**; **1.0**: full speed **up**)
   :param yaw: Rotational speed (**-1.0**: full speed in **counterclockwise** direction; **1.0**: full speed in **clockwise** direction)


.. function:: basicctl.hover()

   Hovers the drone, so it tries to stay at a fixed position. Equivalent to calling ``move(0, 0, 0, 0)``.


.. function:: basicctl.flip(direction)

   Sends the flip command to the drone.

   :param direction: ``'FRONT'``, ``'BACK'``, ``'LEFT'`` or ``'RIGHT'``


.. function:: basicctl.navdata()

   Retrieve the navigation data.

   :returns: A ``dict`` containing all of the available navigation data. Can vary depending on the drone type (AR.Drone 2.0 or Bebop). Will be empty if not connected.


.. function:: basicctl.status()

   Retrieve the drone's status.

   :returns: A ``dict`` containing three boolean keys: ``'connected'``, ``'armed'`` and ``'flying'``.


.. function:: basicctl.limits()

   Get the flight control limits. These directly correspond to the maximum values allowed for :func:`basicctl.move`.

   :returns: A ``dict`` containing the keys:

      * ``'altitude'``: Altitude in meters
      * ``'angle'``: Maximum pitch/roll angle in radians
      * ``'vspeed'``: Maximum vertical speed in m/s
      * ``'yawspeed'``: Maximum rotational (yaw) speed in rad/s


.. function:: basicctl.flattrim()

   Perform a "flat trim".


.. function:: basicctl.set_view(tilt, pan)

   On the Bebop drone, this sets the direction of the digital gimbal.

   :param tilt: Tilt between -100 and 100.
   :param pan: Pan between -100 and 100.


.. function:: basicctl.startrecording()

   Start recording video


.. function:: basicctl.stoprecording()

   Stop recording video.


.. function:: basicctl.switchview(view)

   Switch between front and bottom view. On the AR.Drone 2.0 this switches between the front and bottom camera. On the Bebop this changes the view of the digital gimbal.

   :param view: ``'TOGGLE'`` to toggle between front/bottom, ``'FRONT'`` to look front, ``'BOTTOM'`` to look down.


.. function:: basicctl.takepicture()

   Take a picture.


Image Processing
================

.. warning::

    **Using OpenCV's built in GUI functionality is not (yet) possible in AutoFlight.** To display an image, you should always use ``imgproc.showFrame(img)`` and never OpenCV's ``imshow()`` or similar.


.. note::

    I am still refining the image processing capabilities of AutoScript. Right now it is possible to retrieve images, to process them using OpenCV for Python, to display them in the AutoFlight main window and to use the April Tag detector built into AutoFlight.


.. function:: imgproc.latest_frame()

   Get the latest frame.

   :returns: A numpy array containing the latest received video frame in 8-bit 3 channel BGR format.


.. function:: imgproc.frame_age()

   Get the age of the latest frame. Useful for ignoring outdated frames when the video feed is interrupted.

   :returns: The age of the latest frame, in milliseconds.


.. function:: imgproc.show_frame(frame)

   Show an image in the AutoFlight main window.

   :param frame: A numpy array containing the frame to display. Must be 8-bit 3 channel BGR or binary.


.. function:: imgproc.start_tag_detector()

   Start the integrated April tag detector.


.. function:: imgproc.stop_tag_detector()

   Stop the integrated April tag detector.


.. function:: imgproc.set_tag_family(family)

   Tell the tag detector which April tag family you want to detect.

   :param family: The name of the tag family (e.g.: ``"Tag36h11"``)


.. function:: imgproc.set_tag_roi(x, y, width, height)

   Set the rectangular region of interest for the tag detector. Only this region will be analyzed when running the tag detector, potentially reducing CPU usage significantly.

   :param x: X-coordinate of the upper left point of your ROI
   :param y: Y-coordinate of the upper left point of your ROI
   :param width: Width of the ROI
   :param height: Height of the ROI


.. function:: imgproc.tag_detections()

   Get the detected tags.

   :returns: A list of tuples, each tuple representing one detected tag. Each tuple contains the tag's ID, a flag indicating whether the detection is valid or not and a list of X/Y coordinates of the tag's edges.
