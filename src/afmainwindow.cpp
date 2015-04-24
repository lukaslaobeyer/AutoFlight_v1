#include "afmainwindow.h"

#include "afconstants.h"

#include "dronehelper.h"

#include "widgets/orientation.h"
#include "widgets/altitude.h"
#include "widgets/speed.h"
#include "widgets/battery.h"
#include "widgets/signalstrength.h"
#include "widgets/connection.h"

#include "dialogs/selectcontroller.h"
//#include "dialogs/configurecontrols.h"
#include "dialogs/welcomedialog.h"
//#include "dialogs/dronesettings.h"

//#include "tools/controllerconfigurationfileio.h"
//#include "tools/ardroneconfigurationfileio.h"

#include <QtWidgets>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>

// When on a netbook with limited space, use these values as minimum screen size:
#define MIN_WIDTH 1024
#define MIN_HEIGHT 600
// On a normal sized screen, minimum screen size should be:
#define PREF_WIDTH 1270
#define PREF_HEIGHT 775

#define CONFIRMATION_TIMEOUT 1250

using namespace std;

void AFMainWindow::setWindowAttributes()
{
	setWindowTitle("AutoFlight");
	setWindowIcon(QIcon(":/resources/icon.png"));
	setMinimumSize(PREF_WIDTH, PREF_HEIGHT); //TODO: Netbook support
}

