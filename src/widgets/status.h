#ifndef AUTOFLIGHT_STATUS_H
#define AUTOFLIGHT_STATUS_H

#include <QWidget>
#include <QLabel>

#include "../qinterface/qstatuslistener.h"

class Status : public QWidget, public QStatusListener
{
    Q_OBJECT

    public:
        explicit Status(QWidget *parent = 0);
    public Q_SLOTS:
        void statusUpdateAvailable(int status);
    private:
        QLabel *status = nullptr;
};

#endif
