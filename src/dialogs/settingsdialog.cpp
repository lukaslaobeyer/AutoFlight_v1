#include "settingsdialog.h"

#include <QtWidgets>

using namespace std;

SettingsDialog::SettingsDialog(GenericSettings &settings, QWidget *parent) : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    // Generate UI from GenericSettings
    for(GenericSettings::iterator entry_iter = settings.begin(); entry_iter != settings.end(); entry_iter++)
    {
        string entry_id = entry_iter->first;
        SettingsEntry entry = entry_iter->second;
        newSettings.insert({entry_id, entry});

        QHBoxLayout *sublayout = new QHBoxLayout;
        QLabel *label = new QLabel(QString::fromStdString(entry.description));
        label->setFixedWidth(150);

        sublayout->addWidget(label);

        QLayout *specificlayout = boost::apply_visitor(SettingsEntryCreator(this, entry_id), entry.entry);
        sublayout->addLayout(specificlayout);

        layout->addLayout(sublayout);
    }
}

void SettingsDialog::updateEntry(string entry_id, bool newValue)
{
    VariantSetting newSetting = newValue;
    newSettings[entry_id].entry = newSetting;
}

void SettingsDialog::updateEntry(string entry_id, double newValue)
{
    NumberSetting newSetting = boost::get<NumberSetting>(newSettings[entry_id].entry);
    newSetting.value = newValue;
    newSettings[entry_id].entry = newSetting;
}

GenericSettings SettingsDialog::getSettings()
{
    return newSettings;
}

SettingsEntryCreator::SettingsEntryCreator(SettingsDialog *dialog, string entry_id) : _dialog(dialog), _entry_id(entry_id)
{ }

QLayout *SettingsEntryCreator::operator()(bool setting) const
{
    QHBoxLayout *layout = new QHBoxLayout();

    // Checkbox
    QCheckBox *checkbox = new QCheckBox();
    checkbox->setChecked(setting);

    QObject::connect(checkbox, &QCheckBox::stateChanged, std::bind(static_cast<void (SettingsDialog::*)(string, bool)>(&SettingsDialog::updateEntry), _dialog, _entry_id, std::placeholders::_1));

    layout->addWidget(checkbox);
    return layout;
}

QLayout *SettingsEntryCreator::operator()(const NumberSetting &setting) const
{
    QHBoxLayout *layout = new QHBoxLayout();

    // Slider
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setTracking(true);
    slider->setRange((int) (setting.min * setting.resolution), (int) (setting.max * setting.resolution));
    slider->setValue((int) (setting.value * setting.resolution));
    slider->setFixedWidth(300);

    // Spinner
    QDoubleSpinBox *spinner = new QDoubleSpinBox();
    spinner->setDecimals((int) log10(setting.resolution));
    spinner->setRange(setting.min, setting.max);
    spinner->setValue(setting.value);
    spinner->setFixedWidth(80);

    // Unit label
    QLabel *unit = new QLabel(QString::fromStdString(setting.unit));

    layout->addWidget(slider);
    layout->addWidget(spinner);
    layout->addWidget(unit);

    QObject::connect(slider, &QSlider::valueChanged, [=](int value) {
        spinner->setValue((double) value / (double) setting.resolution);
    });

    QObject::connect(spinner, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
        slider->setValue((int) (value * setting.resolution));
    });

    QObject::connect(spinner, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), std::bind(static_cast<void (SettingsDialog::*)(string, double)>(&SettingsDialog::updateEntry), _dialog, _entry_id, std::placeholders::_1));

    return layout;
}