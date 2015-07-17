#ifndef ARDRONECONFIGURATIONFILEIO_H
#define ARDRONECONFIGURATIONFILEIO_H

#include <pugixml/pugixml.hpp>
#include <string>
#include <drone.h>

namespace DroneConfigurationFileIO
{
	bool saveDroneConfiguration(drone::config config, int index);
	drone::config loadDroneConfiguration(int index);
	void addDroneConfigNode(pugi::xml_node &root, std::string name, std::string value);
};

#endif
