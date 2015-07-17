#include <iostream>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include "../autoflight.h"
#include "droneconfigurationfileio.h"

using namespace std;

bool DroneConfigurationFileIO::saveDroneConfiguration(drone::config config, int index)
{
	if(!config.valid)
    {
        return false;
    }

    pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("af:droneconfig");
	pugi::xml_attribute xmlns = root.append_attribute("xmlns:af");
	xmlns.set_value("http://electronics.kitchen/autoflight");

	addDroneConfigNode(root, "altitude_max", boost::lexical_cast<string>(config.limits.altitude));
	addDroneConfigNode(root, "pitch_roll_max", boost::lexical_cast<string>(config.limits.angle));
	addDroneConfigNode(root, "vertical_speed_max", boost::lexical_cast<string>(config.limits.vspeed));
	addDroneConfigNode(root, "yaw_speed_max", boost::lexical_cast<string>(config.limits.yawspeed));
	addDroneConfigNode(root, "outdoor_flight", boost::lexical_cast<string>(config.outdoor));

	string path = AutoFlight::getProgramDirectory() + "droneconfig_" + boost::lexical_cast<string>(index) + ".xml";
	return doc.save_file(path.c_str());
}

drone::config DroneConfigurationFileIO::loadDroneConfiguration(int index)
{
	string path = AutoFlight::getProgramDirectory() + "droneconfig_" + boost::lexical_cast<string>(index) + ".xml";

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(path.c_str());

	drone::config config;

	if(result.status != pugi::status_ok)
	{
		cout << "[WARNING] Could not find/parse drone configuration file." << endl;
		config.valid = false;
		return config;
	}

	pugi::xml_node root = doc.root().first_child();

	for(pugi::xml_node child = root.first_child(); child; child = child.next_sibling())
	{
		if(strcmp(child.name(), "af:droneconfkey") != 0 || child.attribute("key").as_string()[0] == '\0' || child.attribute("value").as_string()[0] == '\0')
		{
			cerr << "[ERROR] Invalid drone configuration file (" << path << ")." << endl;
            config.valid = false;
			return config;
		}

		string key = child.attribute("key").as_string();
		string value_str = child.attribute("value").as_string();
		float value;
		try
		{
			stringstream value_strstr(value_str);
			value_strstr.imbue(locale("C"));
			value_strstr >> value;
		}
		catch(exception &e)
		{
			cout << e.what() << endl;
			config.valid = false;
            return config;
		}

		if(key == "altitude_max")
		{
			config.limits.altitude = value;
		}
		else if(key == "outdoor_flight")
		{
            config.outdoor = value;
		}
		else if(key == "pitch_roll_max")
		{
			config.limits.angle = value;
		}
		else if(key == "vertical_speed_max")
		{
			config.limits.vspeed = value;
		}
		else if(key == "yaw_speed_max")
		{
			config.limits.yawspeed = value;
		}
	}

    config.valid = true;
	return config;
}

void DroneConfigurationFileIO::addDroneConfigNode(pugi::xml_node &root, string name, string value)
{
	replace(value.begin(), value.end(), ',', '.'); // Decimal separator needs to be a dot (boost lexical_cast seems to recognize the current locale)

	pugi::xml_node event = root.append_child("af:droneconfkey");

	pugi::xml_attribute key_attr = event.append_attribute("key");
	key_attr.set_value(name.c_str());

	pugi::xml_attribute value_attr = event.append_attribute("value");
	value_attr.set_value(value.c_str());
}
