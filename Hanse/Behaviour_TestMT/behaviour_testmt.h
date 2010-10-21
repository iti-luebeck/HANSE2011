#ifndef TestMTBEHAVIOUR_H
#define TestMTBEHAVIOUR_H

#include <Framework/robotbehaviour_mt.h>

#include <Behaviour_TestMT/testmtform.h>
//#include <Framework/robotmodule_mt.h>

class TestMTForm;

class Behaviour_TestMT : public RobotBehaviour_MT
{
    Q_OBJECT
public:
    Behaviour_TestMT(QString id);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    /** starts GoalFollow Behaviour */
    void start();
    /** stops GoalFollow Behaviour */
    void stop();

    void reset();
    /** returns true if Behaviour is active
        return false if the Behaviour is not active
    */
    bool isActive();


private:
public slots:

private slots:

};


#endif // TestMTBEHAVIOUR_H
