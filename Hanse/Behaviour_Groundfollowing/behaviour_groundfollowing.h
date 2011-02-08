#ifndef BEHAVIOUR_GROUNDFOLLOWING_H
#define BEHAVIOUR_GROUNDFOLLOWING_H

#include <Framework/robotbehaviour.h>
#include <Framework/eventthread.h>
#include <Behaviour_GroundFollowing/groundfollowingform.h>
#include <Module_EchoSounder/module_echosounder.h>

class Module_ThrusterControlLoop;
class GroundFollowingForm;

class Behaviour_GroundFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_GroundFollowing(QString id, Module_ThrusterControlLoop* tcl);
    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    void start();
    void stop();

    void reset();
    void terminate();

    bool isActive();
    void setValues(float distance, int range);

    void updateFromSettings();

private:
    // Update Values on GUI
    void updateData();

    Module_EchoSounder* echo;
    Module_ThrusterControlLoop* tcl;
    QTimer timer;
    EventThread updateThread;
//    QStringList files;
//    int fileIndex;

    //distance to ground
    float distance;
    int range;
    // average of the last measures
    float average[500];
    float messungen[10][500];

private slots:
    void timerSlot();

signals:
    void timerStart(int msec);
    void timerStop();
};

#endif // BEHAVIOUR_GROUNDFOLLOWING_H
