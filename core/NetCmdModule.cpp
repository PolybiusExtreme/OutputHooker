/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#include "NetCmdModule.h"

NetCmdModule::NetCmdModule(QObject *parent)
    : QObject{parent}
{
    tcp1IsConnected = false;
    tcp2IsConnected = false;

    p_tcpSocket1 = new QTcpSocket(this);
    p_tcpSocket2 = new QTcpSocket(this);
    p_udpSocket = new QUdpSocket(this);
    p_networkManager = new QNetworkAccessManager(this);

    // TCP socket 1 - Signals & Slots
    connect(p_tcpSocket1, &QTcpSocket::readyRead, this, &NetCmdModule::tcpSocketReadData);
    connect(p_tcpSocket1, &QTcpSocket::connected, this, &NetCmdModule::tcpSocketConnected);
    connect(p_tcpSocket1, &QTcpSocket::disconnected, this, &NetCmdModule::tcpSocketDisconnected);

    // TCP socket 2 - Signals & Slots
    connect(p_tcpSocket2, &QTcpSocket::readyRead, this, &NetCmdModule::tcpSocketReadData);
    connect(p_tcpSocket2, &QTcpSocket::connected, this, &NetCmdModule::tcpSocketConnected);
    connect(p_tcpSocket2, &QTcpSocket::disconnected, this, &NetCmdModule::tcpSocketDisconnected);

    // HTTP network access manager - Signal & Slot
    connect(p_networkManager, &QNetworkAccessManager::finished, this, &NetCmdModule::onHttpFinished);
}

NetCmdModule::~NetCmdModule()
{
    // Disconnect TCP Socket 1
    if (p_tcpSocket1->state() == QAbstractSocket::ConnectedState)
    {
        p_tcpSocket1->disconnectFromHost();
    }

    // Disconnect TCP Socket 2
    if (p_tcpSocket2->state() == QAbstractSocket::ConnectedState)
    {
        p_tcpSocket2->disconnectFromHost();
    }

    // Disconnect network access manager
    p_networkManager->disconnect();

    QList<QNetworkReply*> activeReplies = p_networkManager->findChildren<QNetworkReply*>();

    for (QNetworkReply* reply : std::as_const(activeReplies))
    {
        if (reply->isRunning())
        {
            reply->abort();
        }
    }
}

QTcpSocket* NetCmdModule::getTargetSocket(const quint8 &socket)
{
    if (socket == 1) return p_tcpSocket1;
    if (socket == 2) return p_tcpSocket2;
    return nullptr;
}

// Connect to TCP host
void NetCmdModule::connectTcpHost(const quint8 &socket, const QString &host, quint16 port)
{
    QTcpSocket *targetSocket = getTargetSocket(socket);

    //Validation and connection establishment
    if (targetSocket)
    {
        if (targetSocket->state() == QAbstractSocket::UnconnectedState)
        {
            targetSocket->connectToHost(host, port);
        }
        else
        {
            emit showErrorMessage("TCP - Connect - Error!", QString("Socket %1 is already connected!").arg(socket));
        }
    }
    else
    {
        emit showErrorMessage("TCP - Connect - Error!", "Invalid Socket!");
    }
}

// Disconnect TCP host
void NetCmdModule::disconnectTcpHost(const quint8 &socket)
{
    QTcpSocket *targetSocket = getTargetSocket(socket);

    // Validation and connection termination
    if (targetSocket)
    {
        if (targetSocket->state() == QAbstractSocket::ConnectedState)
        {
            targetSocket->disconnectFromHost();
        }
        else
        {
            emit showErrorMessage("TCP - Disconnect - Error!", QString("Socket %1 is already disconnected!").arg(socket));
        }
    }
    else
    {
        emit showErrorMessage("TCP - Disconnect - Error!", "Invalid Socket!");
    }
}

// Send TCP command
void NetCmdModule::sendTcpCommand(const quint8 &socket, const QByteArray &command)
{
    QTcpSocket *targetSocket = getTargetSocket(socket);

    // Validation and command transmission
    if (targetSocket)
    {
        if (targetSocket->state() == QAbstractSocket::ConnectedState)
        {
            targetSocket->write(command);
            targetSocket->flush();
        }
        else if (targetSocket->state() == QAbstractSocket::ConnectingState || targetSocket->state() == QAbstractSocket::HostLookupState)
        {
            if (socket == 1)
            {
                tcp1Queue.append(command);
            }
            else if (socket == 2)
            {
                tcp2Queue.append(command);
            }
        }
        else
        {
            QString errorMsg = "The TCP command could not be sent!\n" + targetSocket->errorString();
            emit showErrorMessage("TCP - Send Command - Error!", errorMsg);
        }
    }
    else
    {
        emit showErrorMessage("TCP - Send Command - Error!", "Invalid Socket!");
    }
}

