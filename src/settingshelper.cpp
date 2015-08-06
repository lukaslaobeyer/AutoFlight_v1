#include "settingshelper.h"

using namespace std;

void SettingsHelper::applyBebopVideoSettings(std::shared_ptr<Bebop> b, GenericSettings &s)
{
    try
    {
        float exp = (float) (boost::get<NumberSetting>(s["exposure"].entry).value);
        float sat = (float) (boost::get<NumberSetting>(s["saturation"].entry).value);

        bebop::pictureformat pic_fmt = PIC_FMTS.at(boost::get<ListSetting>(s["pictureformat"].entry).selectedValue);
        bebop::whitebalancemode wb_mode = WB_MODES.at(boost::get<ListSetting>(s["whitebalance"].entry).selectedValue);
        bebop::antiflickermode af_mode = AF_MODES.at(boost::get<ListSetting>(s["antiflicker"].entry).selectedValue);

        b->setVideoSettings(pic_fmt, wb_mode, af_mode, exp, sat);
    }
    catch(boost::bad_get &e)
    {
        cout << "error applying new bebop video settings: what(): " << e.what() << endl;
    }
}

void SettingsHelper::applyFlightSettings(std::shared_ptr<Drone> d, GenericSettings &s)
{
    try
    {
        drone::config config;

        config.limits.altitude = (float) (boost::get<NumberSetting>(s["altitude"].entry).value);
        config.limits.angle = (float) ((boost::get<NumberSetting>(s["angle"].entry).value) * (M_PI/180.0));
        config.limits.yawspeed = (float) ((boost::get<NumberSetting>(s["rotation_speed"].entry).value) * (M_PI/180.0));
        config.limits.vspeed = (float) (boost::get<NumberSetting>(s["vertical_speed"].entry).value);
        config.outdoor = FLIGHT_MODES.at(boost::get<ListSetting>(s["flight_mode"].entry).selectedValue);

        config.valid = true;

        d->setConfig(config);
    }
    catch(boost::bad_get &e)
    {
        cout << "error applying new bebop video settings: what(): " << e.what() << endl;
    }
}