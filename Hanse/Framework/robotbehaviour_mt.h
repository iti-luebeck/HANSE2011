#ifndef ROBOTBEHAVIOUR_MT_H
#define ROBOTBEHAVIOUR_MT_H

#include <Framework/robotmodule_mt.h>

class RobotBehaviour_MT : public RobotModule_MT
{
    Q_OBJECT
public:
    RobotBehaviour_MT(QString id);

    virtual bool isActive() = 0;


public slots:

    /**
      * start the behaviour
      */
    virtual void start() = 0;

    /**
      * stop the behaviour
      */
    virtual void stop() = 0;

signals:


    /**
      * This signal is emitted when the behaviour finished its run and stops itself.
      *
      * "success" indicates if the behaviour succeeded in its task.
      */
    void finished(RobotBehaviour_MT* module, bool success);

    void started(RobotBehaviour_MT* module);

};

#endif // ROBOTBEHAVIOUR_MT_H
