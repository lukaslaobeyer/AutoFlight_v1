#include "connection.h"
#include "titledbox.h"
#include <QtWidgets>

#define ORIENTATION_WIDGET_HEIGHT 215

Connection::Connection(QWidget *parent) : QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setSpacing(0);
	layout->setMargin(0);
	
	setStyleSheet("padding: 0;");
	
		QVBoxLayout *contentLayout = new QVBoxLayout();
		contentLayout->setSpacing(2);
		contentLayout->setMargin(0);
		contentLayout->setContentsMargins(4, 4, 4, 4);
		
		QWidget *content = new QWidget();
		content->setLayout(contentLayout);
		
			connectDrone = new QPushButton(tr("Connect to Drone"));
			connectDrone->setStyleSheet("padding: 4px;");
			contentLayout->addWidget(connectDrone);
		
		TitledBox *box = new TitledBox(tr("Connection"), content);
		layout->addWidget(box);
		
	QGraphicsDropShadowEffect *dropShadow = new QGraphicsDropShadowEffect();
	dropShadow->setBlurRadius(6);
	dropShadow->setColor(QColor(0, 0, 0));
	dropShadow->setOffset(0, 0);
	
	setGraphicsEffect(dropShadow);
	
	setLayout(layout);

	connect(connectDrone, SIGNAL(clicked()), this, SIGNAL(droneConnectionRequested()));
}

void Connection::connectionEstablished()
{
	connectDrone->setDisabled(true);
}

void Connection::connectionLost()
{
	connectDrone->setDisabled(false);
}
