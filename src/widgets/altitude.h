#ifndef ALTITUDE_H
#define ALTITUDE_H

#include "../qinterface/qnavdatalistener.h"
#include "rtgraph.h"
#include <QWidget>

class Altitude : public QWidget, public QNavdataListener
{
	Q_OBJECT
	
	public:
		explicit Altitude(QWidget *parent = 0);
	public Q_SLOTS:
		void navdataAvailable(std::shared_ptr<const drone::navdata> nd);
	private:
		RTGraph *graph;
};

#endif
