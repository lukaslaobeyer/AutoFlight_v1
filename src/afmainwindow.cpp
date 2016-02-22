#include "afmainwindow.h"

#include "afconstants.h"

#include <dronehelper.h>

#include "widgets/orientation.h"
#include "widgets/altitude.h"
#include "widgets/speed.h"
#include "widgets/battery.h"
#include "widgets/signalstrength.h"
#include "widgets/connection.h"
#include "widgets/flightplan.h"

#include "dialogs/selectcontroller.h"
#include "dialogs/configurecontrols.h"
#include "dialogs/welcomedialog.h"

#include "tools/controllerconfigurationfileio.h"

#include <QtWidgets>

#include <memory>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>
#include "widgets/gps.h"
#include "widgets/status.h"
#include "tools/bebopftp.h"
#include "dialogs/bebopmediadownload.h"
#include "dialogs/settingsdialog.h"
#include "tools/settingsfileio.h"
#include "defaultsettings.h"
#include "settingshelper.h"

// On a normal sized screen, minimum screen size should be:
#define PREF_WIDTH 1270
#define PREF_HEIGHT 775

#define CONFIRMATION_TIMEOUT 1250

using namespace std;

void AFMainWindow::setWindowAttributes()
{
	string title;
	title = "AutoFlight - " + _af->droneName();

	setWindowTitle(QString::fromStdString(title));
	setWindowIcon(QIcon(":/resources/icon.png"));
	setMinimumSize(PREF_WIDTH, PREF_HEIGHT);
}

AFMainWindow::AFMainWindow(AutoFlight *af, QWidget *parent) : QMainWindow(parent)
{
	_af = af;

	// Register meta types
	qRegisterMetaType<std::string>();
	qRegisterMetaType<shared_ptr<const drone::navdata>>();
    qRegisterMetaType<shared_ptr<const ControllerInput>>();

	_messageTimer = new QTimer(this);
	QObject::connect(_messageTimer, SIGNAL(timeout()), this, SLOT(hideMessages()));
	_messageTimer->setSingleShot(true);

	QWidget *main = new QWidget();
	setCentralWidget(main);

	grid = new QGridLayout();
	main->setLayout(grid);

	setWindowAttributes();

	createMenuBar();

	// Shows important notifications
	msg = new QLabel(this);
	msg->setAlignment(Qt::AlignCenter);
	msg->setStyleSheet("background: rgba(30, 30, 30, 0.85); font-size: 24px; color: #FFFFFF; border-radius: 0;");
	msg->hide();

    bool bebop = false;
    if(_af->bebop())
    {
        bebop = true;
    }

	videoPanel = new VideoDisplay(0, bebop);
	//videoPanel->setAlignment(Qt::AlignCenter);
	grid->addWidget(videoPanel, 0, 0, 1, 1);

	videoPanel->setCurrentFrame(QImage(":/resources/autoflight.png").scaledToWidth(600, Qt::SmoothTransformation));

	horizontalToolbar = createHorizontalToolbar();
	verticalToolbar = createVerticalToolbar();

	grid->addWidget(horizontalToolbar, 1, 0, 1, 1);
	grid->addWidget(verticalToolbar, 0, 1, 2, 1);

	ImageVisualizer *iv = new ImageVisualizer(); // TODO: this is only a test
	QObject::connect(iv, SIGNAL(videoFrameAvailableSignal(QImage)), this, SLOT(processedFrameAvailable(QImage))); // TODO: this is only a test

	_imgProcTest = new ImageProcessor(iv); // TODO: this is only a test

	_af->drone()->addNavdataListener(this);
    _af->drone()->addStatusListener(this);
	if(_af->fpvdrone())
	{
		_af->fpvdrone()->addVideoListener(this);
		//_af->fpvdrone()->addVideoListener(_imgProcTest); // TODO: this is only a test
	}
	_af->drone()->addConnectionStatusListener(this);

	if(_af->mavlink() != nullptr)
	{
		_af->mavlink()->addFlightPlanListener(this);
	}

	_manualcontrol.reset(new ManualControl(_af, this));
	_manualcontrol->startUpdateLoop();
    _manualcontrol->addControllerInputListener(this);

    QObject::connect(this, SIGNAL(showMessageSignal(QString)), this, SLOT(showMessageSlot(QString)));
	QObject::connect(this, SIGNAL(videoFrameAvailableSignal(QImage)), this, SLOT(videoFrameAvailable(QImage)));
	QObject::connect(this, SIGNAL(connectionLostSignal()), this, SLOT(handleConnectionLost()));
	QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), videoPanel, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));
	QObject::connect(this, SIGNAL(controllerInputAvailableSignal(std::shared_ptr<const ControllerInput>)), videoPanel, SLOT(controllerInputAvailable(std::shared_ptr<const ControllerInput>)));
    QObject::connect(this, SIGNAL(statusUpdateAvailableSignal(int)), videoPanel, SLOT(statusUpdateAvailable(int)));

	installEventFilter(this);

	// TODO: Fix memory leaks
	ControllerConfiguration *cc = ControllerConfigurationFileIO::loadControllerConfiguration();
	if(cc != nullptr)
	{
		_manualcontrol->setControllerConfiguration(cc);
	}

	// Initialize sound alerts
    QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), &_soundAlerts, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));

	showFirstRunInfoIfRequired();

    loadDroneConfiguration();
}

