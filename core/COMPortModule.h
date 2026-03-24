#ifndef COMPORTMODULE_H
#define COMPORTMODULE_H

#include <QObject>
#include <QByteArray>

#include <QDebug>

#include "../Global.h"

#include <Windows.h>

class COMPortModule : public QObject
{
    Q_OBJECT

public:
    explicit COMPortModule(QObject *parent = nullptr);
    ~COMPortModule();

    // Reconnect to existing COM Port
    bool reconnectComPort(quint8 comPortNum);

public slots:
    // Connect to COM Port
    void connectComPort(const quint8 &comPortNum, const QString &comPortName, const qint32 &comPortBaud, const quint8 &comPortData, const quint8 &comPortParity, const quint8 &comPortStop, const quint8 &comPortFlow, const QString &comPortPath, const bool &isWriteOnly);

    // Disconnect to COM Port
    void disconnectComPort(const quint8 &comPortNum);

    // Write data to COM Port
    void writeData(const quint8 &comPortNum, const QByteArray &writeData);

    // Disconnect all open COM Ports
    void disconnectAll();

    // Slot to get setting for to Bypass Serial Port Write Checks or Not
    void setBypassSerialWriteChecks(const bool &bypassSPWC);

    // Slot for setting the bypass COM Port connection fail warning pop-up
    void setBypassCOMPortConnectFailWarning(const bool &bypassCPCFW);

signals:
    // Signal used to display error message from COM Port
    void showErrorMessage(const QString &title, const QString &message);

private:
    // How many COM Ports are open
    quint8 numPortOpen;

    // If one or more COM Ports are open
    bool isPortOpen;

    // Bool List to Keep Track on What COM Ports that are Open
    bool comPortOpen[MAXCOMPORTS];

    // Pointer Array of Serial COM Ports
    HANDLE comPortArray[MAXCOMPORTS];

    // COM Port Data
    QList<DCB> comPortDCBList;
    QList<COMMTIMEOUTS> comPortTOList;
    QList<LPCWSTR> comPortLPCList;

    // Debug Setting to Bypass Serial Port Write Checks
    bool bypassSerialWriteChecks;

    // Bypass pop-up error, when Serial COM Port could not connect on INI side
    bool bypassCOMPortConnectFailWarning;

    bool noLightGunWarning[MAXCOMPORTS];

};

#endif // COMPORTMODULE_H
