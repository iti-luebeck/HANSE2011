#ifndef TASKXSENSNAVIGATION_H
#define TASKXSENSNAVIGATION_H

#include <Framework/robotbehaviour.h>

#define XSENS_NAV_STATE_MOVE_START      "move to start"
#define XSENS_NAV_STATE_FOLLOW_XSENS    "do xsens following"
#define XSENS_NAV_STATE_MOVE_B          "move to B"
#define XSENS_NAV_STATE_TURN_180        "turn 180"
#define XSENS_NAV_STATE_MOVE_END        "move to end"
#define XSENS_NAV_STATE_DONE            "done"

class Module_Simulation;
class Module_Navigation;
class Behaviour_XsensFollowing;
class Behaviour_TurnOneEighty;

class TaskXsensNavigation : public RobotBehaviour
{
    Q_OBJECT

    //-----------------------------------------
    // METHODS
    //-----------------------------------------
public:
    TaskXsensNavigation(QString id, Module_Simulation *sim, Behaviour_XsensFollowing *xf, Module_Navigation *n, Behaviour_TurnOneEighty *o180);
    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();
    bool isActive();

private:
    void init();
    void terminate();
    void initBehaviourParameters();

    //-----------------------------------------
    // PARAMETERS
    //-----------------------------------------
private:
    bool active;
    Module_Simulation *sim;
    Behaviour_XsensFollowing *xsensfollow;
    Module_Navigation *navi;
    Behaviour_TurnOneEighty *turn180;

    QTimer taskTimer;
    QString state;

    //-----------------------------------------
    // SLOTS
    //-----------------------------------------
public slots:
    void startBehaviour();
    void stop();

    void reachedWaypoint(QString waypoint);
    void xsensFollowFinished();
    void turn180Finished();

    void controlEnabledChanged(bool b);
    void emergencyStop();
    void timeoutStop();

private slots:
    void stateChanged();

    //-----------------------------------------
    // SIGNALS
    //-----------------------------------------
signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void stopSignal();
    void updateSettings();
    void newState(QString task, QString state);
    void newStateOverview(QString state);
    void setNewWaypoint(QString waypoint);


};

#endif // TASKXSENSNAVIGATION_H
