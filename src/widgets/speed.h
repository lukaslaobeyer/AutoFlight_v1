#ifndef SPEED_H
#define SPEED_H

#include "rotatableimageview.h"
#include "../qinterface/qnavdatalistener.h"
#include <interface/inavdatalistener.h>
#include <QWidget>
#include <QLabel>

class Speed : public QWidget, public INavdataListener
{
	Q_OBJECT
	
	public:
		explicit Speed(QWidget *parent = 0);
	public Q_SLOTS:
		void navdataAvailable(std::shared_ptr<const drone::navdata> navdata);
	private:
		RotatableImageView *speedometerNeedle;
		QLabel *speedometerLabel;
};

#endif
