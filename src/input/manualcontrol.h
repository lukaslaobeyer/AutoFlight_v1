#ifndef MANUALCONTROL_H
#define MANUALCONTROL_H

#include <QtWidgets>

#include "../autoflight.h"
#include "../afmainwindow.h"
#include "../input/controllerconfiguration.h"
#include <Gamepad.h>

#include <boost/thread.hpp>

#include <atomic>
#include <memory>
#include <mutex>

#define CONFIRMATION_TIMEOUT 1250

class AFMainWindow;

class ManualControl : public QObject
{
	Q_OBJECT

	public:
		ManualControl(AutoFlight *af, AFMainWindow *aw);

		void startUpdateLoop();
		void stopUpdateLoop();
		void runUpdateLoop();

		void processKeyPress(QKeyEvent *ke);
		void processKeyRelease(QKeyEvent *ke);

		void setControllerConfiguration(ControllerConfiguration *config);
		ControllerConfiguration *getControllerConfiguration();

		void processControllerInput();

	private:
		std::atomic<bool> _stop_flag{false};
		std::unique_ptr<boost::thread> _updater;
		std::mutex _controllerconfig_mutex;

		AutoFlight *_af = nullptr;
		AFMainWindow *_aw = nullptr;

		ControllerConfiguration *_controllerconfig = nullptr;

		bool _confirmFlip = false;      // Needed to request a confirmation for performing a flip/sending an emergency command
		bool _confirmEmergency = false;

	private Q_SLOTS:
		void clearConfirmationFlags(); // Called after a timeout to clear the flags used by the confirmation mechanism for performing a flip/sending emergency commands
};

#endif
