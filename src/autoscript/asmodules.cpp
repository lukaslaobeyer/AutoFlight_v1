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
    else if(status == drone::NOT_SUPPORTED)
    {
        error = "action not supported for this drone";
    }
    else if(status == drone::ERR_UNKNOWN)
    {
        error = "unknown error";
    }
	PyErr_SetString(PyExc_RuntimeError, error.c_str());
	boost::python::throw_error_already_set();
}

class navdatavisitor : public dronevisitor
{
    public:
        boost::python::dict *navdata;

        drone::error visit(std::shared_ptr<ARDrone2> d)
        {
            if(navdata == nullptr)
            {
                return drone::ERR_UNKNOWN;
            }

            std::shared_ptr<const ardrone2::navdata> nd = std::static_pointer_cast<const ardrone2::navdata>(d->getNavdata());
            (*navdata)["acceleration"] = boost::python::make_tuple(nd->acceleration[0], nd->acceleration[1], nd->acceleration[2]);
            (*navdata)["magnetometer"] = boost::python::make_tuple(nd->magnetometer[0], nd->magnetometer[1], nd->magnetometer[2]);
            (*navdata)["pressure"] = nd->pressure;
            return drone::OK;
        }

        drone::error visit(std::shared_ptr<Bebop> d)
        {
            if(navdata == nullptr)
            {
                return drone::ERR_UNKNOWN;
            }

            std::shared_ptr<const bebop::navdata> nd = std::static_pointer_cast<const bebop::navdata>(d->getNavdata());
            (*navdata)["cameraorientation"] = boost::python::make_tuple(nd->cameraorientation[0], nd->cameraorientation[1]);
            (*navdata)["gps_fix"] = nd->gps_fix;
            (*navdata)["latitude"] = nd->latitude;
            (*navdata)["longitude"] = nd->longitude;
            (*navdata)["gps_altitude"] = nd->gps_altitude;
            (*navdata)["gps_sats"] = nd->gps_sats;
            if(nd->full)
            {
                (*navdata)["ultrasound_height"] = nd->full_navdata.ultrasound_height;
                (*navdata)["pressure"] = nd->full_navdata.pressure;
                (*navdata)["horizontal_velocity"] = boost::python::make_tuple(nd->full_navdata.horiz_speed[0], nd->full_navdata.horiz_speed[1]);
                (*navdata)["ned_velocity"] = boost::python::make_tuple(nd->full_navdata.ned_speed[0], nd->full_navdata.ned_speed[1], nd->full_navdata.ned_speed[2]);
                (*navdata)["vbat"] = nd->full_navdata.vbat;
                (*navdata)["gps_speed"] = nd->full_navdata.gps_speed;
                (*navdata)["gps_bearing"] = nd->full_navdata.gps_bearing;
                (*navdata)["gps_accuracy"] = nd->full_navdata.gps_accuracy;
                (*navdata)["gps_eph"] = nd->full_navdata.gps_eph;
                (*navdata)["gps_epv"] = nd->full_navdata.gps_epv;
            }
            return drone::OK;
        }
};

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

    drone::error status = drone_flip(d, direction);
    handle_status(status);
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

    navdatavisitor ndv;
    ndv.navdata = &navdata;
    d->accept(&ndv);

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

boost::python::dict AutoScriptModule::limits()
{
    boost::python::dict limits;

    drone::limits l = d->getLimits();
    limits["angle"] = l.angle;
    limits["altitude"] = l.altitude;
    limits["vspeed"] = l.vspeed;
    limits["yawspeed"] = l.yawspeed;

    return limits;
}

void AutoScriptModule::flattrim()
{
    drone::error status = d->addCommand(drone::commands::fttrim());
    handle_status(status);
}

void AutoScriptModule::startrecording()
{
    drone::error status = drone_startRecording(d);
    handle_status(status);
}

void AutoScriptModule::stoprecording()
{
    drone::error status = drone_stopRecording(d);
    handle_status(status);
}

void AutoScriptModule::switchview(std::string view)
{
    drone::error status = drone_switchview(d, view);
    handle_status(status);
}

void AutoScriptModule::set_view(float tilt, float pan)
{
    drone::error status = drone_setCameraOrientation(d, tilt, pan);
    handle_status(status);
}

void AutoScriptModule::takepicture()
{
    drone::error status = drone_takePicture(d);
    handle_status(status);
}