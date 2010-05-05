#include <QtNetwork>
#include <iostream>
#include "client.h"


Client::Client(int port)
{
    tcpSocket = new QTcpSocket(this);

    tcpSocket->connectToHost("localhost",port);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
}

Client::~Client() {
    //delete tcpSocket;
}

void Client::sendMessage(char* message)
{
    short len = message[0];
    //tcpSocket->write(message);
    tcpSocket->write(message, len);
}

void Client::receiveMessage()
{
    //char received[256];

    //qint64 count = tcpSocket->read(received, 255);
    //memcpy(++received,message,count);
}
