#include "LedWizModule.h"

//LedWiz SDK
#include "Windows.h"
#include "LEDWiz.h"

LedWizModule::LedWizModule(QObject *parent)
    : QObject{parent}, m_ledWizLib("LEDWiz64")
{
    // Load DLL
    if (m_ledWizLib.load())
    {
        f_LWZ_SBA = (Ptr_LWZ_SBA)m_ledWizLib.resolve("LWZ_SBA");
        f_LWZ_PBA = (Ptr_LWZ_PBA)m_ledWizLib.resolve("LWZ_PBA");
        auto f_LWZ_SET_NOTIFY = (Ptr_LWZ_SET_NOTIFY)m_ledWizLib.resolve("LWZ_SET_NOTIFY");

        if (f_LWZ_SBA && f_LWZ_PBA && f_LWZ_SET_NOTIFY)
        {
            // Initialize devices
            m_deviceList.numdevices = 0;
            f_LWZ_SET_NOTIFY(nullptr, &m_deviceList);
            numberLedWizDevices = m_deviceList.numdevices;

            for (int i = 0; i < numberLedWizDevices; ++i)
            {
                resetCacheToDefaults(static_cast<quint8>(i));
                updateSBA(i);
                updatePBA(i);
            }
        }
        else
        {
            QString errorMsg = "Could not resolve functions in LEDWiz64.dll!";
            emit showErrorMessage("LEDWiz64.dll - Error", errorMsg);
        }
    }
    else
    {
        QString errorMsg = "LEDWiz64.dll not found. LEDWiz support disabled!";
        emit showErrorMessage("LEDWiz64.dll - Error ", errorMsg);
    }
}

LedWizModule::~LedWizModule()
{
    // Turn the lights on all boards off
    turnLightsOnAllBoardsOff();

    // Unload DLL
    m_ledWizLib.unload();
}

// Set pin state
void LedWizModule::setPinState(quint8 id, quint8 pin, bool state)
{
    if (id < numberLedWizDevices)
    {
        int bankIndex = (pin - 1) / 8;
        int bitIndex = (pin - 1) % 8;

        if (state)
            m_deviceCache[id].sbaBanks[bankIndex] |= (1 << bitIndex);
        else
            m_deviceCache[id].sbaBanks[bankIndex] &= ~(1 << bitIndex);

        updateSBA(id);
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id + 1) + " is not in the valid LEDWiz controller list!";
        emit showErrorMessage("Invalid LEDWiz Controller ID!", errorMsg);
    }
}

// Set power level
void LedWizModule::setPowerLevel(quint8 id, quint8 pin, quint8 power)
{
    if (id < numberLedWizDevices)
    {
        m_deviceCache[id].pbaLevels[pin - 1] = validatePowerValue(power);
        updatePBA(id);
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id + 1) + " is not in the valid LEDWiz controller list!";
        emit showErrorMessage("Invalid LEDWiz Controller ID!", errorMsg);
    }
}

// Set RGB LED color
void LedWizModule::setRGBColor(quint8 id, quint8 pin, quint8 valueR, quint8 valueG, quint8 valueB)
{
    if (id < numberLedWizDevices)
    {
        LedWizCache &cache = m_deviceCache[id];
        cache.pbaLevels[pin - 1] = validatePowerValue(valueR);
        cache.pbaLevels[pin]     = validatePowerValue(valueG);
        cache.pbaLevels[pin + 1] = validatePowerValue(valueB);
        updatePBA(id);
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id + 1) + " is not in the valid LEDWiz controller list!";
        emit showErrorMessage("Invalid LEDWiz Controller ID!", errorMsg);
    }
}

// Set pulse rate
void LedWizModule::setPulseRate(quint8 id, quint8 pulse)
{
    if (id < numberLedWizDevices)
    {
        m_deviceCache[id].globalPulseSpeed = pulse;
        updateSBA(id);
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id + 1) + " is not in the valid LEDWiz controller list!";
        emit showErrorMessage("Invalid LEDWiz Controller ID!", errorMsg);
    }
}

// Turn all lights on one board off
void LedWizModule::turnAllLightsOff(quint8 id)
{
    if (id < numberLedWizDevices)
    {
        resetCacheToDefaults(id);
        updateSBA(id);
        updatePBA(id);
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id + 1) + " is not in the valid LEDWiz controller list!";
        emit showErrorMessage("Invalid LEDWiz Controller ID!", errorMsg);
    }
}

// Validate power level
BYTE LedWizModule::validatePowerValue(quint8 value)
{
    // Range 0-48: Mapping to 1-49
    if (value <= 48)
    {
        return static_cast<BYTE>(value + 1);
    }

    // Range 129-132: Special effects (leave unchanged)
    if (value >= 129 && value <= 132)
    {
        return static_cast<BYTE>(value);
    }

    // Fallback for invalid values
    return (BYTE)49;
}

// Write SBA (Set Bank Attributes) cache to device
void LedWizModule::updateSBA(quint8 id)
{
    if (id < numberLedWizDevices && f_LWZ_SBA)
    {
        LWZHANDLE h = m_deviceList.handles[id];
        LedWizCache &cache = m_deviceCache[id];
        f_LWZ_SBA(h, cache.sbaBanks[0], cache.sbaBanks[1], cache.sbaBanks[2], cache.sbaBanks[3], cache.globalPulseSpeed);
    }
}

// Write PBA (Profile Bank Attributes) cache to device
void LedWizModule::updatePBA(quint8 id)
{
    if (id < numberLedWizDevices && f_LWZ_PBA)
    {
        LWZHANDLE h = m_deviceList.handles[id];
        f_LWZ_PBA(h, m_deviceCache[id].pbaLevels);
    }
}

// Reset cache to defaults
void LedWizModule::resetCacheToDefaults(quint8 id)
{
    if (id >= LEDWIZMAXDEVICES)
        return;

    m_deviceCache[id] = LedWizCache{};

    for (int i = 0; i < 32; ++i)
    {
        m_deviceCache[id].pbaLevels[i] = 49;
    }

    m_deviceCache[id].globalPulseSpeed = 7;
}

// Turn the lights on all boards off
void LedWizModule::turnLightsOnAllBoardsOff()
{
    for (int i = 0; i < numberLedWizDevices; ++i)
    {
        turnAllLightsOff(i);
    }
}
