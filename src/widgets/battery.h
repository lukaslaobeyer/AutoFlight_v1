#ifndef BATTERY_H
#define BATTERY_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include "../qinterface/qnavdatalistener.h"

class Battery : public QWidget, public QNavdataListener
{
	Q_OBJECT
	
	public:
		explicit Battery(QWidget *parent = 0);
	public Q_SLOTS:
		void navdataAvailable(std::shared_ptr<const drone::navdata> navdata);
	private:
		QProgressBar *batteryLevel;
		QLabel *batteryLevelLabel;
};

#endif
