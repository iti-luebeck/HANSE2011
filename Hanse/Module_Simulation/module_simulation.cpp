#include "module_simulation.h"
#include "simulation_form.h"

Module_Simulation::Module_Simulation(QString id)
    : RobotModule(id)
{
    thread.start();

    setDefaultValue("server_ip_adress", "localhost");
    setDefaultValue("server_port", 80);

    thread.start();
    //timer.moveToThread(&thread);

    reset();

    this->start();
}

Module_Simulation::~Module_Simulation()
{
}

void Module_Simulation::terminate()
{
    RobotModule::terminate();
    QTimer::singleShot(0, &timer, SLOT(stop()));
}

void Module_Simulation::reset()
{
    RobotModule::reset();
}

QList<RobotModule*> Module_Simulation::getDependencies()
{
    QList<RobotModule*> ret;
    return ret;
}

QWidget* Module_Simulation::createView(QWidget* parent)
{
    return new Simulation_Form(this, parent);
}

void Module_Simulation::doHealthCheck()
{
    if (!isEnabled())
        return;

    setHealthToOk();
}

void Module_Simulation::start()
{
    //qDebug("Client start...");
    qDebug("%s", qPrintable(QString("Client start...")));
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readFortune()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    this->requestNewFortune();
}

void Module_Simulation::requestNewFortune()
{
    tcpSocket->abort();
    tcpSocket->connectToHost("localhost",80);
    tcpSocket->write(QString("Depth\n").toAscii().data(), QString("Depth\n").length());
    tcpSocket->flush();
    //tcpSocket->waitForBytesWritten(0);
    /*while(!(tcpSocket->waitForReadyRead(0))){}

        qDebug("Client read...");
        char readbuf[512] = {0};
        read_ret =  tcpSocket->read(readbuf, 512);
        QString qs = QString(readbuf);*/
}

void Module_Simulation::readFortune()
{
    qDebug("Client read...");
    char readbuf[512] = {0};
   // tcpSocket->abort();
    //int bla = tcpSocket->bytesAvailable();
    qDebug("%s", qPrintable(QString(tcpSocket->errorString())));
    //qDebug("%s", qPrintable(QString(tcpSocket->isOpen())));
    //qDebug("%s", qPrintable(QString(tcpSocket->isReadable())));
    //read_ret =  tcpSocket->read(readbuf, 512);
    //QString qs = QString(readbuf);
    /*if(qs.startsWith("Depth"))
    {
        qDebug("it is depth!!!");
        QString qs2 = qs.section(' ',1,1);
        float ff = qs2.toFloat();
        if(ff < 0){
            qDebug("zero");
        }
        qDebug() << ff << endl;
        qDebug(qPrintable(qs2));
    }
    qDebug("%s", qPrintable(QString(readbuf)));
    */
}

void Module_Simulation::displayError(QAbstractSocket::SocketError Error)
{
    switch (Error) {
    case QAbstractSocket::RemoteHostClosedError:
        qDebug("%s", qPrintable(QString("RemoteHostClosedError...")));
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug("%s", qPrintable(QString("HostNotFoundError...")));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug("%s", qPrintable(QString("ConnectionRefusedError...")));
        break;
    default:
        qDebug("%s", qPrintable(QString(tcpSocket->errorString())));
        break;
    }
}
