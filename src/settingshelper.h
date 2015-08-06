#ifndef AUTOFLIGHT_SETTINGSHELPER_H
#define AUTOFLIGHT_SETTINGSHELPER_H

#include <drones/bebop/bebop.h>
#include <memory>
#include <map>
#include <string>
#include <drone.h>
#include <drones/bebop/types.h>
#include "genericsettings.h"

namespace SettingsHelper
{
    void applyBebopVideoSettings(std::shared_ptr<Bebop> b, GenericSettings &s);
    void applyFlightSettings(std::shared_ptr<Drone> d, GenericSettings &s);

    const std::map<std::string, bebop::pictureformat> PIC_FMTS {
            {"raw", bebop::PICFMT_RAW},
            {"jpeg", bebop::PICFMT_JPEG},
            {"snapshot", bebop::PICFMT_SNAPSHOT},
            {"jpeg_fisheye", bebop::PICFMT_JPEG_FISHEYE}
    };

    const std::map<std::string, bebop::whitebalancemode> WB_MODES {
            {"auto", bebop::WB_AUTO},
            {"tungsten", bebop::WB_TUNGSTEN},
            {"daylight", bebop::WB_DAYLIGHT},
            {"cloudy", bebop::WB_CLOUDY},
            {"cool_white", bebop::WB_COOL_WHITE}
    };

    const std::map<std::string, bebop::antiflickermode> AF_MODES {
            {"auto", bebop::AF_AUTO},
            {"50", bebop::AF_50HZ},
            {"60", bebop::AF_60HZ}
    };

    const std::map<std::string, bool> FLIGHT_MODES {
            {"indoor", false},
            {"outdoor", true}
    };
}

#endif
