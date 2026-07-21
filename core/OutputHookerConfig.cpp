/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#include "OutputHookerConfig.h"

#include <QSettings>

#include "../Global.h"

OutputHookerConfig::OutputHookerConfig(QObject *parent)
    : QObject{parent}
{
    // Set settings to default
    useNewOutputsNotification = true;
    addNewOutputsToDefaultINI = false;
    useMultiThreading = true;
    bypassSerialWriteChecks = false;

    // Define save file and the path
    settingsFile = QApplication::applicationDirPath() + "/" SETTINGSFILE;

    // Load settings
    loadSettings();
}

OutputHookerConfig::~OutputHookerConfig()
{
    // Not used!
}

// Save settings
void OutputHookerConfig::saveSettings()
{
    QSettings settings(settingsFile, QSettings::Format::IniFormat);
    settings.beginGroup("OutputHooker");
    settings.setValue("NotificationOnNewOutputs", useNewOutputsNotification);
    settings.setValue("AddNewOutputsToDefaultINI", addNewOutputsToDefaultINI);
    settings.setValue("MultiThreading", useMultiThreading);
    settings.endGroup();

    settings.beginGroup("COM_Ports");
    QMapIterator<QString, QString> i(comPortPlaceholders);
    while (i.hasNext())
    {
        i.next();
        QString cleanKey = i.key();
        cleanKey.remove("%");
        settings.setValue(cleanKey, i.value());
    }
    settings.endGroup();

    settings.beginGroup("Debug");
    settings.setValue("BypassSerialWriteChecks", bypassSerialWriteChecks);
    settings.endGroup();
}

// Load settings
void OutputHookerConfig::loadSettings()
{
    QSettings settings(settingsFile, QSettings::Format::IniFormat);

    settings.beginGroup("OutputHooker");
    useNewOutputsNotification = (settings.value("NotificationOnNewOutputs", useNewOutputsNotification).toBool());
    addNewOutputsToDefaultINI = (settings.value("AddNewOutputsToDefaultINI", addNewOutputsToDefaultINI).toBool());
    useMultiThreading = (settings.value("MultiThreading", useMultiThreading).toBool());
    settings.endGroup();

    comPortPlaceholders.clear();
    settings.beginGroup("COM_Ports");
    QStringList keys = settings.allKeys();
    foreach (const QString &key, keys)
    {
        QString placeholder = QString("%%1%").arg(key);
        comPortPlaceholders.insert(placeholder, settings.value(key).toString());
    }
    settings.endGroup();

    settings.beginGroup("Debug");
    bypassSerialWriteChecks = (settings.value("BypassSerialWriteChecks", bypassSerialWriteChecks).toBool());
    settings.endGroup();
}

// Get & Set settings
bool OutputHookerConfig::getUseNewOutputsNotification()
{
    return useNewOutputsNotification;
}

void OutputHookerConfig::setUseNewOutputsNotification(bool unoNotification)
{
    useNewOutputsNotification = unoNotification;
}

bool OutputHookerConfig::getAddNewOutputsToDefaultINI()
{
    return addNewOutputsToDefaultINI;
}

void OutputHookerConfig::setAddNewOutputsToDefaultINI(bool anotDefaultINI)
{
    addNewOutputsToDefaultINI = anotDefaultINI;
}

bool OutputHookerConfig::getUseMultiThreading()
{
    return useMultiThreading;
}

void OutputHookerConfig::setUseMultiThreading(bool umThreading)
{
    useMultiThreading = umThreading;
}

QMap<QString, QString> OutputHookerConfig::getComPortPlaceholders()
{
    return comPortPlaceholders;
}

bool OutputHookerConfig::getSerialPortWriteCheckBypass()
{
    return bypassSerialWriteChecks;
}

void OutputHookerConfig::setSerialPortWriteCheckBypass(bool spwCB)
{
    bypassSerialWriteChecks = spwCB;
}
