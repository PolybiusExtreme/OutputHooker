#include "OutputHookerConfig.h"

#include <QSettings>

#include "../Global.h"

OutputHookerConfig::OutputHookerConfig(QObject *parent)
    : QObject{parent}
{
    //Set settings to default
    useNewOutputsNotification = true;
    saveNewOutputsToDefaultINI = false;
    useMultiThreading = true;
    bypassSerialWriteChecks = false;

    //Define save file and the path
    settingsFile = QApplication::applicationDirPath() + "/" SETTINGSFILE;

    //Load settings
    loadSettings();
}

OutputHookerConfig::~OutputHookerConfig()
{
    //Not used!
}

//Save settings
void OutputHookerConfig::saveSettings()
{
    QSettings settings(settingsFile, QSettings::Format::IniFormat);
    settings.beginGroup("OutputHooker");
    settings.setValue("Notification", useNewOutputsNotification);
    settings.setValue("UseDefaultINI", saveNewOutputsToDefaultINI);
    settings.setValue("MultiThreading", useMultiThreading);
    settings.endGroup();
    settings.beginGroup("Debug");
    settings.setValue("BypassSerialWriteChecks", bypassSerialWriteChecks);
    settings.endGroup();
}

//Load settings
void OutputHookerConfig::loadSettings()
{
    QSettings settings(settingsFile, QSettings::Format::IniFormat);
    settings.beginGroup("OutputHooker");
    useNewOutputsNotification = (settings.value("Notification").toBool());
    saveNewOutputsToDefaultINI = (settings.value("UseDefaultINI").toBool());
    useMultiThreading = (settings.value("MultiThreading").toBool());
    settings.endGroup();
    settings.beginGroup("Debug");
    bypassSerialWriteChecks = (settings.value("BypassSerialWriteChecks").toBool());
    settings.endGroup();
}

//Get & Set settings
bool OutputHookerConfig::getUseNewOutputsNotification()
{
    return useNewOutputsNotification;
}

void OutputHookerConfig::setUseNewOutputsNotification(bool unoNotification)
{
    useNewOutputsNotification = unoNotification;
}

bool OutputHookerConfig::getSaveNewOutputsToDefaultINI()
{
    return saveNewOutputsToDefaultINI;
}

void OutputHookerConfig::setSaveNewOutputsToDefaultINI(bool snotDefaultINI)
{
    saveNewOutputsToDefaultINI = snotDefaultINI;
}

bool OutputHookerConfig::getUseMultiThreading()
{
    return useMultiThreading;
}

void OutputHookerConfig::setUseMultiThreading(bool umThreading)
{
    useMultiThreading = umThreading;
}

bool OutputHookerConfig::getSerialPortWriteCheckBypass()
{
    return bypassSerialWriteChecks;
}

void OutputHookerConfig::setSerialPortWriteCheckBypass(bool spwCB)
{
    bypassSerialWriteChecks = spwCB;
}
