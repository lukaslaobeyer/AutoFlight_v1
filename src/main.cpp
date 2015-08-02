#ifdef __MINGW32__
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif

#include "autoflight.h"
#include "afmainwindow.h"
#include "autoscript/asengine.h"
#include <boost/program_options.hpp>
#include <QApplication>
#include <iostream>
#include <string>
#include <algorithm>
#include <Gamepad.h>

int main(int argc, char *argv[])
{
#ifdef __MINGW32__
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif

	// Parse command line options with boost::program_options
	boost::program_options::options_description desc("Available options");
	desc.add_options()
			("help", "show help message")
			("drone", boost::program_options::value<std::string>(), "set which drone to use (ARDrone2 or Bebop, default is ARDrone2)")
			("ip-address", boost::program_options::value<std::string>(), "an alternative IP address for the drone (default is 192.168.1.1 for AR.Drone 2.0 and 192.168.42.1 for Bebop)")
			("stream-resolution", boost::program_options::value<std::string>(), "resolution for the live video stream (360P default, can be set to 720P)")
	;

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if(vm.count("help"))
	{
	    std::cout << desc << std::endl;
	    return 1;
	}

	drone_type drone_type = BEBOP;
	std::string ip_address = bebop::DEFAULT_IP;
	std::string stream_res = "360P";

	// Detect which drone to use
	if(vm.count("drone"))
	{
		std::string drone_name = vm["drone"].as<std::string>();
		std::transform(drone_name.begin(), drone_name.end(), drone_name.begin(), ::tolower);
		if(drone_name == "ardrone2")
		{
			std::cout << "Using AR.Drone 2.0" << std::endl;
			ip_address = ardrone2::DEFAULT_IP;
			drone_type = ARDRONE2;
		}
		else if(drone_name == "bebop")
		{
			std::cout << "Using Bebop Drone" << std::endl;
			ip_address = bebop::DEFAULT_IP;
			drone_type = BEBOP;
		}
		else
		{
			std::cout << "Drone not supported. Currently only Parrot's AR.Drone 2.0 and Bebop are supported." << std::endl;
		}
	}

	if(vm.count("ip-address"))
	{
		ip_address = vm["ip-address"].as<std::string>();
		std::cout << "Drone IP address manually set to " << ip_address << std::endl;
	}
	else
	{
		std::cout << "Using default Drone IP address ("<< ip_address <<"). Use the --ip-address option to choose a different address." << std::endl;
	}

	if(vm.count("stream-resolution"))
	{
		stream_res = vm["stream-resolution"].as<std::string>();
	}

	std::cout << "Starting AutoFlight...\n";

	Gamepad_init();
	Gamepad_detectDevices();

	AutoFlight af(drone_type, ip_address);

	// TODO: Allow setting stream resolution
	/*
	if(stream_res == "720P")
	{
		af.ardrone2()->setDefaultLiveStreamCodec(ardrone2::config::codec::H264_720P);
		std::cout << "AR.Drone 2.0 live video stream resolution manually set to " << stream_res << std::endl;
	}
	*/

	QApplication gui(argc, argv);

	AFMainWindow w(&af);
	w.show();

	gui.exec();

	Gamepad_shutdown();

	Py_Finalize();

	std::cout << "Closing...\n";

	return 0;
}
