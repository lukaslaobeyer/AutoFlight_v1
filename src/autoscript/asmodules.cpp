#include <dronehelper.h>
#include "asmodules.h"

#define CHECK_RATE 25 // Every how many milliseconds it should be checked if a script abort has been requested

//#define ASSERT_DRONE(d)   if(d == NULL) { std::string error = "FATAL: Control module not properly initialized!"; PyErr_SetString(PyExc_RuntimeError, error.c_str()); boost::python::throw_error_already_set(); return; } // Returns false when the drone control object is NULL (for boolean functions)
//#define ASSERT_DRONE_B(d) if(d == NULL) { std::string error = "FATAL: Control module not properly initialized!"; PyErr_SetString(PyExc_RuntimeError, error.c_str()); boost::python::throw_error_already_set(); return false; } // Returns false when the drone control object is NULL (for boolean functions)
//#define ASSERT_DRONE_F(d) if(d == NULL) { std::string error = "FATAL: Control module not properly initialized!"; PyErr_SetString(PyExc_RuntimeError, error.c_str()); boost::python::throw_error_already_set(); return -1; } // Returns -1.0f when the drone control object is NULL (for float functions)

#define SIMULATE_ACTION(sim, text, ssui) if(sim) { ssui->printAction(text); return; } // Returns true if simulation mode is turned on and prints specified text
#define SIMULATE_ACTION_B(sim, text, ssui) if(sim) { ssui->printAction(text); return true; } // Returns true if simulation mode is turned on and prints specified text
#define SIMULATE_INPUT_B(sim, question, ssui) if(sim) { return ssui->getSimulatedBoolInput(question); }    // If simulation mode is on, asks for "true" or "false"
#define SIMULATE_INPUT_F(sim, question, unit, ssui) if(sim) { return ssui->getSimulatedFloatInput(question, unit); } // If sim. is on, asks for a float value

void handle_status(drone::error status)
{
	if(status == drone::OK)
	{
		return;
	}

	std::string error;
	if(status == drone::NOT_ARMED)
	{
		error = "drone is not armed";
	}
	else if(status == drone::NOT_CONNECTED)
	{
		error = "not connected to drone";
	}
	PyErr_SetString(PyExc_RuntimeError, error.c_str());
	boost::python::throw_error_already_set();
}

///////////////////////////////////////////

AutoScriptModule::AutoScriptModule(std::shared_ptr<Drone> drone, bool simulationMode, IScriptSimulationUI *simulationUI)
{
    if(drone == nullptr)
    {
        throw std::runtime_error("drone control reference is null");
    }

    d = drone;
    sim = simulationMode;
    ssui = simulationUI;
}

void AutoScriptModule::takeoff()
{
    SIMULATE_ACTION(sim, "Taking off", ssui)

    handle_status(drone_takeOff(d));
}

void AutoScriptModule::land()
{
    SIMULATE_ACTION(sim, "Landing", ssui)

    handle_status(drone_land(d));
}

void AutoScriptModule::move(float pitch, float roll, float gaz, float yaw)
{
    if(sim)
    {
        std::stringstream description;
        description << "Moving with pitch = " << pitch << "; roll = " << roll << "; gaz = " << gaz << " and yaw = " << yaw << ".";

        SIMULATE_ACTION(sim, description.str(), ssui)
    }

    handle_status(drone_setAttitude(d, pitch, roll, yaw, gaz));
}

void AutoScriptModule::move_rel(float pitch, float roll, float gaz, float yaw)
{
    if(sim)
    {
        std::stringstream description;
        description << "Moving with pitch = " << pitch << "; roll = " << roll << "; gaz = " << gaz << " and yaw = " << yaw << ".";

        SIMULATE_ACTION(sim, description.str(), ssui)
    }

    handle_status(drone_setAttitudeRel(d, pitch, roll, yaw, gaz));
}

