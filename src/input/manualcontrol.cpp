#include "manualcontrol.h"

#include <dronehelper.h>

#include <boost/timer/timer.hpp>

#include <cmath>

#define ENSURE_CONNECTED if(!_af->drone()->isConnected()) { _aw->showMessage(tr("Not connected").toStdString()); return; }

ManualControl::ManualControl(AutoFlight *af, AFMainWindow *aw) : _af(af), _aw(aw) {}

void ManualControl::addControllerInputListener(IControllerInputListener *listener)
{
    _ctrllisteners.push_back(listener);
}

void ManualControl::removeControllerInputListener(IControllerInputListener *listener)
{
    _ctrllisteners.erase(remove(_ctrllisteners.begin(), _ctrllisteners.end(), listener), _ctrllisteners.end());
}

void ManualControl::startUpdateLoop()
{
	_stop_flag = false;
	_updater.reset(new boost::thread(&ManualControl::runUpdateLoop, this));
}

void ManualControl::stopUpdateLoop()
{
	_stop_flag = true;
	if(_updater != nullptr)
	{
		try
		{
			_updater->join();
		}
		catch(boost::thread_interrupted &)
		{}

		_updater.reset(nullptr);
	}
}

void ManualControl::runUpdateLoop()
{
	int runTime = 0;
	int updateInterval = 1000/40;

	boost::timer::cpu_timer timer;

	while(!_stop_flag)
	{
		timer.start();

		_controllerconfig_mutex.lock();
		processControllerInput();
		_controllerconfig_mutex.unlock();

		// Run at continuous update rate
		runTime = timer.elapsed().wall / 1000000;
		timer.stop();

		if(updateInterval - runTime > 0)
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(updateInterval - runTime));
		}
	}
}

void ManualControl::processKeyPress(QKeyEvent *ke)
{
    static const float KEYBOARD_ANGLE = 0.6f;

	if(!ke->isAutoRepeat())
	{
        switch(ke->key())
        {
        case Qt::Key_T:
            // Take off or land
            if(_af->drone()->isFlying())
            {
                perform_landing();
            }
            else
            {
                perform_takeoff();
            }

            break;
        case Qt::Key_F:
            perform_flip();
            break;
        case Qt::Key_P:
            perform_picture();
            break;
        case Qt::Key_R:
            if(_af->fpvdrone())
            {
                if(_af->fpvdrone()->isRecording())
                {
                    perform_stopRecording();
                }
                else
                {
                    perform_startRecording();
                }
            }
            break;
        case Qt::Key_N:
            //TODO: Start/stop Recording Sensor Data
            /*if(_af->ardrone()->drone_isRecordingNavdata())
            {
                showMessage(tr("Stopped recording sensor data").toStdString());
            }
            else
            {
                showMessage(tr("Recording sensor data").toStdString());
            }
            _af->ardrone()->drone_toggleRecordingNavdata();*/
            break;
        case Qt::Key_V:
            // Change View
            perform_switchview();
            break;
        case Qt::Key_Y:
            if((ke->modifiers() & Qt::ShiftModifier) && (ke->modifiers() & Qt::AltModifier))
            {
                // Arm the drone
                if(!drone_isArmed(_af->drone()))
                {
                    perform_arm();
                }
                else
                {
                    perform_disarm();
                }
            }
            else if(!(ke->modifiers() & Qt::ShiftModifier) && !(ke->modifiers() & Qt::AltModifier))
            {
                perform_emergency();
            }
            break;

        case Qt::Key_W:
            // Pitch forward or change camera orientation
            if((ke->modifiers() & Qt::ShiftModifier) && _af->bebop())
            {
                drone_setCameraOrientation(_af->bebop(), 100, 0);
            }
            else
            {
                drone_setPitchRel(_af->drone(), -KEYBOARD_ANGLE);
                _in.pitch = -KEYBOARD_ANGLE;
            }
            break;
        case Qt::Key_A:
            // Roll left or change camera orientation
            if((ke->modifiers() & Qt::ShiftModifier) && _af->bebop())
            {
                drone_setCameraOrientation(_af->bebop(), 0, -100);
            }
            else
            {
                drone_setRollRel(_af->drone(), -KEYBOARD_ANGLE);
                _in.roll = -KEYBOARD_ANGLE;
            }
            break;
        case Qt::Key_S:
            // Pitch backwards or change camera orientation
            if((ke->modifiers() & Qt::ShiftModifier) && _af->bebop())
            {
                drone_setCameraOrientation(_af->bebop(), 0, 0);
            }
            else
            {
                drone_setPitchRel(_af->drone(), KEYBOARD_ANGLE);
                _in.pitch = KEYBOARD_ANGLE;
            }
            break;
        case Qt::Key_D:
            // Roll right or change camera orientation
            if((ke->modifiers() & Qt::ShiftModifier) && _af->bebop())
            {
                drone_setCameraOrientation(_af->bebop(), 0, 100);
            }
            else
            {
                drone_setRollRel(_af->drone(), KEYBOARD_ANGLE);
                _in.roll = KEYBOARD_ANGLE;
            }
            break;

        case Qt::Key_I:
            // Ascend
            drone_setGazRel(_af->drone(), KEYBOARD_ANGLE);
            _in.altitude = KEYBOARD_ANGLE;
            break;
        case Qt::Key_J:
            // Rotate counterclockwise
            drone_setYawRel(_af->drone(), -KEYBOARD_ANGLE);
            _in.yaw = -KEYBOARD_ANGLE;
            break;
        case Qt::Key_K:
            // Descend
            drone_setGazRel(_af->drone(), -KEYBOARD_ANGLE);
            _in.altitude = -KEYBOARD_ANGLE;
            break;
        case Qt::Key_L:
            // Rotate clockwise
            drone_setYawRel(_af->drone(), KEYBOARD_ANGLE);
            _in.yaw = KEYBOARD_ANGLE;
            break;

        default:
            break;
        }

        notifyControllerInputListeners();
	}
}

