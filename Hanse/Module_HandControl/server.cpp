#include <QtNetwork>
#include <iostream>
#include <stdlib.h>
#include <string>
#include "server.h"

Server::Server() {
    stream = NULL;
    tcpSocket = NULL;
    tcpServer = NULL;
}

void Server::open()
{
    qDebug() << "server THREAD ID";
    qDebug() << QThread::currentThreadId();

    if(tcpServer == NULL)
    {
        tcpServer = new QTcpServer(this);
        connect(tcpServer, SIGNAL(newConnection()), this, SLOT(openSocket()));
    }
    QMutexLocker l(&modulMutex);
    if (!tcpServer->listen(QHostAddress::Any,1234)) {
        emit healthProblem("Could not bind to port.");
        tcpServer->close();
    }
}

void Server::openSocket() {
    std::cout << "openSocket()" << std::endl;

    // we allow only one connection at a time
    if (tcpSocket) {
        tcpSocket->close();
        tcpSocket=NULL;
    }
    tcpSocket = tcpServer->nextPendingConnection();

    if (stream) {
        delete stream;
        stream = NULL;
    }
    stream = new QDataStream(tcpSocket);
    stream->setVersion(QDataStream::Qt_4_6);

    connect(tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
    emit statusChanged();
}

void Server::receiveMessage() {

    signed short forwardSpeed, angularSpeed, upDownSpeed;
    bool emergencyButton, stHandControl;
    while(tcpSocket && tcpSocket->bytesAvailable())
        *stream >> forwardSpeed >> angularSpeed >> upDownSpeed >> emergencyButton >> stHandControl;

    if (stHandControl)
        emit startHandControl();


    if (emergencyButton)
        emit emergencyStop();
    else
        emit newMessage(forwardSpeed,angularSpeed,upDownSpeed);
}

void Server::close() {
    qDebug() << "serverCL THREAD ID";
    qDebug() << QThread::currentThreadId();

    QMutexLocker l(&modulMutex);
    if(tcpServer) {
        tcpServer->close();
        tcpServer =  NULL;
    }
}

bool Server::isConnected()
{
    return tcpSocket != NULL && tcpSocket->isOpen();
}

void Server::clientDisconnected()
{
    if (tcpSocket) {
        tcpSocket->close();
        tcpSocket = NULL;
    }
    if (stream) {
        delete stream;
        stream = NULL;
    }
    emit statusChanged();
}
