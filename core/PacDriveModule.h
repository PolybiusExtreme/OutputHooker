#ifndef PACDRIVEMODULE_H
#define PACDRIVEMODULE_H

#include <QObject>
#include <QList>
#include <QMap>

//Ultimarc PacDrive SDK
#include "StdAfx.h"
#include "Windows.h"
#include "PacDrive.h"

#include "../Global.h"

class PacDriveModule : public QObject
{
    Q_OBJECT

public:
    explicit PacDriveModule(QObject *parent = nullptr);
    ~PacDriveModule();

    // Collect Ultimarc data
    void collectUltimarcData();

public slots:
    // Set pin state
    void setPinState(quint8 id, quint8 pin, bool state);

    // Set light intensity
    void setLightIntensity(quint8 id, quint8 pin, quint8 intensity);

    // Turn all lights off
    void turnAllLightsOff(quint8 id);

signals:
    // Show error message in main thread
    void showErrorMessage(const QString &title, const QString &message);

public:
    // Number of Ultimarc devices filtered
    qint8 numberUltimarcDevices;

    // Number of Ultimarc devices that are valid
    qint8 numberUltimarcDevicesValid;

    // Ultimarc device data
    UltimarcData dataUltimarc[ULTIMARCMAXDEVICES];

    // Number of pins
    QList<quint8> numberPins;

    // Number of groups
    QList<quint8> numberGroups;

    // Light state map
    QMap<quint8,QList<bool>> lightStateMap;

    // Light intensity map
    QMap<quint8,QList<quint8>> lightIntensityMap;

    // Group state data
    QMap<quint8,QList<quint8>> groupStateData;
};

#endif // PACDRIVEMODULE_H
