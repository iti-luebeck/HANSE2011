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

void Client::sendMessage(signed short forward, signed short rotation, signed short upDown, bool emergencyButton)
{
    openConnection();
    if (tcpSocket->isOpen()) {
        *dataStream << forward;
        *dataStream << rotation;
        *dataStream << upDown;
        *dataStream << emergencyButton;
    } else {
        qDebug() << "NOT CONNECTED with Hanse.";
    }
}

void Client::disconnected()
{
    qDebug() << "DISCONNECTED from host";
    tcpSocket->close();
}

void Client::openConnection()
{
    if (tcpSocket->isOpen()) {
        return;
    }

    int port = s.value("port", 1234).toInt();

    // try all available hostnames, stop once we're connected.
    foreach(QString host, s.value("hostname", "localhost").toStringList()) {
        qDebug() << "TRYING to connect to host " << host << "on port " << port;
        tcpSocket->connectToHost(host,port);
        bool status = tcpSocket->waitForConnected(100);
        if (status) {
            qDebug() << "SUCCESSFULLY connected to host " << host;
            break;
        }
        qDebug() << "FAILED to connect to host " << host;
        tcpSocket->close();
    }
}