AFMainWindow::AFMainWindow(AutoFlight *af, QWidget *parent) : QMainWindow(parent)
{
	_af = af;

	// Register meta types
	qRegisterMetaType<shared_ptr<const drone::navdata>>();

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
	msg->setStyleSheet("background: rgba(30, 30, 30, 0.85); font-size: 24px; color: #FFFFFF; border-radius: 10px;");
	msg->hide();

	videoPanel = new VideoDisplay();
	//videoPanel->setAlignment(Qt::AlignCenter);
	grid->addWidget(videoPanel, 0, 0, 1, 1);

	videoPanel->setCurrentFrame(QImage(":/resources/autoflight.png"));

	horizontalToolbar = createHorizontalToolbar();
	verticalToolbar = createVerticalToolbar();

	grid->addWidget(horizontalToolbar, 1, 0, 1, 1);
	grid->addWidget(verticalToolbar, 0, 1, 2, 1);

	//_imgProcTest = new ImageProcessor();

	_af->drone()->addNavdataListener(this);
	if(_af->fpvdrone())
	{
		_af->fpvdrone()->addVideoListener(this);
	}
	//TODO: Controller input; _af->ardrone()->addControllerInputListener(this);
	_af->drone()->addConnectionStatusListener(this);
	//_af->ardrone()->addVideoListener(_imgProcTest);


	QObject::connect(this, SIGNAL(videoFrameAvailableSignal(QImage)), this, SLOT(videoFrameAvailable(QImage)));
	QObject::connect(this, SIGNAL(connectionLostSignal()), this, SLOT(handleConnectionLost()));
	QObject::connect(this, SIGNAL(navdataAvailableSignal(std::shared_ptr<const drone::navdata>)), videoPanel, SLOT(navdataAvailable(std::shared_ptr<const drone::navdata>)));
	QObject::connect(this, SIGNAL(controllerInputAvailableSignal(ControllerInput *)), videoPanel, SLOT(controllerInputAvailable(ControllerInput *)));
	QObject::connect(this, SIGNAL(controllerInputAvailableSignal(ControllerInput *)), this, SLOT(controllerInputAvailableSlot(ControllerInput *)));

	installEventFilter(this);

	//TODO: Controller input
	/*
	ControllerConfiguration *cc = ControllerConfigurationFileIO::loadControllerConfiguration();
	if(cc != nullptr)
	{
		_af->ardrone()->setControllerConfiguration(cc);
	}
	*/

	showFirstRunInfoIfRequired();
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
	msg->setText(QString::fromStdString(message));
	msg->setGeometry((int) (size().width() / 2 - 200), (int) (size().height() / 2 - 25), 400, 50);
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

void AFMainWindow::connectionLost()
{
	Q_EMIT connectionLostSignal();
}

void AFMainWindow::connectionEstablished()
{
	//_imgProcTest->startProcessing();
}

void AFMainWindow::videoFrameAvailable(cv::Mat f)
{
	// Convert cv::Mat to QImage
	QImage img(f.data, f.cols, f.rows, f.step, QImage::Format_RGB888);
	img = img.rgbSwapped();

	Q_EMIT videoFrameAvailableSignal(img);
}

/*void AFMainWindow::controllerInputAvailable(ControllerInput *in)
{
	Q_EMIT controllerInputAvailableSignal(in);
}*/

void AFMainWindow::videoFrameAvailable(QImage f)
{
	videoPanel->setMaximized(true);
	videoPanel->setCurrentFrame(f);
}

void AFMainWindow::createMenuBar() {
	QMenu *drone = new QMenu(tr("&Drone"));
	menuBar()->addMenu(drone);
		
		QAction *connectDrone = new QAction(tr("&Connect to drone"), this);
		drone->addAction(connectDrone);
		
		/* TODO: This
		QAction *connectArduino = new QAction(tr("Connect to Arduino"), this);
		drone->addAction(connectArduino);
		*/
		
		drone->addSeparator();
		
		QAction *flatTrim = new QAction(tr("&Flat Trim"), this);
		drone->addAction(flatTrim);
		
		QAction *calibMagneto = new QAction(tr("Calibrate &Magnetometer"), this);
		drone->addAction(calibMagneto);
		
		drone->addSeparator();

		QAction *flightSettings = new QAction(tr("Flight &Settings"), this);
		drone->addAction(flightSettings);

		drone->addSeparator();
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
		tools->addAction(imgProcEdit);

		/* TODO: This
		QAction *controlInfo = new QAction(tr("Controller Information"), this);
		tools->addAction(controlInfo);
		
		tools->addSeparator();
		
		QAction *configEditor = new QAction(tr("Configuration Editor"), this);
		tools->addAction(configEditor);
		
		tools->addSeparator();
		
		QAction *gpsViewer = new QAction(tr("GPS Viewer"), this);
		tools->addAction(gpsViewer);
		*/
	QMenu *view = new QMenu(tr("&View"));
	menuBar()->addMenu(view);
		QAction *toggleHUD = new QAction(tr("Head-&Up Display"), this);
		toggleHUD->setCheckable(true);
		toggleHUD->setShortcut(QKeySequence::fromString("F5"));
		view->addAction(toggleHUD);
		
		/* TODO: This
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
		
		help->addSeparator();
		*/
		QAction *about = new QAction(tr("About Auto&Flight"), this);
		help->addAction(about);

	QWidget::connect(connectDrone, SIGNAL(triggered()), this, SLOT(attemptConnection()));
	QWidget::connect(flatTrim, SIGNAL(triggered()), this, SLOT(flatTrimActionTriggered()));
	QWidget::connect(calibMagneto, SIGNAL(triggered()), this, SLOT(calibrateMagnetometerActionTriggered()));
	QWidget::connect(flightSettings, SIGNAL(triggered()), this, SLOT(showDroneConfigDialog()));

	QWidget::connect(controlConfig, SIGNAL(triggered()), this, SLOT(showControlConfigDialog()));
	QWidget::connect(imgProcEdit, SIGNAL(triggered()), this, SLOT(launchImageProcessingPipelineEditor()));

	QWidget::connect(toggleHUD, SIGNAL(triggered(bool)), this, SLOT(toggleHUD(bool)));

	QWidget::connect(about, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

QWidget *AFMainWindow::createVerticalToolbar()
{
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setSpacing(5);
	
	QWidget *panel = new QWidget();
	panel->setLayout(layout);
	panel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	
	Connection *c = new Connection();
	QObject::connect(c, SIGNAL(droneConnectionRequested()), this, SLOT(attemptConnection()));
	layout->addWidget(c);
	
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
	QObject::connect(launchAutoScript, SIGNAL(clicked()), this, SLOT(launchAutoScriptIDE()));
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
		QMessageBox::warning(this, tr("Could not connect"), tr("<qt>AutoFlight could not connect to the AR.Drone. Please make sure that you are connected to it over WiFi and try again.<br><br><b>Note:</b><br>It may be necessary to reset the AR.Drone by pressing the button under the battery tray (leaving the battery plugged in but outside the tray).</qt>"));
	}
}

/*void AFMainWindow::controllerInputAvailableSlot(ControllerInput *in)
{
	static bool prev_picture_button_state = false;		 // Needed to detect if the picture taking button
	bool current_picture_button_state = in->takePicture; // was pressed

	if(prev_picture_button_state == false && current_picture_button_state == true)
	{
		// Picture button pressed, show message:
		showMessage("Picture saved");
	}

	prev_picture_button_state = current_picture_button_state;
}*/

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

void AFMainWindow::launchAutoScriptIDE()
{
	if(_asWindow == NULL)
	{
		_asWindow = new ASMainWindow(_af->asengine(), this);
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

/*void AFMainWindow::showDroneConfigDialog()
{
	ARDroneConfiguration *conf = ARDroneConfigurationFileIO::loadARDroneConfiguration(0);
	DroneSettings ds(conf, this);
	ds.exec();

	ARDroneConfiguration *new_conf = ds.getConfiguration();

	if(new_conf != nullptr)
	{
		ARDroneConfigurationFileIO::saveARDroneConfiguration(new_conf, 0);
		_af->ardrone()->drone_setConfiguration(*new_conf);
	}
}*/

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
				controllerID = sc.getSelectedDeviceID();
			}
		}
		else
		{
			cancel = true;
		}
	}

	if(!cancel)
	{
		//TODO: Controller input
		/*ConfigureControls cc(controllerID, _af->ardrone()->getControllerConfiguration(), this);
		cc.exec();

		if(cc.result() == QDialog::Accepted)
		{

			_af->ardrone()->setControllerConfiguration(cc.getControllerConfiguration());
			ControllerConfigurationFileIO::saveControllerConfiguration(cc.getControllerConfiguration());
		}*/
	}
}

