/*
 * Original Copyright (c) 2026 PolybiusExtreme
 *
 * Licensed under the GNU GPLv3.
 */

#include "SdlCtrlModule.h"

SdlCtrlModule::SdlCtrlModule(QObject *parent)
    : QObject{parent}
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC))
    {
        // Initial scan at startup
        updateDevices();

        // Check for new devices/events every 1000ms
        QTimer *eventTimer = new QTimer(this);
        connect(eventTimer, &QTimer::timeout, this, &SdlCtrlModule::processEvents);
        eventTimer->start(1000);
    }
    else
    {
        QString errorMsg = QString("Failed to initialize USB HID!\n%1").arg(SDL_GetError());
        emit showErrorMessage("SDL Initialisation - Error!", errorMsg);
    }
}

SdlCtrlModule::~SdlCtrlModule()
{
    // Stop rumble on all controllers
    QMapIterator<int, SDL_Gamepad*> i(m_gamepads);
    while (i.hasNext())
    {
        i.next();
        SDL_Gamepad* gamepad = i.value();

        if (gamepad)
        {
            SDL_RumbleGamepad(gamepad, 0, 0, 0);
        }
    }

    // Close gamepad connections
    for (auto it = m_gamepads.begin(); it != m_gamepads.end(); ++it)
    {
        SDL_Gamepad* gamepad = it.value();

        if (gamepad)
        {
            SDL_CloseGamepad(gamepad);
        }
    }
    m_gamepads.clear();

    // Quit SDL
    SDL_Quit();
}

// Collect SDL data
void SdlCtrlModule::collectSdlData()
{
    QList<SdlCtrlData> deviceList;

    for (auto it = m_gamepads.begin(); it != m_gamepads.end(); ++it)
    {
        SdlCtrlData data;
        data.id = it.key();
        data.name = QString::fromUtf8(SDL_GetGamepadName(it.value()));
        deviceList.append(data);
    }
    emit sdlDeviceList(deviceList);
}

// Process SDL events
void SdlCtrlModule::processEvents()
{
    SDL_Event event;
    bool listChanged = false;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_GAMEPAD_ADDED)
        {
            updateDevices();
            listChanged = true;
        }
        else if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
        {
            SDL_JoystickID id = event.gdevice.which;
            if (m_gamepads.contains(id))
            {
                SDL_CloseGamepad(m_gamepads.value(id));
                m_gamepads.remove(id);
                listChanged = true;
            }
        }
    }

    if (listChanged)
    {
        collectSdlData();
    }
}

// Set gamecontroller rumble
void SdlCtrlModule::setRumble(int id, bool state, uint16_t leftStrength, uint16_t rightStrength, uint32_t duration)
{
    if (!m_gamepads.contains(id))
    {
        updateDevices();
    }

    SDL_Gamepad* gamepad = m_gamepads.value(id, nullptr);

    if (!gamepad)
    {
        QString errorMsg = QString("Controller ID %1 not found!").arg(id);
        emit showErrorMessage("SDL - Error!", errorMsg);
        return;
    }

    if (state == 1)
    {
        // Start rumble
        SDL_RumbleGamepad(gamepad, leftStrength, rightStrength, duration);

        // Stop rumble after duration timer runs out
        QTimer::singleShot(duration, this, [this, id](){ this->setRumble(id, 0, 0, 0, 0); });
    }
    else
    {
        // Stop rumble
        SDL_RumbleGamepad(gamepad, 0, 0, 0);
    }
}

// Update devices
void SdlCtrlModule::updateDevices()
{
    int count;
    SDL_JoystickID* joysticks = SDL_GetGamepads(&count);

    QSet<SDL_JoystickID> currentIds;

    if (joysticks)
    {
        for (int i = 0; i < count; ++i)
        {
            currentIds.insert(joysticks[i]);
        }
    }

    auto it = m_gamepads.begin();

    while (it != m_gamepads.end())
    {
        SDL_JoystickID id = it.key();

        if (!currentIds.contains(id))
        {
            if (it.value())
            {
                SDL_CloseGamepad(it.value());
            }
            it = m_gamepads.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (SDL_JoystickID id : currentIds)
    {
        if (!m_gamepads.contains(id))
        {
            SDL_Gamepad* gamepad = SDL_OpenGamepad(id);

            if (gamepad)
            {
                m_gamepads.insert(id, gamepad);
            }
        }
    }

    if (joysticks)
    {
        SDL_free(joysticks);
    }

    collectSdlData();
}
