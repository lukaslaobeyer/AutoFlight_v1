#include "dronehelper.h"

#include <drones/ardrone2/commands.h>
#include <drones/bebop/commands.h>

#include <Eigen/Dense>

namespace
{
	float _pitch = 0.0f, _roll = 0.0f, _yaw = 0.0f, _gaz = 0.0f;
	float _pitchRel = 0.0f, _rollRel = 0.0f, _yawRel = 0.0f, _gazRel = 0.0f;
}

bool drone_takeOff(std::shared_ptr<Drone> drone)
{
	drone::command takeOff = drone::commands::takeoff();
	return drone->addCommand(takeOff);
}

bool drone_land(std::shared_ptr<Drone> drone)
{
	drone::command land = drone::commands::land();
	return drone->addCommand(land);
}

bool drone_emergency(std::shared_ptr<Drone> drone)
{
	drone::command emergency = drone::commands::emergency();
	return drone->addCommand(emergency);
}

bool drone_hover(std::shared_ptr<Drone> drone)
{
	drone::command attitude = drone::commands::attitude(Eigen::Vector3f(0.0f, 0.0f, 0.0f), 0.0f);
	return drone->addCommand(attitude);
}

bool drone_setAttitude(std::shared_ptr<Drone> drone, float pitch, float roll, float yaw, float gaz)
{
	drone::command attitude = drone::commands::attitude(Eigen::Vector3f(pitch, roll, yaw), gaz);
	return drone->addCommand(attitude);
}

bool drone_setAttitudeRel(std::shared_ptr<Drone> drone, float pitchRel, float rollRel, float yawRel, float gazRel)
{
	drone::command attitude = drone::commands::attituderel(Eigen::Vector3f(pitchRel, rollRel, yawRel), gazRel);
	return drone->addCommand(attitude);
}

bool drone_setPitch(std::shared_ptr<Drone> drone, float pitch)
{
	_pitch = pitch;
	return drone_setAttitude(drone, _pitch, _roll, _yaw, _gaz);
}

bool drone_setRoll(std::shared_ptr<Drone> drone, float roll)
{
	_roll = roll;
	return drone_setAttitude(drone, _pitch, _roll, _yaw, _gaz);
}

bool drone_setYaw(std::shared_ptr<Drone> drone, float yaw)
{
	_yaw = yaw;
	return drone_setAttitude(drone, _pitch, _roll, _yaw, _gaz);
}

bool drone_setGaz(std::shared_ptr<Drone> drone, float gaz)
{
	_gaz = gaz;
	return drone_setAttitude(drone, _pitch, _roll, _yaw, _gaz);
}

bool drone_setPitchRel(std::shared_ptr<Drone> drone, float pitchRel)
{
	_pitchRel = pitchRel;
	return drone_setAttitudeRel(drone, _pitchRel, _rollRel, _yawRel, _gazRel);
}

bool drone_setRollRel(std::shared_ptr<Drone> drone, float rollRel)
{
	_rollRel = rollRel;
	return drone_setAttitudeRel(drone, _pitchRel, _rollRel, _yawRel, _gazRel);
}

bool drone_setYawRel(std::shared_ptr<Drone> drone, float yawRel)
{
	_yawRel = yawRel;
	return drone_setAttitudeRel(drone, _pitchRel, _rollRel, _yawRel, _gazRel);
}

bool drone_setGazRel(std::shared_ptr<Drone> drone, float gazRel)
{
	_gazRel = gazRel;
	return drone_setAttitudeRel(drone, _pitchRel, _rollRel, _yawRel, _gazRel);
}

bool drone_flip(std::shared_ptr<ARDrone2> drone)
{
	drone::command flip = ardrone2::commands::flip(ardrone2::commands::flip::direction::BACK);
	return drone->addCommand(flip);
}

bool drone_flip(std::shared_ptr<Bebop> drone)
{
	drone::command flip = bebop::commands::flip(bebop::commands::flip::direction::BACK);
	return drone->addCommand(flip);
}

bool drone_switchview(std::shared_ptr<ARDrone2> drone)
{
	static ardrone2::commands::switchview::view view = ardrone2::commands::switchview::BOTTOM;

	if(view == ardrone2::commands::switchview::BOTTOM)
	{
		view = ardrone2::commands::switchview::FRONT;
	}
	else
	{
		view = ardrone2::commands::switchview::BOTTOM;
	}

	drone::command switchview = ardrone2::commands::switchview(view);
	return drone->addCommand(switchview);
}

bool drone_switchview(std::shared_ptr<Bebop> drone)
{
	static float tilt = 0.0f;

	if(tilt == 0.0f)
	{
		tilt = 100.0f;
	}
	else
	{
		tilt = 0.0f;
	}

	drone::command camorientation = bebop::commands::camera_orientation(tilt, 0.0f);
	return drone->addCommand(camorientation);
}
