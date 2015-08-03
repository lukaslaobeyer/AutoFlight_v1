#ifndef AUTOFLIGHT_GPS_H
#define AUTOFLIGHT_GPS_H

#include <QWidget>
#include <QLabel>

#include "../qinterface/qnavdatalistener.h"

class GPS : public QWidget, public QNavdataListener
{
    Q_OBJECT

    public:
        explicit GPS(QWidget *parent = 0);
    public Q_SLOTS:
        void navdataAvailable(std::shared_ptr<const drone::navdata> navdata);
    private:
        QLabel *fixStatus = nullptr;
        QLabel *latitude = nullptr;
        QLabel *longitude = nullptr;
        QLabel *altitude = nullptr;
        QLabel *sat_cnt = nullptr;
};

#endif
