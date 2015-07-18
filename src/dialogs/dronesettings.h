#ifndef DRONESETTINGS_H
#define DRONESETTINGS_H

#include <QWidget>
#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QCheckBox>

#include <drone.h>

class DroneSettings : public QDialog
{
	Q_OBJECT

	public:
		explicit DroneSettings(drone::config config, QWidget *parent = 0);

		drone::config getConfiguration();
	private Q_SLOTS:
		void valueChanged(int val);
		void valueChanged(double val);

		void handleAccept();
	private:
		drone::config _config{drone::limits{0, 0, 0, 0}, false, false};

		QSlider *alt_max_slider, *tilt_max_slider, *vz_max_slider, *vyaw_max_slider;
		QCheckBox *outdoor_ckbx;
		QDoubleSpinBox *alt_max_spinner, *vz_max_spinner;
		QSpinBox *tilt_max_spinner, *vyaw_max_spinner;
};

#endif
