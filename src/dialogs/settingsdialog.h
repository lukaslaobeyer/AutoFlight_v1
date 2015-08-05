#ifndef AUTOFLIGHT_SETTINGSDIALOG_H
#define AUTOFLIGHT_SETTINGSDIALOG_H

#include <QDialog>
#include <QLayout>

#include <string>
#include <map>
#include <boost/variant.hpp>

#include "../genericsettings.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit SettingsDialog(GenericSettings &s, QWidget *parent = 0);
        GenericSettings getSettings();
        void updateEntry(std::string entry_id, bool newValue);
        void updateEntry(std::string entry_id, double newValue);
        void updateEntry(std::string entry_id, const QString &newValue);
    Q_SIGNALS:
        void applied();
    private:
        GenericSettings newSettings;
};

class SettingsEntryCreator : public boost::static_visitor<QLayout *>
{
    public:
        SettingsEntryCreator(SettingsDialog *dialog, std::string entry_id);

        QLayout *operator()(bool setting) const;
        QLayout *operator()(const NumberSetting &setting) const;
        QLayout *operator()(ListSetting &setting) const;
    private:
        SettingsDialog *_dialog;
        std::string _entry_id;
};

#endif
