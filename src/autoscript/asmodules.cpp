#include <dronehelper.h>
#include "asmodules.h"

#define CHECK_RATE 25 // Every how many milliseconds it should be checked if a script abort has been requested

#define ASSERT_ARDRONE_B(d) if(d == NULL) { return false; } // Returns false when the ARDrone control object is NULL (for boolean functions)
#define ASSERT_ARDRONE_F(d) if(d == NULL) { return -1.0f; } // Returns -1.0f when the ARDrone control object is NULL (for float functions)

#define SIMULATE_ACTION_B(sim, text, ssui) if(sim) { ssui->printAction(text); return true; } // Returns true if simulation mode is turned on and prints specified text
#define SIMULATE_INPUT_B(sim, question, ssui) if(sim) { return ssui->getSimulatedBoolInput(question); }    // If simulation mode is on, asks for "true" or "false"
#define SIMULATE_INPUT_F(sim, question, unit, ssui) if(sim) { return ssui->getSimulatedFloatInput(question, unit); } // If sim. is on, asks for a float value

//////////// CONTROL FUNCTIONS ////////////

Control::Control(std::shared_ptr<Drone> drone, bool simulationMode, IScriptSimulationUI *simulationUI)
{
	d = drone;
	sim = simulationMode;
	ssui = simulationUI;
}

bool Control::takeOff()
{
	ASSERT_ARDRONE_B(d)
	SIMULATE_ACTION_B(sim, "Taking off", ssui)

	return drone_takeOff(d) == drone::OK;
	return false;
}

bool Control::land()
{
	ASSERT_ARDRONE_B(d)
	SIMULATE_ACTION_B(sim, "Landing", ssui)

	return drone_land(d) == drone::OK;
	return false;
}

bool Control::move(float pitch, float roll, float gaz, float yaw)
{
	ASSERT_ARDRONE_B(d)

	if(sim)
	{
		std::stringstream description;
		description << "Moving with pitch = " << pitch << "; roll = " << roll << "; gaz = " << gaz << " and yaw = " << yaw << ".";

		SIMULATE_ACTION_B(sim, description.str(), ssui)
	}

	return (drone_setAttitudeRel(d, pitch, roll, yaw, gaz) == drone::OK);
	return false;
}

