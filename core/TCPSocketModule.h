#ifndef TCPSOCKETMODULE_H
#define TCPSOCKETMODULE_H

#include <QObject>
#include <QTimer>
#include <QByteArray>
#include <QTcpSocket>
#include <QHostAddress>
#include <QRegularExpression>

class TCPSocketModule : public QObject
{
    Q_OBJECT

public:
    explicit TCPSocketModule(QObject *parent = nullptr);
    ~TCPSocketModule();

    // TCP Socket Class
    QTcpSocket *p_outputTCPSocket;

public slots:
    // Connect the TCP Socket and wait for connection
    void connectTCP();

    // Disconnect the TCP Socket
    void disconnectTCP();

    // Read the TCP Socket and forward it to OutputHookerCore
    void tcpReadData();

    // When the TCP Socket connects, it calls this slot, which emit another signal to
    // OutputHookerCore to let it know that it is connected
    void tcpSocketConnected();

    // When the TCP Socket disconnects, it calls this slot, which emit another signal to
    // OutputHookerCore to let it know that it is disconnected
    void tcpSocketDisconnected();

    // Timeout process
    void tcpConnectionTimeOut();

signals:
    // Signal sent to OutputHookerCore with read data
    void dataRead(const QString &signal, const QString &data);

    // Connection signals sent to OutputHookerCore
    void tcpConnectedSignal();
    void tcpDisconnectedSignal();

    // Send out no used data
    void filteredData(const QString &signal, const QString &data);

    // When game has started (got mame_start or game)
    void gameHasStarted(const QString &data);

    // When an empty game has started
    void emptyGameHasStarted();

    // When game has stopped (got mame_stop or game_stop)
    void gameHasStopped();

private:
    // Timer
    QTimer *p_waitForConnection;

    // Read data
    QByteArray readData;

    // Game is running or not
    bool inGame;

    // Stop connecting to TCP Server
    bool stopConnecting;

public:
    // TCP Socket is connected
    bool isConnected;

    // TCP Socket is trying to connect
    bool isConnecting;
};

#endif // TCPSOCKETMODULE_H
