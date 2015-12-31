#ifndef AUTOFLIGHT_SOUNDALERTS_H
#define AUTOFLIGHT_SOUNDALERTS_H

#include <QtWidgets>
#include <QtMultimedia/QSoundEffect>

#include <memory>

#include "qinterface/qnavdatalistener.h"

#include <drone.h>

class SoundAlerts : public QObject, QNavdataListener
{
    Q_OBJECT

    public:
        SoundAlerts();

    public Q_SLOTS:
        void navdataAvailable(std::shared_ptr<const drone::navdata> navdata);

    private:
        std::unique_ptr<QSoundEffect> battery_low = nullptr;
        std::unique_ptr<QSoundEffect> link_weak = nullptr;
};

#endif