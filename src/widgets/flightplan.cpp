#include <drones/bebop/types.h>
#include "flightplan.h"
#include "titledbox.h"

FlightPlan::FlightPlan(QWidget *parent) : QWidget(parent)
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

    status = new QLabel(tr("<qt><b>No flight plan</b></qt>"));
    status->setAlignment(Qt::AlignCenter);
    status->setMargin(2);
    status->setStyleSheet("font: 10pt;");
    contentLayout->addWidget(status);

    start = new QPushButton(tr("Waiting for GPS lock"));
    start->setStyleSheet("font: 12pt; padding: 4px");
    start->setVisible(false);
    start->setEnabled(false);
    contentLayout->addWidget(start);

    TitledBox *box = new TitledBox(tr("Flight Planning"), content);
    layout->addWidget(box);

    QGraphicsDropShadowEffect *dropShadow = new QGraphicsDropShadowEffect();
    dropShadow->setBlurRadius(6);
    dropShadow->setColor(QColor(0, 0, 0));
    dropShadow->setOffset(0, 0);

    setGraphicsEffect(dropShadow);

    setLayout(layout);
}

void FlightPlan::navdataAvailable(std::shared_ptr<const drone::navdata> uncast_navdata)
{
    std::shared_ptr<const bebop::navdata> navdata = std::static_pointer_cast<const bebop::navdata>(uncast_navdata);

    if(navdata->gps_fix)
    {
        start->setText(tr("Execute flight plan"));
        start->setEnabled(true);
    }
}

void FlightPlan::flightPlanAvailable(std::string path)
{
    status->setText(tr("<qt><b>Flight plan available</b></qt>"));
    start->setVisible(true);
}