void AutoScriptModule::hover()
{
    SIMULATE_ACTION(sim, "Hovering", ssui)

    handle_status(drone_hover(d));
}

void AutoScriptModule::flip(std::string direction)
{
    SIMULATE_ACTION(sim, "Flipping", ssui)

    //TODO: this
    /*
    if(direction == "AHEAD")
    {
        return d->drone_flip(ardrone::flip::AHEAD);
    }
    else if(direction == "BEHIND")
    {
        return d->drone_flip(ardrone::flip::BEHIND);
    }
    else if(direction == "LEFT")
    {
        return d->drone_flip(ardrone::flip::LEFT);
    }
    else if(direction == "RIGHT")
    {
        return d->drone_flip(ardrone::flip::RIGHT);
    }
    else
    {
        return false;
    }*/
}

boost::python::dict AutoScriptModule::navdata()
{
    boost::python::dict navdata;

    if(d == nullptr)
    {
        return navdata;
    }

    std::shared_ptr<const drone::navdata> data = d->getNavdata();

    if(data == nullptr)
    {
        return navdata;
    }

    navdata["flying"] = data->flying;
    navdata["batterystatus"] = data->batterystatus;
    navdata["linkquality"] = data->linkquality;
    navdata["altitude"] = data->altitude;
    navdata["attitude"] = boost::python::make_tuple(data->attitude[0], data->attitude[1], data->attitude[2]);
    navdata["linearvelocity"] = boost::python::make_tuple(data->linearvelocity[0], data->linearvelocity[1], data->linearvelocity[2]);

    return navdata;
}

boost::python::dict AutoScriptModule::status()
{
    boost::python::dict status;

    status["connected"] = d->isConnected();
    status["armed"] = d->isArmed();
    status["flying"] = d->isFlying();

    return status;
}

void AutoScriptModule::flattrim()
{
    drone::error status = d->addCommand(drone::commands::fttrim());
    handle_status(status);
}

void AutoScriptModule::startrecording()
{
    //TODO: this
}

void AutoScriptModule::stoprecording()
{
    //TODO: this
}

void AutoScriptModule::changeview(std::string view)
{
    //TODO: this
}

void AutoScriptModule::takepicture()
{
    //TODO: this
}

