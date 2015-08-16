#ifndef ASMODULES_H
#define ASMODULES_H

#include <drone.h>
#include "iscriptsimulationui.h"
#include <boost/python.hpp>
#include <string>
#include <memory>

class AutoScriptModule
{
    public:
        AutoScriptModule(std::shared_ptr<Drone> drone = nullptr, bool simulationMode = false, IScriptSimulationUI *simUI = nullptr);

        void takeoff();
        void land();

        void move(float pitch, float roll, float gaz, float yaw);
        void move_rel(float pitch, float roll, float gaz, float yaw);
        void hover();

        void flip(std::string direction = "LEFT");

        boost::python::dict navdata();
        boost::python::dict status();

        void flattrim();

        void startrecording();
        void stoprecording();

        void changeview(std::string view);

        void takepicture();

        bool abortFlag = false; // Is checked by functions that run longer, like move_time and move_distance to react to interruptions
    private:
        std::shared_ptr<Drone> d = nullptr;
        bool sim = true;
        IScriptSimulationUI *ssui = NULL;
};
/*
class Control
{
	public:
		Control(std::shared_ptr<Drone> drone = nullptr, bool simulationMode = false, IScriptSimulationUI *simUI = nullptr);

		void takeoff();
		void land();

		void move(float pitch, float roll, float gaz, float yaw);
		void hover();

		void flip(std::string direction = "LEFT");

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

		boost::python::dict navdata();

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
		bool isArmed();

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
*/
#endif
