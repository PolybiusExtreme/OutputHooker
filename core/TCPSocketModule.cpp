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
    if(!isConnected && !isConnecting)
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
    quint8 i;

    // Read the TCP Socket data
    readData = p_outputTCPSocket->readAll();

    // Convert to Byte Array
    QString message = QString::fromStdString(readData.toStdString());

    // Remove the \r at the end
    message.chop(1);

    // If there are multiple data lines, they will be separated into lines, using \r or \n
    // If it had 2 data lines together, then \r would be at end, which is chopped off
    // and middle QRegularExpression endLines("[\r\n]");
    QStringList tcpSocketReadData = message.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);

    for(i = 0; i < tcpSocketReadData.count(); i++)
    {
        // Get the Output signal name
        QStringList splitData = tcpSocketReadData[i].split(" = ", Qt::SkipEmptyParts);

        // Check if game has stopped
        if(splitData[0].size() == 9 && inGame)
        {
            if(splitData[0][5] == 's' && splitData[0][6] == 't' && splitData[0][8] == 'p')
            {
                emit gameHasStopped();
                inGame = false;

                if(splitData.count() == 1)
                    splitData.append("0");
            }
        }
        else
        {
            // Check for game starting
            if(splitData[0] == MAMESTART)
            {
                inGame = true;

                if(splitData[1] == MAMEEMPTY)
                    emit emptyGameHasStarted();
                else
                    emit gameHasStarted(splitData[1]);
            }
            else if(splitData[0] == GAMESTART)
            {
                inGame = true;

                emit gameHasStarted(splitData[1]);
            }
            else
            {
                if(splitData[0][0] == 'M' && splitData[0][1] == 'a' && splitData[0][2] == 'm')
                {
                    if(splitData[0][4] == 'P' && splitData[0].size() == 9)
                        splitData[0] = PAUSE;
                    else if(splitData[0][4] == 'O' && splitData[0].size() == 15)
                        splitData[0].replace(MAMEORIENTATION, ORIENTATION);
                }
                emit dataRead(splitData[0], splitData[1]);
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
    if(!isConnected && !stopConnecting && isConnecting)
    {
        if(p_outputTCPSocket->state() != QAbstractSocket::ConnectedState)
        {
            p_outputTCPSocket->connectToHost(QHostAddress("127.0.0.1"), TCPHOSTPORT);
            p_waitForConnection->start();
            p_outputTCPSocket->waitForConnected(TIMETOWAIT);
        }
    }
}
