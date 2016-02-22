#include "autoflight.h"
#include "afconstants.h"
//#include "tools/ardroneconfigurationfileio.h"

#include <iostream>
#include <boost/filesystem.hpp>
#include <QCoreApplication>
#include <QDir>

using namespace std;

AutoFlight::AutoFlight(drone_type drone_type, string ip, bool enable_mavlink)
{
	_drone_type = drone_type;

	if(drone_type == ARDRONE2)
	{
		_drone = make_shared<ARDrone2>(getHomeDirectory() + "AutoFlightSaves/");
		if(ip.length() > 0)
		{
			ardrone2()->setIP(ip);
		}
	}
	else if(drone_type == BEBOP)
	{
		_drone = make_shared<Bebop>();
		if(ip.length() > 0)
		{
			bebop()->setIP(ip);
		}
	}

	_ase = new ASEngine(fpvdrone());
	_ase->initPython();

	//TODO: Saves; _drone->setSaveDirectory(getHomeDirectory().append("AutoFlightSaves/"));

	_srec = new SessionRecorder(_sessionRecDoc);
	_srec->addEvent("ProgramStart");

	//TODO: Session recording; _drone->setSessionRecorder(_srec);

	if(enable_mavlink == true && drone_type == BEBOP)
	{
		try
		{
			_mavlink = new MAVLinkProxy();
		}
		catch(boost::system::system_error const& e)
		{
			cerr << "Could not start MAVLink proxy: " << e.what() << endl;
		}
	}

	if(_mavlink != nullptr)
	{
		bebop()->addNavdataListener(_mavlink);
		_mavlink->start();
		cout << "Started MAVLink proxy" << endl;
	}
}

AutoFlight::~AutoFlight()
{
	if(_drone != nullptr)
	{
		_drone->stopUpdateLoop();
	}
	if(_mavlink != nullptr)
	{
		_mavlink->stop();
		delete _mavlink;
	}
	delete _ase;
	delete _srec;
}

string AutoFlight::getProgramDirectory()
{
	string pathToApp = QCoreApplication::applicationDirPath().toStdString();

	if(pathToApp.back() != '/' && pathToApp.back() != '\\')
	{
		pathToApp.append("/");
	}

	return pathToApp;
}

string AutoFlight::getHomeDirectory()
{
	string home = QDir::homePath().toStdString();

	if(home.back() != '/' && home.back() != '\\')
	{
		home.append("/");
	}

	return home;
}

string AutoFlight::droneName()
{
	if(_drone_type == ARDRONE2)
	{
		return "AR.Drone 2.0";
	}
	else if(_drone_type == BEBOP)
	{
		return "Bebop";
	}
	else
	{
		return "Unknown drone";
	}
}

shared_ptr<Drone> AutoFlight::drone()
{
	return _drone;
}

shared_ptr<FPVDrone> AutoFlight::fpvdrone()
{
	if(_drone_type == ARDRONE2 || _drone_type == BEBOP)
	{
		return static_pointer_cast<FPVDrone>(_drone);
	}
	else
	{
		return nullptr;
	}
}

shared_ptr<ARDrone2> AutoFlight::ardrone2()
{
	if(_drone_type == ARDRONE2)
	{
		return static_pointer_cast<ARDrone2>(_drone);
	}
	else
	{
		return nullptr;
	}
}

shared_ptr<Bebop> AutoFlight::bebop()
{
	if(_drone_type == BEBOP)
	{
		return static_pointer_cast<Bebop>(_drone);
	}
	else
	{
		return nullptr;
	}
}

ASEngine *AutoFlight::asengine()
{
	return _ase;
}

SessionRecorder *AutoFlight::sessionrecorder()
{
	return _srec;
}

MAVLinkProxy *AutoFlight::mavlink()
{
	return _mavlink;
}

bool AutoFlight::attemptConnectionToDrone()
{
	drone::connectionstatus connected = _drone->connect();
	switch(connected)
	{
	case drone::connectionstatus::ALREADY_CONNECTED:
		cout << "Already connected!\n";
		break;
	case drone::connectionstatus::CONNECTION_FAILED:
		cerr << "Error connecting!\n";
		break;
	case drone::connectionstatus::EXCEPTION_OCCURRED:
		cerr << "Exception occurred while connecting!\n";
		break;
	case drone::connectionstatus::CONNECTION_ESTABLISHED:
		cout << "Connected!\n";
		break;
	}

	if(connected == drone::connectionstatus::CONNECTION_ESTABLISHED || connected == drone::connectionstatus::ALREADY_CONNECTED)
	{
		if(connected == drone::connectionstatus::CONNECTION_ESTABLISHED)
		{
			_drone->startUpdateLoop();
		}
		return true;
	}
	else
	{
		return false;
	}
}

void AutoFlight::saveSession()
{
	string filename = "AF_";
	filename.append(AutoFlight::af_timestamp());
	filename.append(".xml");

	string sessiondir = getHomeDirectory();
	sessiondir.append("AutoFlightSaves/Sessions/");

	boost::filesystem::create_directories(sessiondir);

	cout << "Saving session under " << (sessiondir + filename) << endl;

	_sessionRecDoc.save_file((sessiondir + filename).c_str());
}

string AutoFlight::af_timestamp()
{
	const boost::posix_time::ptime time = boost::posix_time::second_clock::local_time();

	stringstream timestamp;
	timestamp << setw(4) << setfill('0') << time.date().year() << setw(2) << time.date().month().as_number() << setw(2) << time.date().day().as_number();
	timestamp << "T";
	timestamp << setw(2) << time.time_of_day().hours() << setw(2) << time.time_of_day().minutes() << setw(2) << time.time_of_day().seconds();

	return timestamp.str();
}
