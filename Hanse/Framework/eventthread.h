#ifndef EVENTTHREAD_H
#define EVENTTHREAD_H

#include <QThread>

class EventThread : public QThread
{
public:
    EventThread();
    void run();
};

#endif // EVENTTHREAD_H
