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