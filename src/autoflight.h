#ifndef AUTOFLIGHT_H
#define AUTOFLIGHT_H

#include <string>
#include <memory>

#include <pugixml/pugixml.hpp>
#ifdef __MINGW32__
#include <winsock2.h>
#endif
#include <boost/date_time.hpp>

#include <drone.h>
#include <drones/ardrone2/ardrone2.h>
#include <drones/bebop/bebop.h>
#include <src/mavlink/mavlinkproxy.h>

#include "autoscript/asengine.h"
#include "tools/sessionrecorder.h"

enum drone_type
{
	ARDRONE2,
	BEBOP
};

class AutoFlight
{
	public:
		AutoFlight(drone_type drone_type, std::string drone_ip = "", bool enable_mavlink = false);
		~AutoFlight();

		bool attemptConnectionToDrone();

		std::string droneName();
		std::shared_ptr<Drone> drone();
		std::shared_ptr<FPVDrone> fpvdrone();
		std::shared_ptr<ARDrone2> ardrone2();
		std::shared_ptr<Bebop> bebop();

		ASEngine *asengine();
		SessionRecorder *sessionrecorder();
		MAVLinkProxy *mavlink();

		void saveSession();

		static std::string getProgramDirectory(); // Returns the directory where the AutoFlight executable is (needed to find support files)
		static std::string getHomeDirectory();

		static std::string af_timestamp();
	private:
		drone_type _drone_type;

		std::shared_ptr<Drone> _drone = nullptr;
		ASEngine *_ase = nullptr;
		SessionRecorder *_srec = nullptr;

		MAVLinkProxy *_mavlink = nullptr;

		pugi::xml_document _sessionRecDoc;
};

#endif
