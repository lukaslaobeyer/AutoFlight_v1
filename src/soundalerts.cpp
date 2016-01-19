#include "soundalerts.h"

#define BATTERY_ALERT_THRESHOLD 13
#define LINK_ALERT_THRESHOLD 25

SoundAlerts::SoundAlerts()
{
    battery_low.reset(new QSoundEffect());
    battery_low->setSource(QUrl("qrc:/resources/batt_low.wav"));

    link_weak.reset(new QSoundEffect());
    link_weak->setSource(QUrl("qrc:/resources/link_weak.wav"));
}

void SoundAlerts::navdataAvailable(std::shared_ptr<const drone::navdata> nd)
{
    int battery_percent = (int) (nd->batterystatus * 100.0f);
    int link_percent = (int) (nd->linkquality * 100.0f);

    if((battery_percent <= BATTERY_ALERT_THRESHOLD) && (battery_percent > 0))
    {
        if(!battery_low->isPlaying()) battery_low->play();
    }
    else
    {
        if(battery_low->isPlaying()) battery_low->stop();
    }

    if((link_percent <= LINK_ALERT_THRESHOLD) && (link_percent > 0))
    {
        if(!link_weak->isPlaying()) link_weak->play();
    }
    else
    {
        if(link_weak->isPlaying()) link_weak->stop();
    }
}