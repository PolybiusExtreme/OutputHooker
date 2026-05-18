/*
 * Original Copyright (c) 2026 PolybiusExtreme
 *
 * Licensed under the GNU GPLv3.
 */

#ifndef LEDWIZMODULE_H
#define LEDWIZMODULE_H

#include <QObject>
#include <QLibrary>
#include <QList>
#include <QTimer>

//LedWiz SDK
#include "LEDWiz.h"

#include "../Global.h"

// Definition of function pointers for the DLL functions
typedef void (LWZCALL *Ptr_LWZ_SBA)(LWZHANDLE, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
typedef void (LWZCALL *Ptr_LWZ_PBA)(LWZHANDLE, uint8_t const *);
typedef void (LWZCALL *Ptr_LWZ_REGISTER)(LWZHANDLE, HWND);
typedef void (LWZCALL *Ptr_LWZ_SET_NOTIFY_EX)(LWZNOTIFYPROC_EX, void *, LWZDEVICELIST *);
typedef BOOL (LWZCALL *Ptr_LWZ_GET_DEVICE_INFO)(LWZHANDLE, LWZDEVICEINFO *);

class LedWizModule : public QObject
{
    Q_OBJECT

public:
    explicit LedWizModule(QObject *parent = nullptr);
    ~LedWizModule();

    // Set the HWND of the LEDWiz DLL receiver
    void setWinID(HWND handle);

public slots:
    // Collect LEDWiz data
    void collectLedWizData();

    // Set pin state
    void setPinState(quint8 id, quint8 pin, bool state);

    // Set power level
    void setPowerLevel(quint8 id, quint8 pin, quint8 power);

    // Set RGB LED color
    void setRGBColor(quint8 id, quint8 pin, quint8 valueR, quint8 valueG, quint8 valueB);

    // Set pulse rate
    void setPulseRate(quint8 id, quint8 pulse);

    // Turn all lights on one board off
    void turnAllLightsOff(quint8 id);

    // Refresh devices
    void refreshDevices();

signals:
    // Send LEDWiz device list to DeviceWindow
    void ledWizDeviceList(const QList<LedWizData> &devices);

    // Show error message in main thread
    void showErrorMessage(const QString &title, const QString &message);

private:
    // Instance for managing the external DLL (LEDWiz64.dll)
    QLibrary m_ledWizLib;

    // Structure from LEDWiz.h, which stores the hardware handles of all connected devices
    LWZDEVICELIST m_deviceList;

    // Initial zero initialization of the entire array
    LedWizCache m_deviceCache[LEDWIZMAXDEVICES] = {};

    // Number of LedWiz devices
    int numberLedWizDevices = 0;

    // HWND
    HWND m_attachedHwnd = nullptr;

    // Function pointer instances
    Ptr_LWZ_SBA f_LWZ_SBA = nullptr;
    Ptr_LWZ_PBA f_LWZ_PBA = nullptr;
    Ptr_LWZ_REGISTER f_LWZ_REGISTER = nullptr;
    Ptr_LWZ_SET_NOTIFY_EX f_LWZ_SET_NOTIFY_EX = nullptr;
    Ptr_LWZ_GET_DEVICE_INFO f_LWZ_GET_DEVICE_INFO = nullptr;

    // Device notification callback
    static void LWZCALLBACK deviceNotificationCallback(void *puser, int32_t reason, LWZHANDLE hlwz);

    // Handle device change
    void handleDeviceChange(int32_t reason, LWZHANDLE hlwz);

    // Validate power value
    BYTE validatePowerValue(quint8 value);

    // Write SBA (Set Bank Attributes) cache to device
    void updateSBA(quint8 id);

    // Write PBA (Profile Bank Attributes) cache to device
    void updatePBA(quint8 id);

    // Reset cache to defaults
    // (Turn all lights on one board off and set power level to 49)
    void resetCacheToDefaults(quint8 id);

    // Turn the lights on all boards off
    void turnLightsOnAllBoardsOff();
};

#endif // LEDWIZMODULE_H
