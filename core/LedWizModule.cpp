/*
 * Original Copyright (c) 2026 PolybiusExtreme
 *
 * Licensed under the GNU GPLv3.
 */

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
        f_LWZ_REGISTER = (Ptr_LWZ_REGISTER)m_ledWizLib.resolve("LWZ_REGISTER");
        f_LWZ_SET_NOTIFY_EX = (Ptr_LWZ_SET_NOTIFY_EX)m_ledWizLib.resolve("LWZ_SET_NOTIFY_EX");
        f_LWZ_GET_DEVICE_INFO = (Ptr_LWZ_GET_DEVICE_INFO)m_ledWizLib.resolve("LWZ_GET_DEVICE_INFO");

        if (!f_LWZ_SBA || !f_LWZ_PBA || !f_LWZ_REGISTER || !f_LWZ_SET_NOTIFY_EX || !f_LWZ_GET_DEVICE_INFO)
        {
            QString errorMsg = "Could not resolve all functions in LEDWiz64.dll!";
            emit showErrorMessage("LEDWiz64.dll - Error", errorMsg);
        }
    }
    else
    {
        QString errorMsg = "LEDWiz64.dll not found!";
        emit showErrorMessage("LEDWiz64.dll - Error ", errorMsg);
    }
}

LedWizModule::~LedWizModule()
{
    // Turn the lights on all boards off
    turnLightsOnAllBoardsOff();

    // Unregister devices
    if (f_LWZ_REGISTER)
    {
        for (int i = 0; i < numberLedWizDevices; ++i)
        {
            f_LWZ_REGISTER(m_deviceList.handles[i], NULL);
        }
    }

    // Unload DLL
    m_ledWizLib.unload();
}

// Set the HWND of the LEDWiz DLL receiver
void LedWizModule::setWinID(HWND handle)
{
    if (!f_LWZ_SET_NOTIFY_EX)
        return;

    m_attachedHwnd = handle;
    m_deviceList.numdevices = 0;

    // Register callback
    f_LWZ_SET_NOTIFY_EX(&LedWizModule::deviceNotificationCallback, this, &m_deviceList);

    // Register and initialize existing devices
    numberLedWizDevices = m_deviceList.numdevices;
    for (int i = 0; i < numberLedWizDevices; ++i)
    {
        f_LWZ_REGISTER(m_deviceList.handles[i], m_attachedHwnd);

        resetCacheToDefaults(i);
        updateSBA(i);
        updatePBA(i);
    }

    collectLedWizData();
}

// Collect LEDWiz data
void LedWizModule::collectLedWizData()
{
    QList<LedWizData> deviceList;

    for (int i = 0; i < numberLedWizDevices; ++i)
    {
        LWZHANDLE h = m_deviceList.handles[i];
        LWZDEVICEINFO info;
        info.cbSize = sizeof(LWZDEVICEINFO);

        if (f_LWZ_GET_DEVICE_INFO && f_LWZ_GET_DEVICE_INFO(h, &info))
        {
            LedWizData data;
            data.id = i + 1;
            data.type = info.dwDevType;
            data.name = QString::fromLocal8Bit(info.szName);
            deviceList.append(data);
        }
    }
    emit ledWizDeviceList(deviceList);
}

// Set pin state
void LedWizModule::setPinState(quint8 id, quint8 pin, bool state)
{
    if (id < numberLedWizDevices)
    {
        // Pins are 1 based. Without this check a pin of 0 shifts by a negative amount
        // and a pin above the channel count writes past the bank array
        if (pin < 1 || pin > LEDWIZMAXPINS)
        {
            emit showErrorMessage("Invalid LEDWiz Pin!", "Pin " + QString::number(pin) + " is not in the range 1 - " + QString::number(LEDWIZMAXPINS) + "!");
            return;
        }

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
        if (pin < 1 || pin > LEDWIZMAXPINS)
        {
            emit showErrorMessage("Invalid LEDWiz Pin!", "Pin " + QString::number(pin) + " is not in the range 1 - " + QString::number(LEDWIZMAXPINS) + "!");
            return;
        }

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
        // An RGB LED uses the pin and the two channels after it, so the last pin that
        // still fits is two below the channel count
        if (pin < 1 || pin > LEDWIZMAXPINS - 2)
        {
            emit showErrorMessage("Invalid LEDWiz Pin!", "RGB pin " + QString::number(pin) + " is not in the range 1 - " + QString::number(LEDWIZMAXPINS - 2) + "!");
            return;
        }

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

// Refresh devices
void LedWizModule::refreshDevices()
{
    if (f_LWZ_SET_NOTIFY_EX)
    {
        f_LWZ_SET_NOTIFY_EX(&LedWizModule::deviceNotificationCallback, this, &m_deviceList);
    }
}

// Device notification callback
void LWZCALLBACK LedWizModule::deviceNotificationCallback(void *puser, int32_t reason, LWZHANDLE hlwz)
{
    auto *instance = static_cast<LedWizModule*>(puser);

    if (instance)
    {
        instance->handleDeviceChange(reason, hlwz);
    }
}

// Handle device change
void LedWizModule::handleDeviceChange(int32_t reason, LWZHANDLE hlwz)
{
    numberLedWizDevices = m_deviceList.numdevices;

    if (reason == LWZ_REASON_ADD)
    {
        if (f_LWZ_REGISTER && m_attachedHwnd)
        {
            f_LWZ_REGISTER(hlwz, m_attachedHwnd);
        }

        for (int i = 0; i < numberLedWizDevices; ++i)
        {
            if (m_deviceList.handles[i] == hlwz)
            {
                resetCacheToDefaults(i);
                updateSBA(i);
                updatePBA(i);
                break;
            }
        }
    }
    else if (reason == LWZ_REASON_DELETE)
    {
        if (f_LWZ_REGISTER)
        {
            f_LWZ_REGISTER(hlwz, NULL);
        }
    }

    collectLedWizData();
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
