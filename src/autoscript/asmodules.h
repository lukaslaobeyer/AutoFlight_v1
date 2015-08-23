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
        boost::python::dict limits();

        void flattrim();

        void startrecording();
        void stoprecording();

        void switchview(std::string view = "TOGGLE");

        void set_view(float tilt, float pan);

        void takepicture();
    private:
        std::shared_ptr<Drone> d = nullptr;
        bool sim = true;
        IScriptSimulationUI *ssui = NULL;
};
#endif
