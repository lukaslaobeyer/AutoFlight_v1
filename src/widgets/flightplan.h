#ifndef AF_WIDGETS_FLIGHTPLAN_H
#define AF_WIDGETS_FLIGHTPLAN_H

#include <QtWidgets>
#include <drone.h>

class FlightPlan : public QWidget
{
    Q_OBJECT

    public:
        explicit FlightPlan(QWidget *parent = 0);
    public Q_SLOTS:
        void navdataAvailable(std::shared_ptr<const drone::navdata> navdata);
        void flightPlanAvailable(std::string path);
    private:
        QLabel *status;
        QPushButton *start;
};

#endif