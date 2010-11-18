
#ifndef SERVER_H
#define SERVER_H

#include <QTcpSocket>
#include <QDataStream>
#include <QTCore>

class QTcpServer;

class Server : public QObject {
    Q_OBJECT

public:
    Server();
//    void close();
    bool isConnected();
    void open(int port);

public slots:
    void close();

signals:
    void newMessage(int forwardSpeed, int angularSpeed, int speedUpDown);
    void statusChanged();
    void healthProblem(QString);
    void emergencyStop();
    void startHandControl();

private slots:
    void openSocket();
    void receiveMessage();
    void clientDisconnected();

private:
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    QDataStream *stream;
    QMutex modulMutex;

};

#endif
