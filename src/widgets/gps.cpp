#include "gps.h"

#include "titledbox.h"

#include <QtWidgets>
#include <drones/bebop/types.h>

#include <memory>

GPS::GPS(QWidget *parent) : QWidget(parent)
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

            fixStatus = new QLabel("<qt>Fix status: <b>N/A</b></qt>");
            fixStatus->setAlignment(Qt::AlignCenter);
            fixStatus->setMargin(2);
            fixStatus->setStyleSheet("font: 12pt;");
            contentLayout->addWidget(fixStatus);

            latitude = new QLabel("latitude");
            latitude->setAlignment(Qt::AlignCenter);
            latitude->setMargin(2);
            latitude->setStyleSheet("font: 10pt;");
            contentLayout->addWidget(latitude);
            latitude->setVisible(false);

            longitude = new QLabel("longitude");
            longitude->setAlignment(Qt::AlignCenter);
            longitude->setMargin(2);
            longitude->setStyleSheet("font: 10pt;");
            contentLayout->addWidget(longitude);
            longitude->setVisible(false);

            altitude = new QLabel("altitude");
            altitude->setAlignment(Qt::AlignCenter);
            altitude->setMargin(2);
            altitude->setStyleSheet("font: 10pt;");
            contentLayout->addWidget(altitude);
            altitude->setVisible(false);

        TitledBox *box = new TitledBox(tr("GPS"), content);
        layout->addWidget(box);

    QGraphicsDropShadowEffect *dropShadow = new QGraphicsDropShadowEffect();
    dropShadow->setBlurRadius(6);
    dropShadow->setColor(QColor(0, 0, 0));
    dropShadow->setOffset(0, 0);

    setGraphicsEffect(dropShadow);

    setLayout(layout);
}

void GPS::navdataAvailable(std::shared_ptr<const drone::navdata> uncast_navdata)
{
    std::shared_ptr<const bebop::navdata> navdata = std::static_pointer_cast<const bebop::navdata>(uncast_navdata);

    if(navdata->gps_fix)
    {
        fixStatus->setText("<qt>Fix status: <b>Available</b></qt>");
        longitude->setVisible(true);
        latitude->setVisible(true);
        altitude->setVisible(true);

        latitude->setText(QString::number(navdata->latitude) + "º N");
        longitude->setText(QString::number(navdata->longitude) + "º E");
        altitude->setText(QString::number(navdata->gps_altitude) + "m");
    }
    else
    {
        fixStatus->setText("<qt>Fix status: <b>Unavailable</b></qt>");
        longitude->setVisible(false);
        latitude->setVisible(false);
        altitude->setVisible(false);
    }
}