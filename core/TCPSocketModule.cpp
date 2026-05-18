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

    // Is the TCP Socket connected
    isConnected = false;

    // Is TCP Socket trying to connect
    isConnecting = false;

    // Stop connecting to TCP Server
    stopConnecting = false;

    p_outputTCPSocket = new QTcpSocket(this);

    connect(p_outputTCPSocket, &QTcpSocket::readyRead, this, &TCPSocketModule::tcpReadData);
    connect(p_outputTCPSocket, &QTcpSocket::connected, this, &TCPSocketModule::tcpSocketConnected);
    connect(p_outputTCPSocket, &QTcpSocket::disconnected, this, &TCPSocketModule::tcpSocketDisconnected);

    p_waitForConnection = new QTimer(this);
    p_waitForConnection->setInterval(TCPTIMERTIME);
    p_waitForConnection->setSingleShot(true);
    connect(p_waitForConnection, &QTimer::timeout, this, &TCPSocketModule::tcpConnectionTimeOut);
}

TCPSocketModule::~TCPSocketModule()
{
    p_waitForConnection->stop();
    delete p_waitForConnection;
}

// Connect the TCP Socket and wait for connection
void TCPSocketModule::connectTCP()
{
    stopConnecting = false;
    if (!isConnected && !isConnecting)
    {
        // Set the address for the TCP Socket
        p_outputTCPSocket->connectToHost(QHostAddress("127.0.0.1"), TCPHOSTPORT);

        // Start timer for connection
        p_waitForConnection->start();

        // Set the isConnecting bool
        isConnecting = true;

        // Wait for connection
        p_outputTCPSocket->waitForConnected(TIMETOWAIT);
    }
}

// Disconnect the TCP Socket
void TCPSocketModule::disconnectTCP()
{
    // Set to stop TCP Socket from trying to connect again
    stopConnecting = true;

    p_waitForConnection->stop();

    // Close TCP Socket
    p_outputTCPSocket->close();

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
        if (signal.size() == 9 && inGame)
        {
            if (signal[5] == 's' && signal[6] == 't' && signal[8] == 'p')
            {
                emit gameHasStopped();

                inGame = false;
            }

            emit dataRead(signal, value);
        }
        else
        {
            // Check for game starting
            if (signal == MAMESTART)
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
            else if (signal == GAMESTART)
            {
                inGame = true;

                emit gameHasStarted(value);
            }
            else
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
}

// When the TCP Socket connects, it calls this slot, which emit another signal to
// OutputHookerCore to let it know that it is connected
void TCPSocketModule::tcpSocketConnected()
{
    isConnected = true;
    isConnecting = false;

    p_waitForConnection->stop();

    emit tcpConnectedSignal();
}

// When the TCP Socket disconnects, it calls this slot, which emit another signal to
// OutputHookerCore to let it know that it is disconnected
void TCPSocketModule::tcpSocketDisconnected()
{
    isConnected = false;
    isConnecting = false;
    inGame = false;

    emit tcpDisconnectedSignal();
}

// Timeout process
void TCPSocketModule::tcpConnectionTimeOut()
{
    if (!isConnected && !stopConnecting && isConnecting)
    {
        if (p_outputTCPSocket->state() != QAbstractSocket::ConnectedState)
        {
            p_outputTCPSocket->connectToHost(QHostAddress("127.0.0.1"), TCPHOSTPORT);
            p_waitForConnection->start();
            p_outputTCPSocket->waitForConnected(TIMETOWAIT);
        }
    }
}
