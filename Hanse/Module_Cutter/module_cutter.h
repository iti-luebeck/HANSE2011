#ifndef MODULE_CUTTER_H
#define MODULE_CUTTER_H

#include <Framework/robotmodule.h>
#include <Module_UID/module_uid.h>

class Module_Cutter : public RobotModule{
    Q_OBJECT


public:
    Module_Cutter(QString id, Module_UID *uid);
    QList<RobotModule*> getDependencies();
    bool shutdown();

    QWidget* createView(QWidget* parent);

private:
        Module_UID *uid;
        QTimer timer;
        bool isClosed;
        char cut;

        void closeCutter();
        void openCutter();

protected:
        void init();

public slots:
    void reset();
    void terminate();
    void gotEnabled(bool value);

private slots:
    void changeCutDirection();


protected slots:
    void doHealthCheck();

};

#endif
