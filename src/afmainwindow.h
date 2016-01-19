#ifndef AFMAINWINDOW_H
#define AFMAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QWidget>
#include <QEvent>
#include <QTimer>
#include <QGridLayout>
#include <vector>
#include <memory>
#include <string>
#include <opencv2/opencv.hpp>
#include "autoflight.h"
#include "imgprocui/imgprocmainwindow.h"
#include "imgproc/imageprocessor.h"
#include "dialogs/sessionviewer.h"
#include "widgets/map3d.h"
#include "widgets/videodisplay.h"
#include "asmainwindow.h"
#include "input/manualcontrol.h"
#include "input/icontrollerinputlistener.h"
#include "soundalerts.h"

#include <drone.h>
#include <interface/inavdatalistener.h>
#include "qinterface/qnavdatalistener.h"
#include <interface/ivideolistener.h>
#include "qinterface/qvideolistener.h"

Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(std::shared_ptr<const drone::navdata>)
Q_DECLARE_METATYPE(std::shared_ptr<const ControllerInput>)

class ManualControl;

class AFMainWindow : public QMainWindow, public INavdataListener, public IStatusListener, public IConnectionStatusListener, public IVideoListener, public IControllerInputListener, public IFlightPlanListener, public QVideoListener
{
	Q_OBJECT

	public:
		explicit AFMainWindow(AutoFlight *af, QWidget *parent = 0);

		void navdataAvailable(std::shared_ptr<const drone::navdata> nd);
		void statusUpdateAvailable(int status);
		void videoFrameAvailable(cv::Mat f);
		void controllerInputAvailable(std::shared_ptr<const ControllerInput> in);
		void connectionLost();
		void connectionEstablished();
		void flightPlanAvailable(std::string flightplan);

		void showMessage(std::string msg);
		void launchAutoScriptIDE(std::string file = "", std::vector<std::string> args = {});
	public Q_SLOTS:
		void hideMessages();
		void flatTrimActionTriggered();
		void calibrateMagnetometerActionTriggered();
        void downloadBebopMedia();
        void showDefaultKeyboardControls();
		void showAboutDialog();
	protected:
		bool eventFilter(QObject *sender, QEvent *e);
	private:
		void setWindowAttributes();
		void closeEvent(QCloseEvent *);
		void createMenuBar();
		void showFirstRunInfoIfRequired();
		void loadDroneConfiguration();
		QWidget *createVerticalToolbar();
		QWidget *createHorizontalToolbar();

		QGridLayout *grid;

		QWidget *verticalToolbar;
		QWidget *horizontalToolbar;

		VideoDisplay *videoPanel;
		QLabel *msg;

		QTimer *_messageTimer = NULL;

		AutoFlight *_af = NULL;

		ASMainWindow *_asWindow = NULL;
		SessionViewer *_sessionViewerWindow = NULL;
		Map3D *_map = NULL;
		ImgProcMainWindow *_imgProc = NULL;

		std::unique_ptr<ManualControl> _manualcontrol;

        QProgressDialog *_bebopMediaDownload_busy = nullptr;

        uint64_t _lastProcessedFrameTime = 0;

		ImageProcessor *_imgProcTest = NULL;

		SoundAlerts _soundAlerts;
	private Q_SLOTS:
		void showMessageSlot(QString message);
		void attemptConnection();
		void showControlConfigDialog();
		void showDroneConfigDialog();
		void videoFrameAvailable(QImage f);
		void processedFrameAvailable(QImage f);
		void toggleHUD(bool showHUD);
		void resetBebopSettings();
		void launchBebopVideoSettings();
		void launchAutoScriptIDESlot(std::string file = "", std::vector<std::string> args = {});
		void launchSessionViewerDialog();
		void launch3DMapView();
		void launchImageProcessingPipelineEditor();
		void handleConnectionLost();

	Q_SIGNALS:
		void showMessageSignal(QString message);
		void navdataAvailableSignal(std::shared_ptr<const drone::navdata> nd);
        void statusUpdateAvailableSignal(int status);
		void videoFrameAvailableSignal(QImage frame);
		void controllerInputAvailableSignal(std::shared_ptr<const ControllerInput> in);
		void connectionLostSignal();
	    void connectionEstablishedSignal();
        void flightPlanAvailableSignal(std::string path);

		void bebopMediaDownloadFinishedSignal();
};

#endif