void AFMainWindow::showFirstRunInfoIfRequired()
{
	// Check if this is the first run: If the AutoFlightSaves directory exists, AutoFlight was already run
	string savesDir = AutoFlight::getHomeDirectory();
	savesDir += "AutoFlightSaves";
	bool exists = boost::filesystem::exists(savesDir);

	if(!exists)
	{
		// Show first run info
		WelcomeDialog welcome(this);
		welcome.exec();
	}
}

void AFMainWindow::showMessage(string message)
{
	Q_EMIT showMessageSignal(QString::fromStdString(message));
}

void AFMainWindow::launchAutoScriptIDE(std::string file, std::vector<std::string> args)
{
	Q_EMIT launchAutoScriptIDESlot(file, args);
}

void AFMainWindow::showMessageSlot(QString message)
{
	const static int msg_width = 450;

    msg->setText(message);
    msg->setGeometry((size().width() / 2 - (msg_width / 2)), (size().height() / 2 - 25), msg_width, 50);
    msg->raise();
    msg->show();

    _messageTimer->start(CONFIRMATION_TIMEOUT);
}

void AFMainWindow::hideMessages()
{
	_messageTimer->stop(); // Stop the timer if it's running
	msg->hide();
}

void AFMainWindow::navdataAvailable(shared_ptr<const drone::navdata> nd)
{
	Q_EMIT navdataAvailableSignal(nd);
}

void AFMainWindow::statusUpdateAvailable(int status)
{
    Q_EMIT statusUpdateAvailableSignal(status);
}

void AFMainWindow::connectionLost()
{
	Q_EMIT connectionLostSignal();
}

void AFMainWindow::connectionEstablished()
{
    Q_EMIT connectionEstablishedSignal();
	//_imgProcTest->startProcessing(); // TODO: this is only a test
}

void AFMainWindow::flightPlanAvailable(std::string flightplan)
{
	std::string path = AutoFlight::getHomeDirectory() + "AutoFlightSaves/FlightPlan/";

	boost::filesystem::create_directory(path);

	path += AutoFlight::af_timestamp() + ".mavlink";

	std::ofstream out(path);
	out << flightplan;
	out.close();

	Q_EMIT flightPlanAvailableSignal(path);
}

void AFMainWindow::videoFrameAvailable(cv::Mat f)
{
	// Convert cv::Mat to QImage
	QImage img(f.data, f.cols, f.rows, f.step, QImage::Format_RGB888);
	img = img.rgbSwapped();

	Q_EMIT videoFrameAvailableSignal(img);
}

void AFMainWindow::controllerInputAvailable(shared_ptr<const ControllerInput> in)
{
	Q_EMIT controllerInputAvailableSignal(in);
}

