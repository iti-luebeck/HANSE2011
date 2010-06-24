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

    // we allow only one connection at a time
    if (stream) {
        tcpSocket->close();
        return;
    }

    stream = new QDataStream(tcpSocket);

    connect(tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
    emit statusChanged();
}

void Server::receiveMessage() {

    signed short forwardSpeed, angularSpeed, upDownSpeed;
    bool emergencyButton;
    *stream >> forwardSpeed >> angularSpeed >> upDownSpeed >> emergencyButton;

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
    delete stream;
    stream = NULL;
    emit statusChanged();
}
