#ifndef AUTOFLIGHT_SETTINGSFILEIO_H
#define AUTOFLIGHT_SETTINGSFILEIO_H

#include "../genericsettings.h"
#include <string>
#include <boost/variant.hpp>

namespace SettingsFileIO
{
    bool saveSettings(const GenericSettings &settings, std::string filename);
    GenericSettings loadSettings(const GenericSettings &defaults, std::string filename);
    bool parseEntry(GenericSettings *settings, std::string key, YAML::Node value);

    class SettingsVisitor : public boost::static_visitor<void>
    {
        public:
            SettingsVisitor(YAML::Emitter &e);

            void operator()(const NumberSetting &s) const;
            void operator()(bool s) const;
            void operator()(const ListSetting &s) const;
        private:
            YAML::Emitter &out;
    };

    class SettingsLoadVisitor : public boost::static_visitor<VariantSetting>
    {
        public:
            SettingsLoadVisitor(YAML::Node &value);

            VariantSetting operator()(const NumberSetting &s) const;
            VariantSetting operator()(bool s) const;
            VariantSetting operator()(const ListSetting &s) const;
        private:
            YAML::Node &entry;
    };
};

#endif