// Send UDP command
void NetCmdModule::sendUdpCommand(const quint8 &type, const QString &address, quint16 port, const QString &command)
{
    QHostAddress host(address);

    if (type == 1)
    {
        QByteArray datagram = command.toUtf8();

        qint64 sent = p_udpSocket->writeDatagram(datagram, host, port);

        if (sent == -1)
        {
            QString errorMsg = "The UDP (ASCII) command could not be sent!\n" + p_udpSocket->errorString();
            emit showErrorMessage("Send UDP Command - Error!", errorMsg);
        }
    }
    else if (type == 2)
    {
        QByteArray datagram = QByteArray::fromHex(command.toUtf8().replace(" ", ""));

        qint64 sent = p_udpSocket->writeDatagram(datagram, host, port);

        if (sent == -1)
        {
            QString errorMsg = "The UDP (Hex) command could not be sent!\n" + p_udpSocket->errorString();
            emit showErrorMessage("Send UDP Command - Error!", errorMsg);
        }
    }
    else
    {
        QString errorMsg = "Unknown transmission type!";
        emit showErrorMessage("Send UDP Command - Error!", errorMsg);
    }
}

// Send HTTP GET request
void NetCmdModule::sendHttpGetRequest(const QString &urlString)
{
    QUrl url(urlString);

    if (!url.isValid())
    {
        emit showErrorMessage("HTTP GET Request - Error!", "Invalid URL!");
        return;
    }

    // Extract the IP address or hostname
    QString hostIp = url.host();

    // Start the ping test in the background
    pingBeforeGet(hostIp, urlString);
}

// Send HTTP POST request
void NetCmdModule::sendHttpPostRequest(const QString &urlString, const QString &contentType, const QByteArray &data)
{
    QUrl url(urlString);

    if (!url.isValid())
    {
        emit showErrorMessage("HTTP POST Request - Error!", "Invalid URL!");
        return;
    }

    // Extract the IP address or hostname
    QString hostIp = url.host();

    // Start the ping test in the background
    pingBeforePost(hostIp, urlString, contentType, data);
}

// TCP socket 1 slots
void NetCmdModule::tcpSocketReadData()
{
    QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());

    if (!senderSocket)
        return;

    if (senderSocket == p_tcpSocket1)
    {
        tcp1ReadData = p_tcpSocket1->readAll();
    }
    else if (senderSocket == p_tcpSocket2)
    {
        tcp2ReadData = p_tcpSocket2->readAll();
    }
}

// TCP socket connected
void NetCmdModule::tcpSocketConnected()
{
    QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());

    if (!senderSocket)
        return;

    if (senderSocket == p_tcpSocket1)
    {
        tcp1IsConnected = true;

        while (!tcp1Queue.isEmpty())
        {
            p_tcpSocket1->write(tcp1Queue.takeFirst());
        }
        p_tcpSocket1->flush();
    }
    else if (senderSocket == p_tcpSocket2)
    {
        tcp2IsConnected = true;

        while (!tcp2Queue.isEmpty())
        {
            p_tcpSocket2->write(tcp2Queue.takeFirst());
        }
        p_tcpSocket2->flush();
    }
}

// TCP socket disconnected
void NetCmdModule::tcpSocketDisconnected()
{
    QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());

    if (!senderSocket)
        return;

    if (senderSocket == p_tcpSocket1)
    {
        tcp1IsConnected = false;
        tcp1Queue.clear();
    }
    else if (senderSocket == p_tcpSocket2)
    {
        tcp2IsConnected = false;
        tcp2Queue.clear();
    }
}

// Process the HTTP response
void NetCmdModule::onHttpFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray responseData = reply->readAll();
    }
    else
    {
        emit showErrorMessage("HTTP Request Failed!", reply->errorString());
    }
    reply->deleteLater();
}


// Ping before HTTP GET request
void NetCmdModule::pingBeforeGet(const QString &hostIp, const QString &urlString)
{
    QProcess *pingProcess = new QProcess(this);
    QStringList arguments;


    // -n 1 (1 packet), -w 1000 (1 second timeout)
    arguments << "-n" << "1" << "-w" << "1000" << hostIp;
    pingProcess->start("ping", arguments);

    connect(pingProcess, &QProcess::finished, this, [this, urlString, pingProcess](int exitCode)
    {
        // exitCode 0 = Ping was successful, host is here!
        if (exitCode == 0)
        {
            QUrl url(urlString);
            QNetworkRequest request(url);
            p_networkManager->get(request);
        }
        else
        {
            QString errorMsg = "The target IP address " + QUrl(urlString).host() + " is not reachable on the network or is not responding!";
            emit showErrorMessage("HTTP GET Request - Error!", errorMsg);

        }
        pingProcess->deleteLater();
    });
}

// Ping before HTTP POST request
void NetCmdModule::pingBeforePost(const QString &hostIp, const QString &urlString, const QString &contentType, const QByteArray &data)
{
    QProcess *pingProcess = new QProcess(this);
    QStringList arguments;

    // -n 1 (1 packet), -w 1000 (1 second timeout)
    arguments << "-n" << "1" << "-w" << "1000" << hostIp;
    pingProcess->start("ping", arguments);

    connect(pingProcess, &QProcess::finished, this, [this, urlString, contentType, data, pingProcess](int exitCode)
    {
        // exitCode 0 = Ping was successful, host is here!
        if (exitCode == 0)
        {
            QUrl url(urlString);
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
            p_networkManager->post(request, data);
        }
        else
        {
            QString errorMsg = "The target IP address " + QUrl(urlString).host() + " is not reachable on the network or is not responding!";
            emit showErrorMessage("HTTP POST Request - Error!", errorMsg);
        }
        pingProcess->deleteLater();
    });
}
