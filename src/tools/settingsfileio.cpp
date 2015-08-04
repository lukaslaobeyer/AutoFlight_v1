#include "settingsfileio.h"

#include <yaml-cpp/yaml.h>

bool SettingsFileIO::saveSettings(GenericSettings& settings, std::string filename)
{
    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap;

    for(GenericSettings::iterator entry_iter = settings.begin(); entry_iter != settings.end(); entry_iter++)
    {

    }

    out << YAML::EndMap;
    out << YAML::EndDoc;
}