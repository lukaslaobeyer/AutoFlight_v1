#include "settingsfileio.h"

#include <yaml-cpp/yaml.h>
#include <src/genericsettings.h>

#include <string>
#include <fstream>

using namespace std;

bool SettingsFileIO::saveSettings(const GenericSettings& settings, std::string filename)
{
    YAML::Emitter out;
    out.SetIndent(4);

    out << YAML::BeginMap;
    out << YAML::Key << "settings" << YAML::Value;

    out << YAML::BeginMap;
    for(GenericSettings::const_iterator entry_iter = settings.begin(); entry_iter != settings.end(); entry_iter++)
    {
        string entry_id = entry_iter->first;
        SettingsEntry entry = entry_iter->second;

        out << YAML::Key << entry_id;
        out << YAML::Value;
        boost::apply_visitor(SettingsFileIO::SettingsVisitor(out), entry.entry);

    }
    out << YAML::EndMap;

    out << YAML::EndMap;

    ofstream fout(filename + ".yaml");
    fout << out.c_str();

    return true;
}

GenericSettings SettingsFileIO::loadSettings(const GenericSettings &defaults, std::string filename)
{
    GenericSettings settings = defaults;

    try
    {
        YAML::Node ySettings = YAML::LoadFile(filename + ".yaml");

        if(ySettings["settings"] && ySettings["settings"].IsMap())
        {
            for(GenericSettings::iterator entry_iter = settings.begin(); entry_iter != settings.end(); entry_iter++)
            {
                string entry_id = entry_iter->first;
                SettingsEntry entry = entry_iter->second;

                if(ySettings["settings"][entry_id])
                {
                    if(!parseEntry(&settings, entry_id, ySettings["settings"][entry_id]))
                    {
                        cout << "Error parsing key " << entry_id << " in " << filename << endl;
                    }
                }
                else
                {
                    cout << "Settings key " << entry_id << " not found in " << filename << endl;
                }
            }
        }
        else
        {
            cout << filename << " is invalid" << endl;
        }
    }
    catch(YAML::BadFile &e)
    {
        cout << filename << " does not exist!" << endl;
        return defaults;
    }

    return settings;
}

bool SettingsFileIO::parseEntry(GenericSettings *settings, std::string key, YAML::Node value)
{
    if(value.IsScalar())
    {
        try
        {
            (*settings)[key].entry = boost::apply_visitor(SettingsLoadVisitor(value), (*settings)[key].entry);
        }
        catch(YAML::BadConversion &e)
        {
            return false;
        }

        return true;
    }
    else
    {
        return false;
    }
}

SettingsFileIO::SettingsVisitor::SettingsVisitor(YAML::Emitter &e) : out(e)
{ }

void SettingsFileIO::SettingsVisitor::operator()(bool s) const
{
    out << s;
}

void SettingsFileIO::SettingsVisitor::operator()(const NumberSetting &s) const
{
    out << s.value;
    string comment = "Value between " + to_string(s.min) + " and " + to_string(s.max);
    if(s.unit.length() > 0)
    {
        comment += "; Unit: " + s.unit;
    }
    out << YAML::Comment(comment);
}

void SettingsFileIO::SettingsVisitor::operator()(const ListSetting &s) const
{
    out << s.selectedValue;
}

SettingsFileIO::SettingsLoadVisitor::SettingsLoadVisitor(YAML::Node &value) : entry(value)
{ }

VariantSetting SettingsFileIO::SettingsLoadVisitor::operator()(bool s) const
{
    return entry.as<bool>();
}

VariantSetting SettingsFileIO::SettingsLoadVisitor::operator()(const NumberSetting& s) const
{
    double newValue = entry.as<double>();
    if(newValue < s.min || newValue > s.max)
    {
        return s;
    }

    NumberSetting setting = s;
    setting.value = newValue;
    return setting;
}

VariantSetting SettingsFileIO::SettingsLoadVisitor::operator()(const ListSetting& s) const
{
    ListSetting setting = s;

    string newEntry = entry.as<string>();

    if(setting.values.find(newEntry) == setting.values.end())
    {
        return setting;
    }

    setting.selectedValue = newEntry;
    return setting;
}