void AFMainWindow::videoFrameAvailable(QImage f)
{
    if(_lastProcessedFrameTime + 1000 < (uint64_t) QDateTime::currentMSecsSinceEpoch())
    {
        videoPanel->setMaximized(true);
        videoPanel->setCurrentFrame(f);
    }
}

void AFMainWindow::processedFrameAvailable(QImage f)
{
    _lastProcessedFrameTime = (uint64_t) QDateTime::currentMSecsSinceEpoch();

    videoPanel->setMaximized(true);
    videoPanel->setCurrentFrame(f);
}

void AFMainWindow::createMenuBar() {
	QMenu *drone = new QMenu(tr("&Drone"));
	menuBar()->addMenu(drone);

		QAction *connectDrone = new QAction(tr("&Connect to drone"), this);
		drone->addAction(connectDrone);

		drone->addSeparator();

		QAction *flatTrim = new QAction(tr("&Flat Trim"), this);
		drone->addAction(flatTrim);

		QAction *calibMagneto = new QAction(tr("Calibrate &Magnetometer"), this);
		drone->addAction(calibMagneto);

		drone->addSeparator();

		QAction *flightSettings = new QAction(tr("Flight &Settings"), this);
		drone->addAction(flightSettings);

		if(_af->bebop())
		{
			QAction *bebopVideoSettings = new QAction(tr("&Video/picture settings"), this);
			QWidget::connect(bebopVideoSettings, SIGNAL(triggered()), this, SLOT(launchBebopVideoSettings()));
			drone->addAction(bebopVideoSettings);
		}

		drone->addSeparator();

        if(_af->bebop())
        {
            QAction *resetBebop = new QAction(tr("&Reset drone settings"), this);
            QWidget::connect(resetBebop, SIGNAL(triggered()), this, SLOT(resetBebopSettings()));
            drone->addAction(resetBebop);
        }
		/* TODO: This
		QAction *pairDrone = new QAction(tr("Pair (Mac-Address coupling)"), this);
		drone->addAction(pairDrone);

		QAction *unpairDrone = new QAction(tr("Unpair"), this);
		drone->addAction(unpairDrone);
		*/
	QMenu *tools = new QMenu(tr("&Tools"));
	menuBar()->addMenu(tools);

		QAction *controlConfig = new QAction(tr("&Controller Configuration"), this);
		tools->addAction(controlConfig);

		QAction *imgProcEdit = new QAction(tr("&Image Processing Pipeline Editor"), this);
		//tools->addAction(imgProcEdit);

        if(_af->bebop())
        {
            tools->addSeparator();
            QAction *mediaDownload = new QAction(tr("&Download Media stored on Bebop"), this);
            QWidget::connect(mediaDownload, SIGNAL(triggered()), this, SLOT(downloadBebopMedia()));
            tools->addAction(mediaDownload);
        }

		/* TODO: This
		QAction *controlInfo = new QAction(tr("Controller Information"), this);
		tools->addAction(controlInfo);

		tools->addSeparator();

		QAction *configEditor = new QAction(tr("Configuration Editor"), this);
		tools->addAction(configEditor);
		*/
	QMenu *view = new QMenu(tr("&View"));
	menuBar()->addMenu(view);
		QAction *toggleHUD = new QAction(tr("Head-&Up Display"), this);
		toggleHUD->setCheckable(true);
		toggleHUD->setShortcut(QKeySequence::fromString("F5"));
		view->addAction(toggleHUD);

		/* TODO: Fullscreen
		view->addSeparator();

		QAction *toggleFullscreen = new QAction(tr("Fullscreen"), this);
		toggleFullscreen->setCheckable(true);
		view->addAction(toggleFullscreen);
		*/
	QMenu *help = new QMenu(tr("&Help"));
	menuBar()->addMenu(help);
		/* TODO: This
		QAction *onlineHelp = new QAction(tr("Online Help"), this);
		help->addAction(onlineHelp);
        */
        QAction *keyCtrls = new QAction(tr("&Keyboard piloting controls"), this);
        help->addAction(keyCtrls);

		help->addSeparator();

		QAction *about = new QAction(tr("About Auto&Flight"), this);
		help->addAction(about);


	QWidget::connect(connectDrone, SIGNAL(triggered()), this, SLOT(attemptConnection()));
	QWidget::connect(flatTrim, SIGNAL(triggered()), this, SLOT(flatTrimActionTriggered()));
	QWidget::connect(calibMagneto, SIGNAL(triggered()), this, SLOT(calibrateMagnetometerActionTriggered()));
	QWidget::connect(flightSettings, SIGNAL(triggered()), this, SLOT(showDroneConfigDialog()));

	QWidget::connect(controlConfig, SIGNAL(triggered()), this, SLOT(showControlConfigDialog()));
	QWidget::connect(imgProcEdit, SIGNAL(triggered()), this, SLOT(launchImageProcessingPipelineEditor()));

	QWidget::connect(toggleHUD, SIGNAL(triggered(bool)), this, SLOT(toggleHUD(bool)));

    QWidget::connect(keyCtrls, SIGNAL(triggered()), this, SLOT(showDefaultKeyboardControls()));
    QWidget::connect(about, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

QWidget *AFMainWindow::createVerticalToolbar()
{
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setSpacing(15);

	QWidget *panel = new QWidget();
	panel->setLayout(layout);
	panel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

	Connection *c = new Connection();
	QObject::connect(c, SIGNAL(droneConnectionRequested()), this, SLOT(attemptConnection()));
    QObject::connect(this, SIGNAL(connectionLostSignal()), c, SLOT(connectionLost()));
    QObject::connect(this, SIGNAL(connectionEstablishedSignal()), c, SLOT(connectionEstablished()));
	layout->addWidget(c);

	Status *s = new Status();
    QObject::connect(this, SIGNAL(statusUpdateAvailableSignal(int)), s, SLOT(statusUpdateAvailable(int)));
	layout->addWidget(s);

    if(_af->bebop())
    {
        GPS *gps = new GPS();
        QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), gps, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));
        layout->addWidget(gps);

		if(_af->mavlink())
		{
			FlightPlan *fp = new FlightPlan();
			QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), fp, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));
			QObject::connect(this, SIGNAL(flightPlanAvailableSignal(std::string)), fp, SLOT(flightPlanAvailable(std::string)));
			layout->addWidget(fp);
		}
    }

	layout->addStretch();

	QPushButton *launchMap = new QPushButton("Map View");
	//launchMap->setIconSize(QSize(200, 50));
	QObject::connect(launchMap, SIGNAL(clicked()), this, SLOT(launch3DMapView()));
	layout->addWidget(launchMap);

	QPushButton *launchSessionViewer = new QPushButton("Session Viewer");
	//launchSessionViewer->setIcon(QIcon(QPixmap::fromImage(QImage(":/resources/sessionviewer.png"))));
	//launchSessionViewer->setIconSize(QSize(200, 50));
	QObject::connect(launchSessionViewer, SIGNAL(clicked()), this, SLOT(launchSessionViewerDialog()));
	layout->addWidget(launchSessionViewer);

	QPushButton *launchAutoScript = new QPushButton();
	launchAutoScript->setIcon(QIcon(QPixmap::fromImage(QImage(":/resources/autoscript.png"))));
	launchAutoScript->setIconSize(QSize(200, 50));
	QObject::connect(launchAutoScript, SIGNAL(clicked()), this, SLOT(launchAutoScriptIDESlot()));
	layout->addWidget(launchAutoScript);

	return panel;
}

