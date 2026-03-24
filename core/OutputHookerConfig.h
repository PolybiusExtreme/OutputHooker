#ifndef OUTPUTHOOKERCONFIG_H
#define OUTPUTHOOKERCONFIG_H

#include <QObject>
#include <QDir>
#include <QApplication>
#include <QScreen>
#include <QThread>

class OutputHookerConfig : public QObject
{
    Q_OBJECT

public:
    explicit OutputHookerConfig(QObject *parent = nullptr);
    ~OutputHookerConfig();

    //Save and load settings
    void saveSettings();
    void loadSettings();

    //Get & Set settings functions
    //Get a notification if a new game is found
    bool getUseNewOutputsNotification();
    void setUseNewOutputsNotification(bool unoNotification);

    //Save new outputs to default.ini
    bool getSaveNewOutputsToDefaultINI();
    void setSaveNewOutputsToDefaultINI(bool snotDefaultINI);

    //Use Multi-Threading
    bool getUseMultiThreading();
    void setUseMultiThreading(bool umThreading);

    //Ignore Serial COM Port checks for debug
    bool getSerialPortWriteCheckBypass();
    void setSerialPortWriteCheckBypass(bool spwCB);

private:
    //Settings file
    QString settingsFile;

    //Settings
    bool useNewOutputsNotification;
    bool saveNewOutputsToDefaultINI;
    bool useMultiThreading;
    bool bypassSerialWriteChecks;
};

#endif // OUTPUTHOOKERCONFIG_H