void AFMainWindow::clearConfirmationFlags()
{
	_confirmEmergency = false;
	_confirmFlip = false;
}

bool AFMainWindow::eventFilter(QObject *watched, QEvent* e)
{
	//TODO: Good controller input
	if(e->type() == QEvent::KeyPress)
	{
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);

		if(!ke->isAutoRepeat())
		{
			if(_af->drone()->isConnected())
			{
				switch(ke->key())
				{
				case Qt::Key_T:
					// Take off or land
					if(_af->drone()->isFlying())
					{
						showMessage(tr("Landing").toStdString());
						drone_land(_af->drone());
					}
					else
					{
						showMessage(tr("Taking off").toStdString());
						drone_takeOff(_af->drone());
					}

					break;
				case Qt::Key_F:
					if(!_confirmFlip)
					{
						showMessage(tr("[Flip] Are you sure?").toStdString());
						QTimer::singleShot(CONFIRMATION_TIMEOUT, this, SLOT(clearConfirmationFlags()));
						_confirmFlip = true;
					}
					else
					{
						hideMessages();
						showMessage(tr("Performing flip!").toStdString());
						if(_af->ardrone2())
						{
							drone_flip(_af->ardrone2());
						}
						else if(_af->bebop())
						{
							drone_flip(_af->bebop());
						}
						else
						{
							showMessage(tr("Flip for this drone not supported").toStdString());
						}
					}
					// Flip
					break;
				case Qt::Key_P:
					//TODO: Take Picture
					// _af->ardrone()->drone_takePicture();
					showMessage(tr("Picture saved").toStdString());
					break;
				case Qt::Key_R:
					//TODO: Start/stop Recording
					/*if(_af->ardrone()->drone_isRecording())
					{
						showMessage(tr("Stopped recording").toStdString());
					}
					else
					{
						showMessage(tr("Recording").toStdString());
					}
					_af->ardrone()->drone_toggleRecording();*/
					break;
				case Qt::Key_N:
					//TODO: Start/stop Recording Sensor Data
					/*if(_af->ardrone()->drone_isRecordingNavdata())
					{
						showMessage(tr("Stopped recording sensor data").toStdString());
					}
					else
					{
						showMessage(tr("Recording sensor data").toStdString());
					}
					_af->ardrone()->drone_toggleRecordingNavdata();*/
					break;
				case Qt::Key_V:
					// Change View
					showMessage(tr("Switching view").toStdString());
					if(_af->ardrone2())
					{
						drone_switchview(_af->ardrone2());
					}
					else if(_af->bebop())
					{
						drone_switchview(_af->bebop());
					}
					else
					{
						showMessage(tr("Switching view not supported by this drone").toStdString());
					}
					break;
				case Qt::Key_Y:
					if(!_confirmEmergency)
					{
						showMessage(tr("[Emergency] Are you sure?").toStdString());
						QTimer::singleShot(CONFIRMATION_TIMEOUT, this, SLOT(clearConfirmationFlags()));
						_confirmEmergency = true;
					}
					else
					{
						hideMessages();
						showMessage(tr("Emergency command sent").toStdString());
						drone_emergency(_af->drone());
					}
					// Emergency
					break;

				case Qt::Key_W:
					// Pitch forward
					drone_setPitchRel(_af->drone(), -0.6f);
					break;
				case Qt::Key_A:
					// Roll left
					drone_setRollRel(_af->drone(), -0.6f);
					break;
				case Qt::Key_S:
					// Pitch backwards
					drone_setPitchRel(_af->drone(), 0.6f);
					break;
				case Qt::Key_D:
					// Roll right
					drone_setRollRel(_af->drone(), 0.6f);
					break;

				case Qt::Key_I:
					// Ascend
					drone_setGazRel(_af->drone(), 0.6f);
					break;
				case Qt::Key_J:
					// Rotate counterclockwise
					drone_setYawRel(_af->drone(), 0.6f);
					break;
				case Qt::Key_K:
					// Descend
					drone_setGazRel(_af->drone(), -0.6f);
					break;
				case Qt::Key_L:
					// Rotate clockwise
					drone_setYawRel(_af->drone(), 0.6f);
					break;

				default:
					break;
				}
			}
			else
			{
				if(ke->key() >= Qt::Key_0 && ke->key() <= Qt::Key_Z)
				{
					showMessage("Not connected to AR.Drone");
				}
			}
		}
	}
	else if(e->type() == QEvent::KeyRelease)
	{
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);

		if(!ke->isAutoRepeat())
		{
			switch(ke->key())
			{
			case Qt::Key_W:
				// Stop forward pitch
				drone_setPitchRel(_af->drone(), 0);
				break;
			case Qt::Key_A:
				// Stop left roll
				drone_setRollRel(_af->drone(), 0);
				break;
			case Qt::Key_S:
				// Stop backward pitch
				drone_setPitchRel(_af->drone(), 0);
				break;
			case Qt::Key_D:
				// Stop right roll
				drone_setRollRel(_af->drone(), 0);
				break;

			case Qt::Key_I:
				// Stop ascending
				drone_setGazRel(_af->drone(), 0);
				break;
			case Qt::Key_J:
				// Stop rotating counterclockwise
				drone_setYawRel(_af->drone(), 0);
				break;
			case Qt::Key_K:
				// Stop descending
				drone_setGazRel(_af->drone(), 0);
				break;
			case Qt::Key_L:
				// Stop rotating clockwise
				drone_setYawRel(_af->drone(), 0);
				break;

			default:
				break;
			}
		}
	}

	return false;
}

