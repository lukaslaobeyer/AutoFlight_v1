#ifndef AUTOFLIGHT_GENERICSETTINGS_H
#define AUTOFLIGHT_GENERICSETTINGS_H

#include <string>
#include <iostream>
#include <boost/variant.hpp>
#include <map>
#include <yaml-cpp/emitter.h>

struct NumberSetting
{
    double value;
    double min;
    double max;
    int resolution;
    std::string unit;

    friend std::ostream &operator<<(std::ostream &out, const NumberSetting &setting)
    {
        out << setting.value;
        return out;
    }
};

struct ListSetting
{
    std::string selectedValue;
    std::map<std::string, std::string> values;
};

typedef boost::variant<NumberSetting, bool, ListSetting> VariantSetting;

struct SettingsEntry
{
    std::string description;
    VariantSetting entry;
};

typedef std::map<std::string, SettingsEntry> GenericSettings;

#endif