bool Control::move_distance(float pitch, float roll, float gaz, float yaw, float centimeters)
{
	ASSERT_ARDRONE_B(d)

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
	ASSERT_ARDRONE_B(d)

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

bool Control::forward(float speed)
{
	return move(-speed, 0, 0, 0);
}

bool Control::forward_time(float speed, int milliseconds)
{
	return move_time(-speed, 0, 0, 0, milliseconds);
}

bool Control::forward_distance(float speed, float centimeters)
{
	return move_distance(-speed, 0, 0, 0, centimeters);
}

bool Control::backward(float speed)
{
	return move(speed, 0, 0, 0);
}

bool Control::backward_time(float speed, int milliseconds)
{
	return move_time(speed, 0, 0, 0, milliseconds);
}

bool Control::backward_distance(float speed, float centimeters)
{
	return move_distance(speed, 0, 0, 0, centimeters);
}

bool Control::left(float speed)
{
	return move(0, -speed, 0, 0);
}

bool Control::left_time(float speed, int milliseconds)
{
	return move_time(0, -speed, 0, 0, milliseconds);
}

bool Control::left_distance(float speed, float centimeters)
{
	return move_distance(0, -speed, 0, 0, centimeters);
}

bool Control::right(float speed)
{
	return move(0, speed, 0, 0);
}

bool Control::right_time(float speed, int milliseconds)
{
	return move_time(0, speed, 0, 0, milliseconds);
}

bool Control::right_distance(float speed, float centimeters)
{
	return move_distance(0, speed, 0, 0, centimeters);
}

bool Control::up(float speed)
{
	return move(0, 0, speed, 0);
}

bool Control::up_time(float speed, int milliseconds)
{
	return move_time(0, 0, speed, 0, milliseconds);
}

bool Control::up_distance(float speed, float centimeters)
{
	return move_distance(0, 0, speed, 0, centimeters);
}

bool Control::down(float speed)
{
	return move(0, 0, -speed, 0);
}

bool Control::down_time(float speed, int milliseconds)
{
	return move_time(0, 0, -speed, 0, milliseconds);
}

bool Control::down_distance(float speed, float centimeters)
{
	return move_distance(0, 0, -speed, 0, centimeters);
}

bool Control::rotate(float speed, float degs)
{
	ASSERT_ARDRONE_B(d)

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
}

bool Control::hover()
{
	ASSERT_ARDRONE_B(d)
	SIMULATE_ACTION_B(sim, "Hovering", ssui)

	return drone_hover(d) == drone::OK;
	return false;
}

bool Control::flip(std::string direction)
{
	ASSERT_ARDRONE_B(d)
	SIMULATE_ACTION_B(sim, "Flipping", ssui)

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
	{*/
		return false;
	//}
}

//////////// SENSOR DATA RETREIVAL FUNCTIONS ////////////

Sensors::Sensors(std::shared_ptr<Drone> drone, bool simulationMode, IScriptSimulationUI *simulationUI)
{
	d = drone;
	sim = simulationMode;
	ssui = simulationUI;
}

float Sensors::getAltitude()
{
	ASSERT_ARDRONE_F(d)
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
	ASSERT_ARDRONE_F(d)
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
	ASSERT_ARDRONE_F(d)
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
	ASSERT_ARDRONE_F(d)
	SIMULATE_INPUT_F(sim, std::string("Please enter a simulated value for the acceleration on the drone's ").append(axis.append(" axis")), "g", ssui)

	//TODO: this
	/*
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
	{*/
		return -1;
	//}
}

float Sensors::getLinearVelocity(std::string axis)
{
	ASSERT_ARDRONE_F(d)
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
	ASSERT_ARDRONE_F(d)
	SIMULATE_INPUT_F(sim, "Please enter a simulated value for the battery level of the drone", "%", ssui)

    std::shared_ptr<const drone::navdata> data = d->getNavdata();
    if(data == nullptr)
    {
        return 0;
    }

    return data->batterystatus;
}

//////////// UTILITY FUNCTIONS ////////////

Util::Util(std::shared_ptr<Drone> drone, bool simulationMode, IScriptSimulationUI *simulationUI)
{
	d = drone;
	sim = simulationMode;
	ssui = simulationUI;
}

bool Util::isConnected()
{
	ASSERT_ARDRONE_B(d)
	SIMULATE_INPUT_B(sim, "Is the drone connected to the computer?", ssui)

	return d->isConnected();
}

bool Util::isArmed()
{
    ASSERT_ARDRONE_B(d)
    SIMULATE_INPUT_B(sim, "Is the drone armed?", ssui)

    return d->isArmed();
}

bool Util::isFlying()
{
	ASSERT_ARDRONE_B(d)
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
	ASSERT_ARDRONE_B(d)
	SIMULATE_ACTION_B(sim, "Started recording", ssui)

	//TODO: return d->drone_startRecording();
	return false;
}

bool Util::stopRecording()
{
	ASSERT_ARDRONE_B(d)
	SIMULATE_ACTION_B(sim, "Stopped recording", ssui)

	//TODO: return d->drone_stopRecording();
	return false;
}

bool Util::flatTrim()
{
	ASSERT_ARDRONE_B(d)
	SIMULATE_ACTION_B(sim, "Calibrating gyroscope", ssui)

	return drone_flattrim(d);
}

bool Util::calibrateMagnetometer()
{
	ASSERT_ARDRONE_B(d)
	SIMULATE_ACTION_B(sim, "Calibrating magnetometer", ssui)

	//TODO: return d->drone_calibmagneto();
	return false;
}

bool Util::changeView(std::string view)
{
	ASSERT_ARDRONE_B(d)
	SIMULATE_ACTION_B(sim, std::string("Changing camera to ").append(view), ssui)

	//TODO: this
	/*
	if(view == "FRONT")
	{
		return d->drone_changeView(ardrone::camera::FRONT);
	}
	else if(view == "BOTTOM")
	{
		return d->drone_changeView(ardrone::camera::BOTTOM);
	}
	else
	{*/
		return false;
	//}
}

bool Util::savePicture(std::string path)
{
	ASSERT_ARDRONE_B(d)
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
