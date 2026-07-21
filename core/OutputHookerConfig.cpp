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
    outputSourcePriority = SourceNetwork;
    outputProcessingMethod = MethodPriority;
    startMinimized = false;

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
    settings.setValue("OutputSourcePriority", (outputSourcePriority == SourceWinMsg) ? PRIORITYWINMSGNAME : PRIORITYNETWORKNAME);

    QString methodName = METHODPRIORITYNAME;
    if (outputProcessingMethod == MethodExclusive)
        methodName = METHODEXCLUSIVENAME;
    else if (outputProcessingMethod == MethodConcurrent)
        methodName = METHODCONCURRENTNAME;

    settings.setValue("OutputProcessingMethod", methodName);
    settings.setValue("StartMinimized", startMinimized);
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

    // Only the Windows message system moves the preference away from the default (Network)
    outputSourcePriority = (settings.value("OutputSourcePriority", PRIORITYNETWORKNAME).toString().compare(PRIORITYWINMSGNAME, Qt::CaseInsensitive) == 0) ? SourceWinMsg : SourceNetwork;

    // Anything that is not Exclusive or Concurrent falls back to the default (Priority)
    QString methodName = settings.value("OutputProcessingMethod", METHODPRIORITYNAME).toString();

    if (methodName.compare(METHODEXCLUSIVENAME, Qt::CaseInsensitive) == 0)
        outputProcessingMethod = MethodExclusive;
    else if (methodName.compare(METHODCONCURRENTNAME, Qt::CaseInsensitive) == 0)
        outputProcessingMethod = MethodConcurrent;
    else
        outputProcessingMethod = MethodPriority;

    startMinimized = (settings.value("StartMinimized", startMinimized).toBool());
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

OutputSource OutputHookerConfig::getOutputSourcePriority()
{
    return outputSourcePriority;
}

void OutputHookerConfig::setOutputSourcePriority(OutputSource osPriority)
{
    outputSourcePriority = osPriority;
}

OutputProcessingMethod OutputHookerConfig::getOutputProcessingMethod()
{
    return outputProcessingMethod;
}

void OutputHookerConfig::setOutputProcessingMethod(OutputProcessingMethod opMethod)
{
    outputProcessingMethod = opMethod;
}

bool OutputHookerConfig::getStartMinimized()
{
    return startMinimized;
}

void OutputHookerConfig::setStartMinimized(bool sMinimized)
{
    startMinimized = sMinimized;
}

bool OutputHookerConfig::getAutostartWithSystem()
{
    QSettings runKey(AUTOSTARTREGKEY, QSettings::NativeFormat);

    // Only report autostart as on if the entry points at this executable. A stale entry
    // from a copy that has been moved or deleted would never start OutputHooker
    QString registered = QDir::toNativeSeparators(runKey.value(AUTOSTARTVALUENAME).toString()).remove('"');
    QString thisExe = QDir::toNativeSeparators(QApplication::applicationFilePath());

    return (registered.compare(thisExe, Qt::CaseInsensitive) == 0);
}

void OutputHookerConfig::setAutostartWithSystem(bool awSystem)
{
    QSettings runKey(AUTOSTARTREGKEY, QSettings::NativeFormat);

    if (awSystem)
    {
        // Quoted, as the path to the executable can contain spaces
        QString thisExe = QDir::toNativeSeparators(QApplication::applicationFilePath());
        runKey.setValue(AUTOSTARTVALUENAME, "\"" + thisExe + "\"");
    }
    else
    {
        runKey.remove(AUTOSTARTVALUENAME);
    }
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
