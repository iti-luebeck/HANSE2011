#include <QtNetwork>
#include <iostream>
#include "client.h"


Client::Client()
{
    tcpSocket = new QTcpSocket(this);
    dataStream = new QDataStream(tcpSocket);
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

Client::~Client()
{
}

void Client::sendMessage(signed short forward, signed short rotation, signed short upDown, bool emergencyButton, bool handControl)
{
    openConnection();
    if (tcpSocket->isOpen()) {
        *dataStream << forward;
        *dataStream << rotation;
        *dataStream << upDown;
        *dataStream << emergencyButton;
        *dataStream << handControl;
        tcpSocket->flush();
    } else {
        qDebug() << "NOT CONNECTED with Hanse.";
    }
}

void Client::disconnected()
{
    qDebug() << "DISCONNECTED from host";
    tcpSocket->close();
}

void Client::setConfig(QString hosts, int port)
{
    this->hosts = hosts;
    this->port = port;
}

void Client::openConnection()
{
    // TODO: at the moment we don't detect stale (open but dead) connections.
    if (tcpSocket->isOpen()) {
        return;
    }

    // try all available hostnames, stop once we're connected.
    foreach(QString host, hosts.split(",")) {
        tcpSocket->connectToHost(host,port);
        bool status = tcpSocket->waitForConnected(100);
        if (status) {
            qDebug() << "SUCCESSFULLY connected to host " << host << "on port " << port;
            break;
        }
        qDebug() << "FAILED to connect to host " << host << "on port " << port;
        tcpSocket->close();
    }
}
