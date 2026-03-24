#ifndef WINMSGMODULE_H
#define WINMSGMODULE_H

#include <QObject>
#include <QCoreApplication>
#include <QAbstractNativeEventFilter>
#include <QTimer>
#include <QMap>

#include <windows.h>

class WinMsgModule : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit WinMsgModule(QObject *parent = nullptr);
    ~WinMsgModule();

    // Set the HWND of the Windows message system receiver
    void setWinID(HWND handle);

public slots:
    // Find the hidden output window
    void findHiddenWindow();

    // Connect the Windows message system and wait for connection
    void connectWinMsg();

    // Disconnect the Windows message system
    void disconnectWinMsg();

    // When the Windows message system connects, it calls this slot, which emit another signal to
    // OutputHookerCore to let it know that it is connected
    void winMsgConnected();

    // When the Windows message system disconnects, it calls this slot, which emit another signal to
    // OutputHookerCore to let it know that it is disconnected
    void winMsgDisconnected();

protected:
    // Native event filter for intercepting Windows events
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

signals:
    // Signal sent to OutputHookerCore with read data
    void dataRead(const QString &signal, const QString &data);

    // Connection signals sent to OutputHookerCore
    void winMsgConnectedSignal();
    void winMsgDisconnectedSignal();

    // Send out no used data
    void filteredData(const QString &signal, const QString &data);

    // When Game has started (got mame_start)
    void gameHasStarted(const QString &data);

    // When an empty game has started
    void emptyGameHasStarted();

    // When game has stopped (got mame_stop)
    void gameHasStopped();

private:
    // Register with MAME output window
    void registerWithMame();

    // Process and emit signal
    void processAndEmitSignal(QString signalName, QString value);

    // Handle of the MAME instance
    HWND m_mameHwnd;

    // Handle of the application (for replies)
    HWND m_receiverHwnd;

    // Client ID
    const uint m_clientId = 2323;

    // Timer
    QTimer *p_waitForHiddenWindow;

    // Mapping of values from unknown output name
    QMap<uint, QString> m_pendingValues;

    // Mapping of output names from ID
    QMap<uint, QString> m_outputNames;

    // MAME Message IDs
    UINT OM_MAME_START;
    UINT OM_MAME_STOP;
    UINT OM_MAME_UPDATE_STATE;
    UINT OM_MAME_REGISTER_CLIENT;
    UINT OM_MAME_UNREGISTER_CLIENT;
    UINT OM_MAME_GET_ID_STRING;

    // Game is running or not
    bool inGame;

    // Stop connecting to the Windows message system
    bool stopConnecting;

public:
    // Is the Windows message system connected
    bool isConnected;

    // Is the Windows message system trying to connect
    bool isConnecting;
};

#endif // WINMSGMODULE_H
