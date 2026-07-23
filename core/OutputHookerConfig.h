/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#ifndef OUTPUTHOOKERCONFIG_H
#define OUTPUTHOOKERCONFIG_H

#include <QObject>
#include <QDir>
#include <QApplication>
#include <QScreen>
#include <QThread>

#include "../Global.h"

class OutputHookerConfig : public QObject
{
    Q_OBJECT

public:
    explicit OutputHookerConfig(QObject *parent = nullptr);
    ~OutputHookerConfig();

    // Save and load settings
    void saveSettings();
    void loadSettings();

    // Get & Set settings functions
    // Get a notification if a new game is found
    bool getUseNewOutputsNotification();
    void setUseNewOutputsNotification(bool unoNotification);

    // Save new outputs to default.ini
    bool getAddNewOutputsToDefaultINI();
    void setAddNewOutputsToDefaultINI(bool anotDefaultINI);

    // Use Multi-Threading
    bool getUseMultiThreading();
    void setUseMultiThreading(bool umThreading);

    // Output source that is preferred, when both output streams report a game
    OutputSource getOutputSourcePriority();
    void setOutputSourcePriority(OutputSource osPriority);

    // How the two output streams are handled
    OutputProcessingMethod getOutputProcessingMethod();
    void setOutputProcessingMethod(OutputProcessingMethod opMethod);

    // Start OutputHooker in the system tray
    bool getStartMinimized();
    void setStartMinimized(bool sMinimized);

    // Start OutputHooker with Windows. Read from and written to the registry Run key,
    // not the INI file, so the menu always shows what Windows will actually do
    bool getAutostartWithSystem();
    void setAutostartWithSystem(bool awSystem);

    // Get COM Port placeholders
    QMap<QString, QString> getComPortPlaceholders();

    // Ignore Serial COM Port checks for debug
    bool getSerialPortWriteCheckBypass();
    void setSerialPortWriteCheckBypass(bool spwCB);

private:
    // Settings file
    QString settingsFile;

    // Settings
    bool useNewOutputsNotification;
    bool addNewOutputsToDefaultINI;
    bool useMultiThreading;
    bool bypassSerialWriteChecks;
    OutputSource outputSourcePriority;
    OutputProcessingMethod outputProcessingMethod;
    bool startMinimized;

    // QMap - COM Port placeholders
    QMap<QString, QString> comPortPlaceholders;
};

#endif // OUTPUTHOOKERCONFIG_H
