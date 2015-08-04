#ifndef AUTOFLIGHT_SETTINGSFILEIO_H
#define AUTOFLIGHT_SETTINGSFILEIO_H

#include "../src/genericsettings.h"
#include <string>

namespace SettingsFileIO
{
    bool saveSettings(GenericSettings &settings, std::string filename);
};

#endif