/*
//////////// CONTROL FUNCTIONS ////////////

Control::Control(std::shared_ptr<Drone> drone, bool simulationMode, IScriptSimulationUI *simulationUI)
{
	d = drone;
	sim = simulationMode;
	ssui = simulationUI;
}

void Control::takeoff()
{
	ASSERT_DRONE(d)
	SIMULATE_ACTION(sim, "Taking off", ssui)

	handle_status(drone_takeOff(d));
}

void Control::land()
{
	ASSERT_DRONE(d)
	SIMULATE_ACTION(sim, "Landing", ssui)

    handle_status(drone_land(d));
}

void Control::move(float pitch, float roll, float gaz, float yaw)
{
	ASSERT_DRONE(d)

	if(sim)
	{
		std::stringstream description;
		description << "Moving with pitch = " << pitch << "; roll = " << roll << "; gaz = " << gaz << " and yaw = " << yaw << ".";

		SIMULATE_ACTION(sim, description.str(), ssui)
	}

    handle_status(drone_setAttitudeRel(d, pitch, roll, yaw, gaz));
}
*/
/*bool Control::move_distance(float pitch, float roll, float gaz, float yaw, float centimeters)
{
	ASSERT_DRONE_B(d)

	if(sim)
	{
		std::stringstream description;
		description << "Moving for " << centimeters << "cm with pitch = " << pitch << "; roll = " << roll << "; gaz = " << gaz << " and yaw = " << yaw << ".";

		SIMULATE_ACTION_B(sim, description.str(), ssui)
	}

    std::shared_ptr<const drone::navdata> data = d->getNavdata();
    if(data == nullptr)
    {
        return false;
    }
	if(data->linearvelocity[0] != 0) // Check if linear velocity data is real
	{
		double distance = centimeters / 100.0;

		double time = 0;
		double speedsum = 0;
		int iteration_number = 0;

		double altitude_at_beginning = data->altitude;

		bool ok = move(pitch, roll, gaz, yaw);
		if(!ok)
		{
			return false;
		}

		bool completed = false;
		while(!completed)
		{
            data = d->getNavdata();

            iteration_number++;

			boost::this_thread::sleep_for(boost::chrono::milliseconds(CHECK_RATE));

			if(pitch == 0 && roll == 0 && gaz != 0) // Precision altitude control mode
			{

			}
			else if(gaz == 0) // Ignore altitude readings
			{
                Eigen::Vector3f v = data->linearvelocity;

				time += CHECK_RATE / 1000;
				speedsum += sqrt(v[0]*v[0] + v[1]*v[1]);
				double averagespeed = speedsum / iteration_number;

				if(averagespeed * time >= distance)
				{
					completed = true;
				}
			}
			else if(pitch == 0 && roll == 0 && gaz == 0)
			{
				break;
			}
			else // Combined altitude and pitch/roll distance measurement
			{
                Eigen::Vector3f v = data->linearvelocity;

				time += CHECK_RATE / 1000;
                speedsum += sqrt(v[0]*v[0] + v[1]*v[1]);
				double averagespeed = speedsum / iteration_number;

				double deltaAltitude = fabs(data->altitude - altitude_at_beginning);

				if(sqrt(averagespeed * time + deltaAltitude) >= distance)
				{
					completed = true;
				}
			}

			if(abortFlag)
			{
				break;
			}
		}

		drone_hover(d);

		return completed;
	}
	else
	{
		return false; // Something is wrong with the vertical camera. Linear velocity reported to be 0, which is not possible
	}
}

bool Control::move_time(float pitch, float roll, float gaz, float yaw, int milliseconds)
{
	ASSERT_DRONE_B(d)

	if(sim)
	{
		std::stringstream description;
		description << "Moving for " << milliseconds << "ms with pitch = " << pitch << "; roll = " << yaw << "; gaz = " << gaz << " and yaw = " << yaw << ".";

		SIMULATE_ACTION_B(sim, description.str(), ssui)
	}

	bool ok = move(pitch, roll, gaz, yaw);
	if(!ok)
	{
		return false;
	}

	for(int i = 0; i < (milliseconds / CHECK_RATE); i++)
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds(CHECK_RATE));

		if(abortFlag)
		{
			break;
		}
	}

	hover();

	return true;
}

bool Control::rotate(float speed, float degs)
{
	ASSERT_DRONE_B(d)

	if(sim)
	{
		std::stringstream description;
		description << "Rotating " << degs << " degrees with yaw = " << speed << ".";
		SIMULATE_ACTION_B(sim, description.str(), ssui)
	}

	float initialHeading = d->getNavdata()->attitude(2);
	float previousHeading = initialHeading;
	float degreesRotated = 0;

    bool ok = move(0, 0, 0, speed);
    if(!ok)
    {
        return false;
    }

	bool complete = false;

	while(!complete)
	{
		float heading = d->getNavdata()->attitude(2);

		if(heading >= previousHeading)
		{
			degreesRotated += heading - previousHeading;
		}
		else
		{
			// Just crossed the 0 degree mark
			degreesRotated += (360 - previousHeading) + heading;
		}

		if(degreesRotated >= degs)
		{
			complete = true;
			break;
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(CHECK_RATE));

		previousHeading = heading;

		if(abortFlag)
		{
			break;
		}
	}

	hover();

	return complete;
	return false;
}*/
/*
void Control::hover()
{
	ASSERT_DRONE(d)
	SIMULATE_ACTION(sim, "Hovering", ssui)

	handle_status(drone_hover(d));
}

void Control::flip(std::string direction)
{
	ASSERT_DRONE(d)
	SIMULATE_ACTION(sim, "Flipping", ssui)

	//TODO: this
	*//*
	if(direction == "AHEAD")
	{
		return d->drone_flip(ardrone::flip::AHEAD);
	}
	else if(direction == "BEHIND")
	{
		return d->drone_flip(ardrone::flip::BEHIND);
	}
	else if(direction == "LEFT")
	{
		return d->drone_flip(ardrone::flip::LEFT);
	}
	else if(direction == "RIGHT")
	{
		return d->drone_flip(ardrone::flip::RIGHT);
	}
	else
	{
		return false;
	}*/
