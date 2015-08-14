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
                {"antiflicker", SettingsEntry{"Antiflicker mode", ListSetting{"auto", {
                        {"auto", "Auto (recommended)"},
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

const GenericSettings defaultFlightConfig
        {
                {"altitude", SettingsEntry{"Maximum altitude", NumberSetting{5, 1, 200, 10, "m"}}},
                {"angle", SettingsEntry{"Maximum pitch/roll angle", NumberSetting{12, 5, 30, 1, "deg"}}},
                {"vertical_speed", SettingsEntry{"Maximum vertical speed", NumberSetting{1, 0.2, 2, 100, "m/s"}}},
                {"rotation_speed", SettingsEntry{"Maximum rotation speed", NumberSetting{100, 10, 360, 1, "deg/s"}}},
                {"flight_mode", SettingsEntry{"Flight mode", ListSetting{"indoor", {
                        {"indoor", "Indoor with hull"},
                        {"outdoor", "Outdoor without hull"}
                }}}}
        };

/*const GenericSettings defaultControllerConfig
        {
                {"controller", SettingsEntry{"Controller ID", ControllerIDSetting{-1, -1, -1}}},

                {"height", SettingsEntry{"Height control", AnalogControlSetting{-1, -1, -1}}},
                {"yaw", SettingsEntry{"Yaw control", AnalogControlSetting{-1, -1, -1}}},
                {"pitch", SettingsEntry{"Pitch control", AnalogControlSetting{-1, -1, -1}}},
                {"roll", SettingsEntry{"Roll control", AnalogControlSetting{-1, -1, -1}}},

                {"takeoff_land", SettingsEntry{"Take off/land", -1}},
                {"emergency", SettingsEntry{"Emergency motor cutoff", -1}},
                {"recording", SettingsEntry{"Start/stop recording", -1}},
                {"picture", SettingsEntry{"Take a picture", -1}},
                {"switchview", SettingsEntry{"Switch view", -1}},
                {"flip", SettingsEntry{"Perform flip", -1}},
                {"slow", SettingsEntry{"Slow mode", -1}},
                {"bebop_camera", SettingsEntry{"Bebop drone camera angle", -1}}
        };*/

#endif
