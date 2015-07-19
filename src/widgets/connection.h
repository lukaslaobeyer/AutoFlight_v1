#ifndef CONNECTION_H
#define CONNECTION_H

#include <QWidget>
#include <QPushButton>

#include "../qinterface/qconnectionstatuslistener.h"

class Connection : public QWidget, public QConnectionStatusListener
{
	Q_OBJECT
	
	public:
		explicit Connection(QWidget *parent = 0);
	public Q_SLOTS:
		void connectionEstablished();
		void connectionLost();
	private:
		QPushButton *connectDrone;
	Q_SIGNALS:
		void droneConnectionRequested();
};

#endif