/*}

//////////// SENSOR DATA RETREIVAL FUNCTIONS ////////////

Sensors::Sensors(std::shared_ptr<Drone> drone, bool simulationMode, IScriptSimulationUI *simulationUI)
{
	d = drone;
	sim = simulationMode;
	ssui = simulationUI;
}

boost::python::dict Sensors::navdata()
{
	boost::python::dict navdata;

	if(d == nullptr)
	{
		return navdata;
	}

	std::shared_ptr<const drone::navdata> data = d->getNavdata();

    if(data == nullptr)
    {
        return navdata;
    }

	navdata["flying"] = data->flying;
	navdata["batterystatus"] = data->batterystatus;
	navdata["linkquality"] = data->linkquality;
	navdata["altitude"] = data->altitude;
    navdata["attitude"] = boost::python::make_tuple(data->attitude[0], data->attitude[1], data->attitude[2]);
    navdata["linearvelocity"] = boost::python::make_tuple(data->linearvelocity[0], data->linearvelocity[1], data->linearvelocity[2]);

	return navdata;
}*/
/*
float Sensors::getAltitude()
{
	ASSERT_DRONE_F(d)
	SIMULATE_INPUT_F(sim, "Please enter a simulated value for the altitude of the drone", "m", ssui)

    std::shared_ptr<const drone::navdata> data = d->getNavdata();
    if(data)
    {
        return data->altitude;
    }
    else
    {
        return 0;
    }
}

float Sensors::getOrientation(std::string axis)
{
	ASSERT_DRONE_F(d)
	SIMULATE_INPUT_F(sim, std::string("Please enter a simulated angle for the drone's ").append(axis.append(" axis (-180 to +180)")), "degree", ssui)

    std::shared_ptr<const drone::navdata> data = d->getNavdata();
    if(data == nullptr)
    {
        return 0;
    }

	if(axis == "YAW")
	{
		return data->attitude[2];
	}
	else if(axis == "PITCH")
	{
        return data->attitude[0];
	}
	else if(axis == "ROLL")
	{
        return data->attitude[1];
	}
	else
	{
		return -1;
	}
}

float Sensors::getOrientation360(std::string axis, bool clockwise)
{
	ASSERT_DRONE_F(d)
	SIMULATE_INPUT_F(sim, std::string("Please enter a simulated angle for the drone's ").append(axis.append(" axis (0 to 360)")), "degree", ssui)

	float orientation = getOrientation(axis);

	if(clockwise)
	{
		if(orientation < 0)
		{
			orientation += 360;
		}
	}
	else
	{
		if(orientation >= 0)
		{
			orientation = 360 - orientation;
		}
		else
		{
			orientation *= -1;
		}
	}

	return orientation;
}

float Sensors::getAcceleration(std::string axis)
{
	ASSERT_DRONE_F(d)
	SIMULATE_INPUT_F(sim, std::string("Please enter a simulated value for the acceleration on the drone's ").append(axis.append(" axis")), "g", ssui)

	//TODO: this

	if(axis == "X")
	{
		return d->drone_getAcceleration().ax;
	}
	else if(axis == "Y")
	{
		return d->drone_getAcceleration().ay;
	}
	else if(axis == "Z")
	{
		return d->drone_getAcceleration().az;
	}
	else
	{
		return -1;
	}
}

float Sensors::getLinearVelocity(std::string axis)
{
	ASSERT_DRONE_F(d)
	SIMULATE_INPUT_F(sim, std::string("Please enter a simulated linear velocity on the drone's ").append(axis.append(" axis")), "m/s", ssui)

    std::shared_ptr<const drone::navdata> data = d->getNavdata();
    if(data == nullptr)
    {
        return 0;
    }

	if(axis == "X")
	{
		return data->linearvelocity[0];
	}
	else if(axis == "Y")
	{
        return data->linearvelocity[1];
	}
	else if(axis == "Z")
	{
        return data->linearvelocity[2];
	}
	else
	{
		return -1;
	}
}

float Sensors::getBatteryLevel()
{
	ASSERT_DRONE_F(d)
	SIMULATE_INPUT_F(sim, "Please enter a simulated value for the battery level of the drone", "%", ssui)

    std::shared_ptr<const drone::navdata> data = d->getNavdata();
    if(data == nullptr)
    {
        return 0;
    }

    return data->batterystatus;
}*/
/*
//////////// UTILITY FUNCTIONS ////////////

Util::Util(std::shared_ptr<Drone> drone, bool simulationMode, IScriptSimulationUI *simulationUI)
{
	d = drone;
	sim = simulationMode;
	ssui = simulationUI;
}

bool Util::isConnected()
{
	ASSERT_DRONE_B(d)
	SIMULATE_INPUT_B(sim, "Is the drone connected to the computer?", ssui)

	return d->isConnected();
}

bool Util::isArmed()
{
    ASSERT_DRONE_B(d)
    SIMULATE_INPUT_B(sim, "Is the drone armed?", ssui)

    return d->isArmed();
}

bool Util::isFlying()
{
	ASSERT_DRONE_B(d)
	SIMULATE_INPUT_B(sim, "Is the drone flying?", ssui)

	std::shared_ptr<const drone::navdata> data = d->getNavdata();
	if(data == nullptr)
	{
		return false;
	}

	return data->flying;

	return false;
}

bool Util::startRecording()
{
	ASSERT_DRONE_B(d)
	SIMULATE_ACTION_B(sim, "Started recording", ssui)

	//TODO: return d->drone_startRecording();
	return false;
}

bool Util::stopRecording()
{
	ASSERT_DRONE_B(d)
	SIMULATE_ACTION_B(sim, "Stopped recording", ssui)

	//TODO: return d->drone_stopRecording();
	return false;
}

bool Util::flatTrim()
{
	ASSERT_DRONE_B(d)
	SIMULATE_ACTION_B(sim, "Calibrating gyroscope", ssui)

	return drone_flattrim(d);
}

bool Util::calibrateMagnetometer()
{
	ASSERT_DRONE_B(d)
	SIMULATE_ACTION_B(sim, "Calibrating magnetometer", ssui)

	//TODO: return d->drone_calibmagneto();
	return false;
}

bool Util::changeView(std::string view)
{
	ASSERT_DRONE_B(d)
	SIMULATE_ACTION_B(sim, std::string("Changing camera to ").append(view), ssui)

	//TODO: this

	if(view == "FRONT")
	{
		return d->drone_changeView(ardrone::camera::FRONT);
	}
	else if(view == "BOTTOM")
	{
		return d->drone_changeView(ardrone::camera::BOTTOM);
	}
	else
	{
		return false;
	//}
}

bool Util::savePicture(std::string path)
{
	ASSERT_DRONE_B(d)
	SIMULATE_ACTION_B(sim, "Took picture", ssui)

	//TODO: return d->drone_takePicture(path);
	return false;
}

//////////// HARDWARE EXTENSION RELATED FUNCTIONS ////////////

HWExt::HWExt(std::shared_ptr<Drone> drone, bool simulationMode, IScriptSimulationUI *simulationUI)
{
	d = drone;
	sim = simulationMode;
	ssui = simulationUI;
}
*/