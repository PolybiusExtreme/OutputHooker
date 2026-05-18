/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#ifndef HIDAPIMODULE_H
#define HIDAPIMODULE_H

#include <QObject>

#include "../Global.h"

#include <Windows.h>
#include "hidapi_winapi.h"

class HidapiModule : public QObject
{
    Q_OBJECT

public:
    explicit HidapiModule(QObject *parent = nullptr);
    ~HidapiModule();

public slots:
    // Disconnect all USB HIDs
    void disconnectAll();

    // Connect to an USB HID
    void connectHID(const quint8 &playerNum, const HIDInfo &deviceHIDInfo);

    // Disconnect the USB HID connection
    void disconnectHID(const quint8 &playerNum);

    // Write data to USB HID
    void writeDataHID(const quint8 &playerNum, const QByteArray &writeData);

signals:
    // Show error message in main thread
    void showErrorMessage(const QString &title, const QString &message);

private:
    // Pointer array of USB HIDs
    hid_device *p_hidConnection[MAXPLAYERS];

    // Bool list to keep track on what USB HID are open
    bool hidOpen[MAXPLAYERS];

    // HID info array of the HIDs being used
    HIDInfo hidInfoArray[MAXPLAYERS];

    //Bypass pop-up error, if Serial COM Port could not connect on INI side
    bool bypassCOMPortConnectFailWarning;
};

#endif // HIDAPIMODULE_H