QWidget *AFMainWindow::createHorizontalToolbar()
{
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setSpacing(5);

	QWidget *panel = new QWidget();
	panel->setLayout(layout);
	panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	layout->addStretch();

	Orientation *o = new Orientation();
	layout->addWidget(o);
	QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), o, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));

	Altitude *a = new Altitude();
	layout->addWidget(a);
	QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), a, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));

	Speed *s = new Speed();
	layout->addWidget(s);
	QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), s, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));

	Battery *b = new Battery();
	layout->addWidget(b);
	QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), b, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));

	SignalStrength *ss = new SignalStrength();
	layout->addWidget(ss);
	QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), ss, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));

	layout->addStretch();

	return panel;
}

void AFMainWindow::attemptConnection()
{
	if(!_af->attemptConnectionToDrone())
	{
		QMessageBox::warning(this, tr("Could not connect"), tr("<qt>AutoFlight could not connect to the drone. Please make sure that you are connected to it over WiFi and try again.<br><br><b>Note (AR.Drone 2.0 only):</b><br>It may be necessary to reset the AR.Drone by pressing the button under the battery tray (leaving the battery plugged in but outside the tray).</qt>"));
	}
}

void AFMainWindow::handleConnectionLost()
{
	videoPanel->connectionLost();
}

