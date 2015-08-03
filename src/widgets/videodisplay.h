#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QGLWidget>

#include <types.h>

#include <memory>

#include "../input/controllerinput.h"
#include "../qinterface/qstatuslistener.h"

class VideoDisplay : public QGLWidget, public QStatusListener
{
	Q_OBJECT

	public:
		VideoDisplay(QWidget *parent = 0, bool bebop = false);

		void setCurrentFrame(const QImage &img);
		void setInputData(double prollInput, double ppitchInput, double paltitudeInput, double pyawInput);
		void setNavdata(double yaw, double pitch, double roll, double altitude, double charge, double speed);

		void setMaximized(bool maximize);
		void showHUD(bool show);
		void connectionLost();

		void setRecordingVideo(bool rec);
		void setRecordingNavdata(bool rec);

	public Q_SLOTS:
		void navdataAvailable(std::shared_ptr<const drone::navdata> nd);
		void controllerInputAvailable(std::shared_ptr<const ControllerInput> in);
        void statusUpdateAvailable(int status);

	protected:
		void paintEvent(QPaintEvent *e);

	private:
		double to360Format(double number, bool clockwise);
		void drawTriangle(QPainter &p, int size, int x, int y, int rotate);

		QImage _img;
        bool _bebop = false;
		bool _maximize = false;
		bool _hud = false;
		bool _connectionLost = false;
		bool _rec_vid = false;
		bool _rec_nav = false;
        bool _armed = false;
        bool _show_gps_indicator = false;
        bool _gps_lock = false;
        int _gps_sats = 0;

		double yaw = 0, pitch = 0, roll = 0, altitude = 0, charge = 0, link = 0, speed = 0, rollInput = 0, pitchInput = 0, altitudeInput = 0, yawInput = 0;
};

#endif