void ManualControl::processKeyRelease(QKeyEvent *ke)
{
	if(!ke->isAutoRepeat())
	{
		switch(ke->key())
		{
		case Qt::Key_W:
			// Stop forward pitch
			drone_setPitchRel(_af->drone(), 0);
            _in.pitch = 0;
			break;
		case Qt::Key_A:
			// Stop left roll
			drone_setRollRel(_af->drone(), 0);
            _in.roll = 0;
			break;
		case Qt::Key_S:
			// Stop backward pitch
			drone_setPitchRel(_af->drone(), 0);
            _in.pitch = 0;
			break;
		case Qt::Key_D:
			// Stop right roll
			drone_setRollRel(_af->drone(), 0);
            _in.roll = 0;
			break;

		case Qt::Key_I:
			// Stop ascending
			drone_setGazRel(_af->drone(), 0);
            _in.altitude = 0;
			break;
		case Qt::Key_J:
			// Stop rotating counterclockwise
			drone_setYawRel(_af->drone(), 0);
            _in.yaw = 0;
			break;
		case Qt::Key_K:
			// Stop descending
			drone_setGazRel(_af->drone(), 0);
            _in.altitude = 0;
			break;
		case Qt::Key_L:
			// Stop rotating clockwise
			drone_setYawRel(_af->drone(), 0);
            _in.yaw = 0;
			break;

		default:
			break;
		}

        notifyControllerInputListeners();
	}
}

void ManualControl::setControllerConfiguration(ControllerConfiguration *config)
{
	_controllerconfig_mutex.lock();
	_controllerconfig = config;
	_controllerconfig_mutex.unlock();
}

ControllerConfiguration *ManualControl::getControllerConfiguration()
{
	return _controllerconfig;
}

