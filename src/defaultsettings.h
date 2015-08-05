#ifndef AUTOFLIGHT_DEFAULTSETTINGS_H
#define AUTOFLIGHT_DEFAULTSETTINGS_H

#include "genericsettings.h"

const GenericSettings defaultBebopVideoSettings
        {
                {"exposure", SettingsEntry{"Exposure", NumberSetting{0, -3, 3, 10, ""}}},
                {"saturation", SettingsEntry{"Saturation", NumberSetting{0, -100, 100, 1, ""}}},
                {"whitebalance", SettingsEntry{"Whitebalance mode", ListSetting{"auto", {
                        {"auto", "Auto"},
                        {"tungsten", "Tungsten"},
                        {"daylight", "Daylight"},
                        {"cloudy", "Cloudy"},
                        {"cool_white", "Cool white"}
                }}}},
                {"antiflicker", SettingsEntry{"Antiflicker mode", ListSetting{"50", {
                        {"50", "50 Hz"},
                        {"60", "60 Hz"}
                }}}},
                {"pictureformat", SettingsEntry{"Picture format", ListSetting{"jpeg", {
                        {"raw", "Raw image"},
                        {"jpeg", "4:3 JPEG picture"},
                        {"snapshot", "16:9 snapshot"},
                        {"jpeg_fisheye", "Fisheye JPEG"}
                }}}}
        };

#endif
