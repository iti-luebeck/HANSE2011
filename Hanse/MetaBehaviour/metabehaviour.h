//#ifndef METABEHAVIOUR_H
//#define METABEHAVIOUR_H

//#include <Framework/robotmodule.h>
//#include <Framework/robotbehaviour.h>

//class Module_ThrusterControlLoop;
//class Behaviour_PipeFollowing;
//class Module_HandControl;
//class ModulesGraph;
//class Module_PressureSensor;
//class Behaviour_PipeFollowing;
//class Behaviour_BallFollowing;
//class Behaviour_TurnOneEighty;
//class Behaviour_WallFollowing;
//class MetaBehaviourForm;
//class CommandCenter;

//class MetaBehaviour : public RobotModule
//{
//    friend class MetaBehaviourForm;
//    Q_OBJECT
//public:
//    MetaBehaviour(QString id, ModulesGraph* graph, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Behaviour_PipeFollowing* pipe,Behaviour_BallFollowing* ball, Behaviour_TurnOneEighty* o80, Behaviour_WallFollowing* wall, CommandCenter* comm);

//    QList<RobotModule*> getDependencies();

//    QWidget* createView(QWidget *parent);


//private:
//    Module_ThrusterControlLoop* tcl;
//    Module_HandControl* handControl;
//    Module_PressureSensor* pressure;
//    Behaviour_PipeFollowing* pipe;
//    Behaviour_BallFollowing* ball;
//    Behaviour_TurnOneEighty* o80;
//    Behaviour_WallFollowing* wall;
//    CommandCenter* comm;
//    ModulesGraph* craph;
//    QList<RobotBehaviour*> behaviours;
//    QTimer depthWaitTimer;
//    QTimer timeoutTimer;
//    bool reachedEOP;
//    void init();

//private slots:
//    void finishedPipe(RobotBehaviour*,bool);
//    void depthChanged(float);
//    void stateTimeout();

//    void badHealth(RobotModule *m);
//    void finishedTurn(RobotBehaviour*, bool success);

//public slots:
//    void terminate();
//    void reset();
//    void emergencyStop();
//    void startHandControl();
//    void testPipe();
//    void pipeFollowForward();
//    void simpleForward();
//    void simple180deg();
//    void fullProgram();

//signals:
////    void emergencyStopp();
//    void setDepth(float depth);
//    void setForwardSpeed(float forwardSpeed);
//    void setAngularSpeed(float angularSpeed);
//    /* define a start/stop signal for every behaviour */
//    void stopAllBehaviours();
//    void resetTCL();
//    void startPipeFollow();
//    void stopPipeFollow();
//    void startTurnO80();
//    void stopTurnO80();
//    void startBallFollow();
//    void stopBallFollow();
//    void startHandCtr();
//    void stopHandCtr();

//};


//#endif // METABEHAVIOUR_H
