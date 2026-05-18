/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#include "HidapiModule.h"

HidapiModule::HidapiModule(QObject *parent)
    : QObject{parent}
{
    // Initialize the USB HID
    if (hid_init())
    {
        QString errorMsg = "Failed to initialize USB HID!";
        emit showErrorMessage("USB HID Initialisation - Error!", errorMsg);
    }

    for (quint8 i = 0; i < MAXPLAYERS; i++)
    {
        hidOpen[i] = false;
    }
}

HidapiModule::~HidapiModule()
{
    // Disconnect all devices
    disconnectAll();

    // Exit the USB HID
    hid_exit ();
}

// Disconnect all USB HIDs
void HidapiModule::disconnectAll()
{
    for (quint8 i = 0; i < MAXPLAYERS; i++)
    {
        if (hidOpen[i])
        {
            hid_close(p_hidConnection[i]);
            hidOpen[i] = false;
        }
    }
}

// Connect to an USB HID
void HidapiModule::connectHID(const quint8 &playerNum, const HIDInfo &deviceHIDInfo)
{
    // Check if connection is already made for USB HID
    if (!hidOpen[playerNum])
    {
        hidInfoArray[playerNum] = deviceHIDInfo;

        QByteArray pathBA = deviceHIDInfo.path.toUtf8();

        char* pathPtr = pathBA.data();

        p_hidConnection[playerNum] = hid_open_path(pathPtr);

        if (!p_hidConnection[playerNum])
        {
            if (!bypassCOMPortConnectFailWarning)
            {
                // Failed to open USB HID
                const wchar_t* errorWChar = hid_error(p_hidConnection[playerNum]);
                QString errorMsg = "Failed to connect to USB HID for player: " + QString::number(playerNum+1) + "\nError Message: " + QString::fromWCharArray(errorWChar);
                emit showErrorMessage("USB HID Connect - Error!", errorMsg);
            }
        }
        else
        {
            // Connection made, ready to go
            hidOpen[playerNum] = true;
        }
    }
}

// Disconnect the USB HID connection
void HidapiModule::disconnectHID(const quint8 &playerNum)
{
    // Check if connection is open
    if (hidOpen[playerNum])
    {
        hid_close(p_hidConnection[playerNum]);
        hidOpen[playerNum] = false;
    }
}

// Write data to USB HID
void HidapiModule::writeDataHID(const quint8 &playerNum, const QByteArray &writeData)
{
    // Check if connection is open
    if (hidOpen[playerNum])
    {
        const unsigned char *constDataPtr = reinterpret_cast<const unsigned char*>(writeData.constData());
        std::size_t size = writeData.size();
        quint8 retry = 0;
        qint16 bytesWritten = hid_write(p_hidConnection[playerNum], constDataPtr, size);

        while (bytesWritten == -1 && retry != WRITERETRYATTEMPTS)
        {
            bytesWritten = hid_write(p_hidConnection[playerNum], constDataPtr, size);
            retry++;
        }

        if (bytesWritten == -1)
        {
            // Write failed
            const wchar_t* errorWChar = hid_error(p_hidConnection[playerNum]);
            QString errorMsg = "Failed to write USB HID data!\nError Message: " + QString::fromWCharArray(errorWChar);
            emit showErrorMessage("USB HID Write Data - Error!", errorMsg);
        }
    }
}