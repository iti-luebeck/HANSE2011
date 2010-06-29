#include "eventthread.h"

EventThread::EventThread()
{
    //this->exec();
    timer.moveToThread( this );
}

void EventThread::run()
{
    this->exec();
}

void EventThread::moveTimer()
{
    timer.moveToThread( this );
}
