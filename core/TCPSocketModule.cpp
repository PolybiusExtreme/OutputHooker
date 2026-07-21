/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#include "TCPSocketModule.h"

#include "../Global.h"

TCPSocketModule::TCPSocketModule(QObject *parent)
    : QObject{parent}
{
    // Game is not running
    inGame = false;

    // TCP Socket is not connected
    isConnected = false;

    // TCP Socket is not trying to connect
    isConnecting = false;

    // TCP socket is set to keep connecting
    stopConnecting = false;

    p_outputTCPSocket = new QTcpSocket(this);

    connect(p_outputTCPSocket, &QTcpSocket::readyRead, this, &TCPSocketModule::tcpReadData);
    connect(p_outputTCPSocket, &QTcpSocket::connected, this, &TCPSocketModule::tcpSocketConnected);
    connect(p_outputTCPSocket, &QTcpSocket::disconnected, this, &TCPSocketModule::tcpSocketDisconnected);
    connect(p_outputTCPSocket, &QTcpSocket::errorOccurred, this, &TCPSocketModule::tcpSocketError);

    p_waitForConnection = new QTimer(this);
    p_waitForConnection->setInterval(TCPTIMERTIME);
    p_waitForConnection->setSingleShot(true);
    connect(p_waitForConnection, &QTimer::timeout, this, &TCPSocketModule::tcpConnect);
}

TCPSocketModule::~TCPSocketModule()
{
    if (p_outputTCPSocket)
    {
        p_outputTCPSocket->abort();
    }
}

// Connect the TCP Socket
void TCPSocketModule::tcpConnect()
{
    // Clear the stop that tcpDisconnect() set, so the TCP Socket connects again after the
    // OutputHookerCore has been stopped and started. WinMsgModule::connectWinMsg() does
    // the same. The flag still keeps tcpSocketDisconnected() and tcpSocketError() from
    // starting the reconnect timer after a deliberate disconnect
    stopConnecting = false;

    if (!isConnected && !isConnecting)
    {
        isConnecting = true;
        p_outputTCPSocket->connectToHost(QHostAddress::LocalHost, TCPHOSTPORT);
    }
}

// Disconnect the TCP Socket
void TCPSocketModule::tcpDisconnect()
{
    // TCP socket stops connecting
    stopConnecting = true;

    // Stop reconnect timer
    p_waitForConnection->stop();

    // Close TCP Socket
    p_outputTCPSocket->abort();

    isConnected = false;
    isConnecting = false;
    inGame = false;
}

// Read the TCP Socket and forward it to OutputHookerCore
void TCPSocketModule::tcpReadData()
{
    // Read the TCP Socket data
    QByteArray rawData = p_outputTCPSocket->readAll();

    QString message = QString::fromUtf8(rawData);

    // If there are multiple data lines, they will be separated into lines, using \r or \n
    // If it had 2 data lines together, then \r would be at end, which is chopped off
    // and middle QRegularExpression endLines("[\r\n]");
    QStringList tcpSocketReadData = message.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);

    for (int i = 0; i < tcpSocketReadData.count(); i++)
    {
        QString line = tcpSocketReadData[i].trimmed();
        if (line.isEmpty())
            continue;

        QStringList splitData = line.split("=");
        if (splitData.count() < 2)
            continue;

        QString signal = splitData[0].trimmed();
        QString value = splitData[1].trimmed();

        // Check if game has stopped
        if (signal == MAMESTOP || signal == GAMESTOP)
        {
            emit gameHasStopped();

            inGame = false;

            emit dataRead(signal, value);
        }
        // Check for game starting
        else if (signal == MAMESTART || signal == GAMESTART)
        {
            inGame = true;

            if (value == MAMEEMPTY)
            {
                emit emptyGameHasStarted();
            }
            else
            {
                emit gameHasStarted(value);
            }
        }
        else if (inGame)
        {
            // MAME specific signals (Pause/Orientation)
            if (signal.startsWith("mame", Qt::CaseInsensitive))
            {
                if (signal.contains("pause", Qt::CaseInsensitive))
                {
                    signal = PAUSE;
                }
                else if (signal.contains("orientation", Qt::CaseInsensitive))
                {
                    signal.replace(MAMEORIENTATION, ORIENTATION);
                }
            }
            emit dataRead(signal, value);
        }
    }
}

// When the TCP Socket connects, it calls this slot, which emit a signal to
// OutputHookerCore to let it know that it is connected
void TCPSocketModule::tcpSocketConnected()
{
    isConnected = true;
    isConnecting = false;

    p_waitForConnection->stop();

    emit tcpConnectedSignal();
}

// When the TCP Socket disconnects, it calls this slot, which emit a signal to
// OutputHookerCore to let it know that it is disconnected
void TCPSocketModule::tcpSocketDisconnected()
{
    isConnected = false;
    isConnecting = false;
    inGame = false;

    emit tcpDisconnectedSignal();

    if (!stopConnecting)
    {
        p_waitForConnection->start();
    }
}

// TCP Socket error process
void TCPSocketModule::tcpSocketError(QAbstractSocket::SocketError socketError)
{
    isConnecting = false;
    isConnected = false;

    if (!stopConnecting)
    {
        p_waitForConnection->start();
    }
}