void AFMainWindow::toggleHUD(bool showHUD)
{
	videoPanel->showHUD(showHUD);

	if(showHUD)
	{
		grid->removeWidget(videoPanel);
		grid->removeWidget(horizontalToolbar);
		grid->removeWidget(verticalToolbar);
		grid->addWidget(videoPanel, 0, 0, 2, 2);
	}
	else
	{
		grid->removeWidget(videoPanel);
		grid->addWidget(horizontalToolbar, 1, 0, 1, 1);
		grid->addWidget(videoPanel, 0, 0, 1, 1);
		grid->addWidget(verticalToolbar, 0, 1, 2, 1);
	}

	horizontalToolbar->setVisible(!showHUD);
	verticalToolbar->setVisible(!showHUD);
}

void AFMainWindow::launchAutoScriptIDESlot(std::string file, std::vector<std::string> args)
{
	if(_asWindow == NULL)
	{
		_asWindow = new ASMainWindow(_af->asengine(), this);
		if(file != "")
		{
			_asWindow->openFile(file);
		}
		if(args.size() > 0)
		{
			_asWindow->setScriptArgs(args);
		}
        QObject::connect(_asWindow, SIGNAL(processedFrameAvailableSignal(QImage)), this, SLOT(processedFrameAvailable(QImage)));
	}

	_asWindow->show();
}

void AFMainWindow::launch3DMapView()
{
	if(_map == NULL)
	{
		_map = new Map3D();
		QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), _map, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));
	}
	_map->show();
}

void AFMainWindow::launchSessionViewerDialog()
{
	if(_sessionViewerWindow == NULL)
	{
		_sessionViewerWindow = new SessionViewer(this);
	}

	_sessionViewerWindow->show();
}

void AFMainWindow::launchImageProcessingPipelineEditor()
{
	if(_imgProc == NULL)
	{
		_imgProc = new ImgProcMainWindow(this);
	}

	_imgProc->show();
}


void AFMainWindow::loadDroneConfiguration()
{
    GenericSettings settings = SettingsFileIO::loadSettings(defaultFlightConfig, "flightconfig");
    SettingsHelper::applyFlightSettings(_af->drone(), settings);

    if(_af->bebop())
    {
        GenericSettings bebopvideosettings = SettingsFileIO::loadSettings(defaultBebopVideoSettings, "bebopvideo");
        SettingsHelper::applyBebopVideoSettings(_af->bebop(), bebopvideosettings);
    }
}

void AFMainWindow::resetBebopSettings()
{
    if(_af->bebop())
    {
        boost::thread([=]()
        {
            _af->bebop()->resetSettings();
            boost::this_thread::sleep(boost::posix_time::milliseconds(2500));
            loadDroneConfiguration();
        });
    }
}

void AFMainWindow::showDroneConfigDialog()
{
	GenericSettings settings = SettingsFileIO::loadSettings(defaultFlightConfig, "flightconfig");

	SettingsDialog *dialog = new SettingsDialog(settings, this);

	QObject::connect(dialog, &SettingsDialog::applied, [=]() {
		GenericSettings newSettings = dialog->getSettings();
		SettingsFileIO::saveSettings(newSettings, "flightconfig");
		SettingsHelper::applyFlightSettings(_af->drone(), newSettings);
	});

	dialog->exec();

	delete dialog;
}