void ManualControl::processControllerInput()
{
	static const int N_CYCLES = 16;
    static const int N_CYCLES_ARMING = 100;
	static const int N_COUNTERS = 9;
	static int cyclesToWait[] = { // Update loop cycles to wait for each action before accepting new commands:
			0, // 0 Takeoff          Needed to ensure that actions do not trigger multiple times on somewhat longer button presses.
			0, // 1 Land             When an action is triggered, the corresponding cycle counter is set to N_CYCLES and decremented
			0, // 2 Zap              by one every cycle until the counter reaches 0.
			0, // 3 Photo
			0, // 4 Recording
			0, // 5 Flip
			0, // 6 Slow mode
			0, // 7 Emergency
			0  // 8 Camera orientation
	};
    static int cyclesToWait_arming = 0;

	if(_controllerconfig != nullptr)
	{
		Gamepad_device *device = Gamepad_deviceAtIndex((unsigned int) _controllerconfig->deviceID);
		Gamepad_processEvents();
		bool slow = false;
		bool changeCamOrientation = false;

		// Decrement cyclesToWait counters if needed
		for(int i = 0; i < N_COUNTERS; i++)
		{
			if(cyclesToWait[i] > 0)
			{
				cyclesToWait[i]--;
			}
		}

		if(device == nullptr)
		{
			drone_hover(_af->drone());
			return;
		}

		if(_controllerconfig->takeoff >= 0 && cyclesToWait[0] == 0)
		{
			if(device->buttonStates[_controllerconfig->takeoff])
			{
				perform_takeoff();
				cyclesToWait[0] = N_CYCLES;
			}
		}
		if(_controllerconfig->land >= 0 && cyclesToWait[1] == 0)
		{
			if(device->buttonStates[_controllerconfig->land])
			{
				perform_landing();
				cyclesToWait[1] = N_CYCLES;
			}
		}
		if(_controllerconfig->zap >= 0 && cyclesToWait[2] == 0)
		{
			if(device->buttonStates[_controllerconfig->zap])
			{
				perform_switchview();
				cyclesToWait[2] = N_CYCLES;
			}
		}
		if(_controllerconfig->photo >= 0 && cyclesToWait[3] == 0)
		{
			if(device->buttonStates[_controllerconfig->photo])
			{
				perform_picture();
				cyclesToWait[3] = N_CYCLES;
			}
		}
		if(_controllerconfig->recording >= 0 && cyclesToWait[4] == 0)
		{
			if(device->buttonStates[_controllerconfig->recording])
			{
				if(_af->fpvdrone())
				{
					if(_af->fpvdrone()->isRecording())
					{
						perform_stopRecording();
					}
					else
					{
						perform_startRecording();
					}
				}
				cyclesToWait[4] = N_CYCLES;
			}
		}
		if(_controllerconfig->flip >= 0 && cyclesToWait[5] == 0)
		{
			if(device->buttonStates[_controllerconfig->flip])
			{
				perform_flip();
				cyclesToWait[5] = N_CYCLES;
			}
		}
		if(_controllerconfig->slow >= 0 && cyclesToWait[6] == 0)
		{
			if(device->buttonStates[_controllerconfig->slow])
			{
				slow = true;
				cyclesToWait[6] = N_CYCLES;
			}
		}
		if(_controllerconfig->camorientation >= 0)
		{
            changeCamOrientation = device->buttonStates[_controllerconfig->camorientation];
		}
		if(_controllerconfig->emergency >= 0 && cyclesToWait[7] == 0)
		{
			if(device->buttonStates[_controllerconfig->emergency])
			{
				perform_emergency();
				cyclesToWait[7] = N_CYCLES;
			}
		}

		float phi = 0, theta = 0, gaz = 0, yaw = 0;

		if(_controllerconfig->rollM >= 0 && _controllerconfig->rollP >= 0)
		{
			phi += device->buttonStates[_controllerconfig->rollP];
			phi -= device->buttonStates[_controllerconfig->rollM];
		}
		else if(_controllerconfig->roll >= 0)
		{
			phi = device->axisStates[_controllerconfig->roll];
		}

		if(_controllerconfig->pitchM >= 0 && _controllerconfig->pitchP >= 0)
		{
			theta += device->buttonStates[_controllerconfig->pitchP];
			theta -= device->buttonStates[_controllerconfig->pitchM];
		}
		else if(_controllerconfig->pitch >= 0)
		{
			theta = device->axisStates[_controllerconfig->pitch];
		}

		if(_controllerconfig->heightM >= 0 && _controllerconfig->heightP >= 0)
		{
			gaz += device->buttonStates[_controllerconfig->heightP];
			gaz -= device->buttonStates[_controllerconfig->heightM];
		}
		else if(_controllerconfig->height >= 0)
		{
			gaz = -device->axisStates[_controllerconfig->height];
		}

		if(_controllerconfig->yawM >= 0 && _controllerconfig->yawP >= 0)
		{
			yaw += device->buttonStates[_controllerconfig->yawP];
			yaw -= device->buttonStates[_controllerconfig->yawM];
		}
		else if(_controllerconfig->yaw >= 0)
		{
			yaw = device->axisStates[_controllerconfig->yaw];
		}

		if(slow)
		{
			phi *= 0.3f;
			theta *= 0.3f;
			gaz *= 0.5f;
			yaw *= 0.5f;
		}

		// Ignore very small values
		if(fabs(phi) < 0.1f)
		{
			phi = 0;
		}
		if(fabs(theta) < 0.1f)
		{
			theta = 0;
		}
		if(fabs(gaz) < 0.1f)
		{
			gaz = 0;
		}
		if(fabs(yaw) < 0.1f)
		{
			yaw = 0;
		}

		if(changeCamOrientation)
		{
			if(cyclesToWait[8] == 0)
			{
				if(_af->bebop())
				{
					// Drone can't deal with too many camera orientation commands (why, Parrot? :( )
					cyclesToWait[8] = 8;

					drone_setCameraOrientation(_af->bebop(), gaz * 100.0f, yaw * 100.0f);
				}
			}

            gaz = 0;
            yaw = 0;
		}

        if(theta > 0.9f && gaz < -0.9f)
        {
            cyclesToWait_arming--;

            if(cyclesToWait_arming <= 0)
            {
                perform_arm();
                cyclesToWait_arming = N_CYCLES_ARMING;
            }
        }
        else
        {
            cyclesToWait_arming = N_CYCLES_ARMING;
        }

		if(!(phi == 0 && theta == 0 && gaz == 0 && yaw == 0))
		{
			drone_setAttitudeRel(_af->drone(), theta, phi, yaw, gaz);
		}
		else
		{
			drone_hover(_af->drone());
		}

		_in.altitude = gaz;
		_in.yaw = yaw;
		_in.pitch = theta;
		_in.roll = phi;

		notifyControllerInputListeners();
	}
}