void AFMainWindow::closeEvent(QCloseEvent *event)
{
	_af->sessionrecorder()->addEvent("ProgramExit");

	_af->drone()->removeNavdataListener(this);
	if(_af->fpvdrone())
	{
		_af->fpvdrone()->removeVideoListener(this);
	}
	//_af->ardrone()->removeControllerInputListener(this);
	_af->drone()->removeConnectionStatusListener(this);
	//_af->ardrone()->removeVideoListener(_imgProcTest);

	//_imgProcTest->stopProcessing();
	//delete _imgProcTest;

	_af->saveSession();
}

void AFMainWindow::flatTrimActionTriggered()
{
	//TODO: flat trim
	/*if(!_af->ardrone()->drone_flattrim())
	{
		showMessage(tr("Not connected").toStdString());
	}
	else
	{
		showMessage(tr("Performing Flat Trim").toStdString());
	}*/
}

void AFMainWindow::calibrateMagnetometerActionTriggered()
{
	//TODO: magneto calibration
	/*if(!_af->ardrone()->drone_calibmagneto())
	{
		showMessage(tr("Not connected").toStdString());
	}
	else
	{
		showMessage(tr("Calibrating Magnetometer").toStdString());
	}*/
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

	right->addStretch();

	QChar copyrightSymbol(0x00A9);
	QLabel *copyright = new QLabel(QString("Copyright ").append(copyrightSymbol).append(" 2013, 2014 Lukas Lao Beyer"));
	right->addWidget(copyright);

	QLabel *website = new QLabel(QString("<a href=\"http://lbpclabs.com/autoflight\">electronics.kitchen/autoflight</a>"));
	website->setOpenExternalLinks(true);
	right->addWidget(website);

	aboutDialog.exec();
}