void AFMainWindow::launchBebopVideoSettings()
{
    GenericSettings settings = SettingsFileIO::loadSettings(defaultBebopVideoSettings, "bebopvideo");

    SettingsDialog *dialog = new SettingsDialog(settings, this);

    QObject::connect(dialog, &SettingsDialog::applied, [=]() {
        GenericSettings newSettings = dialog->getSettings();
        SettingsFileIO::saveSettings(newSettings, "bebopvideo");
        if(_af->bebop())
        {
            SettingsHelper::applyBebopVideoSettings(_af->bebop(), newSettings);
        }
    });

    dialog->exec();

    delete dialog;
}

void AFMainWindow::showControlConfigDialog()
{
	bool invalidController = true;
	bool cancel = false;
	unsigned int controllerID = 0;

	while(invalidController && !cancel)
	{
		SelectController sc(this);
		sc.exec();

		if(sc.result() == QDialog::Accepted)
		{
			if(sc.getSelectedDeviceID() == -1)
			{
				QMessageBox::warning(this, tr("Error"), tr("Please select a device."));
				invalidController = true;
			}
			else
			{
				invalidController = false;
				controllerID = (unsigned int) sc.getSelectedDeviceID();
			}
		}
		else
		{
			cancel = true;
		}
	}

	if(!cancel)
	{
		ConfigureControls cc(controllerID, _manualcontrol->getControllerConfiguration(), this);
		cc.exec();

		if(cc.result() == QDialog::Accepted)
		{
			_manualcontrol->setControllerConfiguration(cc.getControllerConfiguration());
			ControllerConfigurationFileIO::saveControllerConfiguration(cc.getControllerConfiguration());
		}
	}
}

bool AFMainWindow::eventFilter(QObject *watched, QEvent* e)
{
	if(e->type() == QEvent::KeyPress)
	{
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);

		_manualcontrol->processKeyPress(ke);
	}
	else if(e->type() == QEvent::KeyRelease)
	{
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);

		_manualcontrol->processKeyRelease(ke);
	}

	return false;
}

void AFMainWindow::closeEvent(QCloseEvent *event)
{
	_manualcontrol->stopUpdateLoop();

	_af->sessionrecorder()->addEvent("ProgramExit");

	_af->drone()->removeNavdataListener(this);
	if(_af->fpvdrone())
	{
		_af->fpvdrone()->removeVideoListener(this);
		//_af->fpvdrone()->removeVideoListener(_imgProcTest); //TODO: this is only a test
	}
	_manualcontrol->removeControllerInputListener(this);
	_af->drone()->removeConnectionStatusListener(this);

	_imgProcTest->stopProcessing();
	delete _imgProcTest;

	_af->saveSession();
}

void AFMainWindow::flatTrimActionTriggered()
{
	if(!drone_flattrim(_af->drone()))
	{
		showMessage(tr("Not connected").toStdString());
	}
	else
	{
		showMessage(tr("Performing Flat Trim").toStdString());
	}
}

void AFMainWindow::calibrateMagnetometerActionTriggered()
{
	if(_af->ardrone2())
	{
		if(!drone_calibmagneto(_af->ardrone2()))
		{
			showMessage(tr("Not connected").toStdString());
		}
		else
		{
			showMessage(tr("Calibrating Magnetometer").toStdString());
		}
	}
	else
	{
		showMessage(tr("Automatic magnetometer calibration not supported").toStdString());
	}
}