void ManualControl::clearConfirmationFlags()
{
	_confirmEmergency = false;
	_confirmFlip = false;
}

void ManualControl::notifyControllerInputListeners()
{
    std::shared_ptr<ControllerInput> in = std::make_shared<ControllerInput>();

    *in = _in; // Copy local controller status

    for(IControllerInputListener *l : _ctrllisteners)
    {
        l->controllerInputAvailable(in);
    }
}

void ManualControl::perform_arm()
{
    ENSURE_CONNECTED

    drone_arm(_af->drone());
    _aw->showMessage(tr("Drone armed").toStdString());
}

void ManualControl::perform_disarm()
{
    ENSURE_CONNECTED

    drone_disarm(_af->drone());
    _aw->showMessage(tr("Drone disarmed").toStdString());
}

void ManualControl::perform_emergency()
{
    ENSURE_CONNECTED

    // Emergency
    if(!_confirmEmergency)
    {
        _aw->showMessage(tr("[Emergency] Are you sure?").toStdString());
        QTimer::singleShot(CONFIRMATION_TIMEOUT, this, SLOT(clearConfirmationFlags()));
        _confirmEmergency = true;
    }
    else
    {
        _aw->hideMessages();
        _aw->showMessage(tr("Emergency command sent").toStdString());
        drone_emergency(_af->drone());
    }
}

void ManualControl::perform_landing()
{
    ENSURE_CONNECTED

    _aw->showMessage(tr("Landing").toStdString());
    drone_land(_af->drone());
}

void ManualControl::perform_takeoff()
{
    ENSURE_CONNECTED

    if(!drone_isArmed(_af->drone()))
    {
        _aw->showMessage(tr("Drone is not armed").toStdString());
    }
    else
    {
        _aw->showMessage(tr("Taking off").toStdString());
        drone_takeOff(_af->drone());
    }
}

void ManualControl::perform_switchview()
{
    ENSURE_CONNECTED

    _aw->showMessage(tr("Switching view").toStdString());
    if(_af->ardrone2())
    {
        drone_switchview(_af->ardrone2());
    }
    else if(_af->bebop())
    {
        drone_switchview(_af->bebop());
    }
    else
    {
        _aw->showMessage(tr("Switching view not supported by this drone").toStdString());
    }
}

void ManualControl::perform_flip()
{
    ENSURE_CONNECTED

    if(!_confirmFlip)
    {
        _aw->showMessage(tr("[Flip] Are you sure?").toStdString());
        QTimer::singleShot(CONFIRMATION_TIMEOUT, this, SLOT(clearConfirmationFlags()));
        _confirmFlip = true;
    }
    else
    {
        _aw->hideMessages();
        _aw->showMessage(tr("Performing flip!").toStdString());
		drone::error status = drone_flip(_af->drone());

        if(status == drone::NOT_SUPPORTED)
        {
            _aw->showMessage(tr("Flip for this drone not supported").toStdString());
        }
    }
}

void ManualControl::perform_picture()
{
    ENSURE_CONNECTED

    if(_af->fpvdrone())
    {
        fpvdrone::picturestatus status = _af->fpvdrone()->takePicture();

        if(status == fpvdrone::PIC_OK)
        {
            _aw->showMessage(tr("Picture taken").toStdString());
        }
        else if(status == fpvdrone::PIC_BUSY)
        {
            _aw->showMessage(tr("BUSY: Could not take picture").toStdString());
        }
        else
        {
            _aw->showMessage(tr("ERROR: Could not take picture").toStdString());
        }
    }
}

void ManualControl::perform_startRecording()
{
    ENSURE_CONNECTED

    fpvdrone::picturestatus status = _af->fpvdrone()->startRecording();
    if(status == fpvdrone::PIC_OK)
    {
        _aw->showMessage(tr("Started recording").toStdString());
    }
    else if(status == fpvdrone::PIC_BUSY)
    {
        _aw->showMessage(tr("BUSY: Could not start recording").toStdString());
    }
    else
    {
        _aw->showMessage(tr("ERROR: Could not start recording").toStdString());
    }
}

void ManualControl::perform_stopRecording()
{
    ENSURE_CONNECTED

    _af->fpvdrone()->stopRecording();
    _aw->showMessage(tr("Stopped recording").toStdString());
}