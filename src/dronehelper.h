#ifndef DRONE_HELPER_H
#define DRONE_HELPER_H

#include <memory>
#include <drone.h>
#include <drones/ardrone2/ardrone2.h>
#include <drones/bebop/bebop.h>

bool drone_takeOff(std::shared_ptr<Drone> drone);
bool drone_land(std::shared_ptr<Drone> drone);
bool drone_emergency(std::shared_ptr<Drone> drone);

bool drone_hover(std::shared_ptr<Drone> drone);
bool drone_setAttitude(std::shared_ptr<Drone> drone, float pitch, float roll, float yaw, float gaz); // Absolute values in radians / m/s
bool drone_setAttitudeRel(std::shared_ptr<Drone> drone, float pitchRel, float rollRel, float yawRel, float gazRel); // Relative values from -1 to 1

bool drone_setPitch(std::shared_ptr<Drone> drone, float pitch);
bool drone_setRoll(std::shared_ptr<Drone> drone, float roll);
bool drone_setYaw(std::shared_ptr<Drone> drone, float yaw);
bool drone_setGaz(std::shared_ptr<Drone> drone, float gaz);

bool drone_setPitchRel(std::shared_ptr<Drone> drone, float pitchRel);
bool drone_setRollRel(std::shared_ptr<Drone> drone, float rollRel);
bool drone_setYawRel(std::shared_ptr<Drone> drone, float yawRel);
bool drone_setGazRel(std::shared_ptr<Drone> drone, float gazRel);

bool drone_flip(std::shared_ptr<ARDrone2> drone);
bool drone_flip(std::shared_ptr<Bebop> drone);

bool drone_switchview(std::shared_ptr<ARDrone2> drone);
bool drone_switchview(std::shared_ptr<Bebop> drone);

#endif
