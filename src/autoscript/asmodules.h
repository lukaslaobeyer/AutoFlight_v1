#ifndef ASMODULES_H
#define ASMODULES_H

#include <drone.h>
#include "iscriptsimulationui.h"
#include <boost/python.hpp>
#include <string>
#include <memory>

class Control
{
	public:
		Control(std::shared_ptr<Drone> drone = nullptr, bool simulationMode = false, IScriptSimulationUI *simUI = nullptr);

		bool takeOff();
		bool land();

		bool move(float phi, float theta, float gaz, float yaw);
		bool move_time(float phi, float theta, float gaz, float yaw, int milliseconds);
		bool move_distance(float phi, float theta, float gaz, float yaw, float centimeters);

		bool forward(float speed);
		bool forward_time(float speed, int milliseconds);
		bool forward_distance(float speed, float centimeters);

		bool backward(float speed);
		bool backward_time(float speed, int milliseconds);
		bool backward_distance(float speed, float centimeters);

		bool left(float speed);
		bool left_time(float speed, int milliseconds);
		bool left_distance(float speed, float centimeters);

		bool right(float speed);
		bool right_time(float speed, int milliseconds);
		bool right_distance(float speed, float centimeters);

		bool up(float speed);
		bool up_time(float speed, int milliseconds);
		bool up_distance(float speed, float centimeters);

		bool down(float speed);
		bool down_time(float speed, int milliseconds);
		bool down_distance(float speed, float centimeters);

		bool rotate(float speed, float degs);

		bool hover();

		bool flip(std::string direction = "LEFT");

		bool abortFlag = false; // Is checked by functions that run longer, like move_time and move_distance to react to interruptions
	private:
		std::shared_ptr<Drone> d = nullptr;
		bool sim = true;
		IScriptSimulationUI *ssui = NULL;
};

class Sensors
{
	public:
		Sensors(std::shared_ptr<Drone> drone = nullptr, bool simulationMode = false, IScriptSimulationUI *simUI = nullptr);

		float getAltitude();
		float getOrientation(std::string axis);
		float getOrientation360(std::string axis, bool clockwise);
		float getAcceleration(std::string axis);
		float getLinearVelocity(std::string axis);
		float getBatteryLevel();

		bool abortFlag = false;
	private:
		std::shared_ptr<Drone> d = nullptr;
		bool sim = true;
		IScriptSimulationUI *ssui = NULL;
};

class Util
{
	public:
		Util(std::shared_ptr<Drone> drone = nullptr, bool simulationMode = false, IScriptSimulationUI *simUI = nullptr);

		bool isConnected();
		bool isFlying();

		bool startRecording();
		bool stopRecording();

		bool flatTrim();
		bool calibrateMagnetometer();

		bool changeView(std::string view);

		bool savePicture(std::string path);

		bool abortFlag = false;
	private:
		std::shared_ptr<Drone> d = nullptr;
		bool sim = true;
		IScriptSimulationUI *ssui;
};

class HWExt
{
	public:
		HWExt(std::shared_ptr<Drone> drone = nullptr, bool simulationMode = false, IScriptSimulationUI *simUI = nullptr);

		bool abortFlag = false;
	private:
		std::shared_ptr<Drone> d = nullptr;
		bool sim = true;
		IScriptSimulationUI *ssui;
};

#endif
