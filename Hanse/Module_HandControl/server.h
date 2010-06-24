
#ifndef SERVER_H
#define SERVER_H

#include <QTcpSocket>
#include <QDataStream>

class QTcpServer;

class Server : public QObject {
    Q_OBJECT

public:
    Server();
    void close();
    bool isConnected();
    void open(int port);

signals:
    void newMessage(int forwardSpeed, int angularSpeed, int speedUpDown);
    void statusChanged();
    void healthProblem(QString);

private slots:
    void openSocket();
    void receiveMessage();
    void clientDisconnected();

private:
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    QDataStream *stream;

};

#endif
