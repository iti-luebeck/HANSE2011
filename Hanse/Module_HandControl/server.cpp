#include <QtNetwork>
#include <iostream>
#include <stdlib.h>
#include <string>
#include "server.h"

Server::Server() {
    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(openSocket()));
}

void Server::open(int port)
{
    if (!tcpServer->listen(QHostAddress::Any,port)) {
        emit healthProblem("Could not bind to port.");
        tcpServer->close();
    }
}

void Server::openSocket() {
    std::cout << "openSocket()" << std::endl;
    tcpSocket = tcpServer->nextPendingConnection();
    connect(tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
    emit statusChanged();
}

void Server::receiveMessage() {
    QByteArray a = tcpSocket->readLine(255);
    if (a.size()<6) {
        emit healthProblem("Received message to small.");
        return;
    }
    int angularSpeed = a.at(2);
    int forwardSpeed = a.at(3);
    int upDownSpeed = a.at(5);
    emit newMessage(forwardSpeed,angularSpeed,upDownSpeed);
}

void Server::close() {
    tcpServer->close();
}

bool Server::isConnected()
{
    return tcpSocket != NULL && tcpSocket->isOpen();
}

void Server::clientDisconnected()
{
    tcpSocket->close();
    emit statusChanged();
}
