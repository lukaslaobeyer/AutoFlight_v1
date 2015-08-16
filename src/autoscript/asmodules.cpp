#include <dronehelper.h>
#include "asmodules.h"

#define CHECK_RATE 25 // Every how many milliseconds it should be checked if a script abort has been requested

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