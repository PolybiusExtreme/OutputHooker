#include "WinMsgModule.h"

#include "../Global.h"

WinMsgModule::WinMsgModule(QObject *parent)
    : QObject{parent}, m_mameHwnd(nullptr), m_receiverHwnd(nullptr)
{
    // Game is not running
    inGame = false;

    // Is the Windows message system connected
    isConnected = false;

    // Is the Windows message system trying to connect
    isConnecting = false;

    // Stop connecting to the Windows message system
    stopConnecting = false;

    // Registration of MAME-specific Windows messages
    OM_MAME_START = RegisterWindowMessage(L"MAMEOutputStart");
    OM_MAME_STOP = RegisterWindowMessage(L"MAMEOutputStop");
    OM_MAME_UPDATE_STATE = RegisterWindowMessage(L"MAMEOutputUpdateState");
    OM_MAME_REGISTER_CLIENT = RegisterWindowMessage(L"MAMEOutputRegister");
    OM_MAME_UNREGISTER_CLIENT = RegisterWindowMessage(L"MAMEOutputUnregister");
    OM_MAME_GET_ID_STRING = RegisterWindowMessage(L"MAMEOutputGetIDString");

    p_waitForHiddenWindow = new QTimer(this);
    p_waitForHiddenWindow->setInterval(WINMSGTIMERTIME);
    p_waitForHiddenWindow->setSingleShot(true);
    connect(p_waitForHiddenWindow, &QTimer::timeout, this, &WinMsgModule::findHiddenWindow);
}

WinMsgModule::~WinMsgModule()
{
    qApp->removeNativeEventFilter(this);

    delete p_waitForHiddenWindow;

    if (m_mameHwnd && m_receiverHwnd)
    {
        PostMessage(m_mameHwnd, OM_MAME_UNREGISTER_CLIENT, (WPARAM)m_receiverHwnd, m_clientId);
    }
}

// Set the HWND of the Windows message system receiver
void WinMsgModule::setWinID(HWND handle)
{
    m_receiverHwnd = handle;

    if (m_mameHwnd)
        registerWithMame();
}

// Find the hidden output window
void WinMsgModule::findHiddenWindow()
{
    m_mameHwnd = FindWindow(L"MAMEOutput", L"MAMEOutput");
}

// Connect the Windows message system and wait for connection
void WinMsgModule::connectWinMsg()
{
    stopConnecting = false;
    if (!isConnected && !isConnecting)
    {
        // Install native event filter
        qApp->installNativeEventFilter(this);

        // Start timer for FindHiddenWindow
        QTimer::singleShot(0, p_waitForHiddenWindow, qOverload<>(&QTimer::start));

        // Set the isConnecting bool to true
        isConnecting = true;
    }
}

// Disconnect the Windows message system
void WinMsgModule::disconnectWinMsg()
{
    // Set to stop Windows message system from trying to connect again
    stopConnecting = true;

    // Remove native event filter
    qApp->removeNativeEventFilter(this);

    // Stop timer for FindHiddenWindow
    QTimer::singleShot(0, p_waitForHiddenWindow, &QTimer::stop);

    isConnected = false;
    isConnecting = false;
    inGame = false;
}

// When the Windows message system connects, it calls this slot, which emit another signal to
// OutputHookerCore to let it know that it is connected
void WinMsgModule::winMsgConnected()
{
    isConnected = true;
    isConnecting = false;

    QTimer::singleShot(0, p_waitForHiddenWindow, &QTimer::stop);

    emit winMsgConnectedSignal();
}

// When the Windows message system disconnects, it calls this slot, which emit another signal to
// OutputHookerCore to let it know that it is disconnected
void WinMsgModule::winMsgDisconnected()
{
    isConnected = false;
    isConnecting = false;
    inGame = false;

    emit winMsgDisconnectedSignal();
}

// Native event filter for intercepting Windows events
bool WinMsgModule::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    MSG *msg = static_cast<MSG *>(message);

    if (msg->hwnd != m_receiverHwnd && msg->message != OM_MAME_START)
    {
        return false;
    }

    if (msg->message == OM_MAME_START)
    {
        if (m_mameHwnd != (HWND)msg->wParam)
        {
            m_mameHwnd = (HWND)msg->wParam;
            registerWithMame();
        }

        return false;
    }
    else if (msg->message == OM_MAME_STOP)
    {
        if (m_mameHwnd != nullptr)
        {
            m_mameHwnd = nullptr;

            emit gameHasStopped();
            inGame = false;

            winMsgDisconnected();
        }

        return false;
    }
    else if (msg->message == OM_MAME_UPDATE_STATE)
    {
        uint id = (uint)msg->wParam;
        QString value = QString::number((int)msg->lParam);

        if (!m_outputNames.contains(id))
        {
            // Name unknown -> buffer value and send request to MAME
            m_pendingValues[id] = value;
            PostMessage(m_mameHwnd, OM_MAME_GET_ID_STRING, (WPARAM)m_receiverHwnd, id);
        }
        else
        {
            // Name known -> process directly
            processAndEmitSignal(m_outputNames[id], value);
        }

        return true;
    }
    else if (msg->message == WM_COPYDATA)
    {
        PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)msg->lParam;
        if (pcds->dwData == 1)
        {
            uint32_t id = *(uint32_t*)pcds->lpData;
            char* namePtr = (char*)pcds->lpData + sizeof(uint32_t);
            QString name = QString::fromLatin1(namePtr);

            if (id == 0)
            {
                inGame = true;
                m_outputNames.clear();
                m_pendingValues.clear();
                if (name == MAMEEMPTY)
                    emit emptyGameHasStarted();
                else
                    emit gameHasStarted(name);
            }
            else
            {
                m_outputNames[id] = name;

                // Check if a value for this name is waiting in the buffer
                if (m_pendingValues.contains(id))
                {
                    processAndEmitSignal(name, m_pendingValues.take(id));
                }
            }

            return true;
        }
    }

    return false;
}

// Register with MAME output window
void WinMsgModule::registerWithMame()
{
    if (!m_mameHwnd || !m_receiverHwnd) return;
    {
        // Send registration to MAME
        PostMessage(m_mameHwnd, OM_MAME_REGISTER_CLIENT, (WPARAM)m_receiverHwnd, m_clientId);
        // Request game name (ID 0)
        PostMessage(m_mameHwnd, OM_MAME_GET_ID_STRING, (WPARAM)m_receiverHwnd, 0);

        winMsgConnected();
    }
}

// Process and emit signal
void WinMsgModule::processAndEmitSignal(QString signalName, QString value)
{
    if (signalName.startsWith("Mame"))
    {
        if (signalName.contains("Pause") && signalName.size() == 9)
        {
            signalName = PAUSE;
        }
        else if (signalName.contains("Orientation") && signalName.size() == 15)
        {
            signalName.replace(MAMEORIENTATION, ORIENTATION);
        }
    }

    emit dataRead(signalName, value);
}
