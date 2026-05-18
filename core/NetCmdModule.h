/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#ifndef NETCMDMODULE_H
#define NETCMDMODULE_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QUdpSocket>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

class NetCmdModule : public QObject
{
    Q_OBJECT

public:
    explicit NetCmdModule(QObject *parent = nullptr);
    ~NetCmdModule();

    // Connect TCP host
    void connectTcpHost(const quint8 &socket, const QString &host, quint16 port);

    // Disconnect TCP host
    void disconnectTcpHost(const quint8 &socket);

    // Send TCP command
    void sendTcpCommand(const quint8 &socket, const QByteArray &command);

    // Send UDP command
    void sendUdpCommand(const quint8 &type, const QString &address, quint16 port, const QString &command);

    // Send HTTP GET request
    void sendHttpGetRequest(const QString &urlString);

    // Send HTTP POST request
    void sendHttpPostRequest(const QString &urlString, const QByteArray &data, const QString &contentType = "application/json");

public slots:
    // TCP socket slots
    void tcpSocketReadData();
    void tcpSocketConnected();
    void tcpSocketDisconnected();

    // Slot to process the HTTP response
    void onHttpFinished(QNetworkReply *reply);

signals:
    // Show error message in main thread
    void showErrorMessage(const QString &title, const QString &message);

private:
    // Get target socket by socket number
    QTcpSocket* getTargetSocket(const quint8 &socket);

    // Pointer - TCP socket 1
    QTcpSocket *p_tcpSocket1;

    // Pointer - TCP socket 2
    QTcpSocket *p_tcpSocket2;

    // Pointer - UDP socket
    QUdpSocket *p_udpSocket;

    // Pointer - HTTP network access manager
    QNetworkAccessManager *p_networkManager;

    bool tcp1IsConnected;
    bool tcp2IsConnected;
    QByteArray tcp1ReadData;
    QByteArray tcp2ReadData;
    QList<QByteArray> tcp1Queue;
    QList<QByteArray> tcp2Queue;
};

#endif // NETCMDMODULE_H
