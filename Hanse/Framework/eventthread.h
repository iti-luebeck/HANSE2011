#ifndef EVENTTHREAD_H
#define EVENTTHREAD_H

#include <QThread>
#include <QTimer>

class EventThread : public QThread
{
    Q_OBJECT
public:
    EventThread();
    void run();
    void moveTimer();

//public slots:
//    void timerStart( int msec );
//    void timerStop();

public:
    QTimer timer;
};

#endif // EVENTTHREAD_H
