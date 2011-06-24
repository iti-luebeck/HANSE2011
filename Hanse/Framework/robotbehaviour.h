#ifndef ROBOTBEHAVIOUR_H
#define ROBOTBEHAVIOUR_H

#include <Framework/robotmodule.h>

class RobotBehaviour : public RobotModule
{
    Q_OBJECT
public:
    RobotBehaviour(QString id);

    virtual bool isActive() = 0;

public slots:

    /**
      * start the behaviour
      */
    virtual void startBehaviour() = 0;

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
    void finished(RobotBehaviour* module, bool success);

    void started(RobotBehaviour* module);

};

#endif // ROBOTBEHAVIOUR_H
