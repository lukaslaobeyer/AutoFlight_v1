#ifndef AUTOFLIGHT_GENERICSETTINGS_H
#define AUTOFLIGHT_GENERICSETTINGS_H

struct NumberSetting
{
    double value;
    double min;
    double max;
    int resolution;
    std::string unit;

    friend std::ostream& operator<<(std::ostream &out, const NumberSetting &setting)
    {
        out << setting.value;
        return out;
    }
};

typedef boost::variant<NumberSetting, bool> VariantSetting;

struct SettingsEntry
{
    std::string description;
    VariantSetting entry;
};

typedef std::map<std::string, SettingsEntry> GenericSettings;

#endif
