#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include <Framework/robotmodule.h>
#include <QtCore>

class Module_Simulation;

class CommandCenter : public RobotModule
{
    Q_OBJECT

     public:

        void pleaseStop();
        void run(void);

    private:
        CommandCenter* c;
        Module_Simulation* sim;
        QTextStream* fileStream;
        bool running;

public:
    CommandCenter(QString id, Module_Simulation *sim);
    ~CommandCenter();

    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();

    void newSchedule();

private slots:
    void gotEnabledChanged(bool);

signals:
    void error();

private:
    QTimer timer;

    void init();
};

#endif // COMMANDCENTER_H
