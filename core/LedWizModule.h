#ifndef LEDWIZMODULE_H
#define LEDWIZMODULE_H

#include <QObject>
#include <QLibrary>

//LedWiz SDK
#include "LEDWiz.h"

#include "../Global.h"

// Definition of function pointers for the DLL functions
typedef void (LWZCALL *Ptr_LWZ_SBA)(LWZHANDLE, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
typedef void (LWZCALL *Ptr_LWZ_PBA)(LWZHANDLE, uint8_t const *);
typedef void (LWZCALL *Ptr_LWZ_SET_NOTIFY)(LWZNOTIFYPROC, LWZDEVICELIST *);

class LedWizModule : public QObject
{
    Q_OBJECT

public:
    explicit LedWizModule(QObject *parent = nullptr);
    ~LedWizModule();

public slots:
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

signals:
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

    // Function pointer instances
    Ptr_LWZ_SBA f_LWZ_SBA = nullptr;
    Ptr_LWZ_PBA f_LWZ_PBA = nullptr;

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