void AFMainWindow::downloadBebopMedia()
{
    static string saveDir = _af->getHomeDirectory() + "AutoFlightSaves/Media/";
    static bool eraseOnDrone = false;

    if(_af->bebop() == nullptr)
    {
        return;
    }

    if(_af->bebop()->isConnected())
    {
        // Dialog

        BebopMediaDownload bmd(saveDir, eraseOnDrone, this);
        bmd.exec();

        if(bmd.result() != QDialog::Accepted)
        {
            return;
        }

        saveDir = bmd.getSavePath();
        eraseOnDrone = bmd.eraseOnDrone();

        if(_bebopMediaDownload_busy == nullptr)
        {
            _bebopMediaDownload_busy = new QProgressDialog(this);
            _bebopMediaDownload_busy->setCancelButton(0);
            _bebopMediaDownload_busy->setWindowFlags(_bebopMediaDownload_busy->windowFlags() & ~Qt::WindowCloseButtonHint);
            _bebopMediaDownload_busy->setWindowTitle(tr("Downloading media files..."));
            _bebopMediaDownload_busy->setMinimum(0);
            _bebopMediaDownload_busy->setMaximum(0);
            QObject::connect(this, SIGNAL(bebopMediaDownloadFinishedSignal()), _bebopMediaDownload_busy, SLOT(cancel()));
        }

        // Download media over FTP

        boost::filesystem::create_directories(saveDir);

        boost::thread([this]() {
            bebopftp::downloadMedia(_af->bebop()->getIP(), saveDir, eraseOnDrone);
            Q_EMIT bebopMediaDownloadFinishedSignal();
        });

        // Show busy dialog
        _bebopMediaDownload_busy->exec();
    }
    else
    {
        showMessage(tr("Not connected").toStdString());
    }
}

void AFMainWindow::showDefaultKeyboardControls()
{
    QDialog helpDialog(this);
    helpDialog.setWindowTitle(tr("Drone Keyboard Controls"));

    QVBoxLayout *l = new QVBoxLayout();
    l->setAlignment(Qt::AlignTop);
    helpDialog.setLayout(l);

    QLabel *controls = new QLabel();
    controls->setPixmap(QPixmap::fromImage(QImage(":/resources/af_controls.png")));
    l->addWidget(controls);

    QLabel *website = new QLabel(QString("Full documentation at <a href=\"http://electronics.kitchen/docs/autoflight\">electronics.kitchen/docs/autoflight</a>"));
    website->setOpenExternalLinks(true);
    website->setStyleSheet("font: 12pt; ");
    website->setAlignment(Qt::AlignCenter);
    l->addWidget(website);

    helpDialog.layout()->setSizeConstraint(QLayout::SetFixedSize);
    helpDialog.exec();
}

void AFMainWindow::showAboutDialog()
{
	QDialog aboutDialog(this);
	aboutDialog.setWindowTitle(tr("About AutoFlight"));
	aboutDialog.setMinimumHeight(200);

	QHBoxLayout *l = new QHBoxLayout();
	aboutDialog.setLayout(l);

	QVBoxLayout *left = new QVBoxLayout();
	left->setAlignment(Qt::AlignTop);
	l->addLayout(left);

	QLabel *icon = new QLabel();
	icon->setPixmap(QPixmap::fromImage(QImage(":/resources/icon.png").scaled(75, 75)));
	icon->setFixedSize(75, 75);
	left->addWidget(icon);

	QVBoxLayout *right = new QVBoxLayout();
	l->addLayout(right);

	QLabel *autoflight = new QLabel(tr("About AutoFlight"));
	autoflight->setStyleSheet("font-size: 24px");
	right->addWidget(autoflight);

	QLabel *version = new QLabel(tr("Version ").append(QString::fromStdString(autoflight::SOFTWARE_VERSION)));
	right->addWidget(version);

	QLabel *build = new QLabel(tr("Build ").append(QString::fromStdString(autoflight::BUILD_NUMBER)));
	right->addWidget(build);

	QLabel *qtVersion = new QLabel(tr("Qt ").append(qVersion()));
	right->addWidget(qtVersion);

	right->addStretch();

	QChar copyrightSymbol(0x00A9);
	QLabel *copyright = new QLabel(QString("Copyright ").append(copyrightSymbol).append(" 2013 - 2016 Lukas Lao Beyer"));
	right->addWidget(copyright);

	QLabel *website = new QLabel(QString("<a href=\"http://electronics.kitchen/autoflight\">electronics.kitchen/autoflight</a>"));
	website->setOpenExternalLinks(true);
	right->addWidget(website);

	aboutDialog.exec();
}
