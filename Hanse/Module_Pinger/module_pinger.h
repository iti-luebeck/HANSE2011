#ifndef MODULE_PINGER_H
#define MODULE_PINGER_H

#include <Framework/robotmodule.h>

class Module_Pinger : public RobotModule{
    Q_OBJECT


public:
    Module_Pinger(QString id);
    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget* parent);

private:


protected:
    void init();

public slots:
    void reset();
    void terminate();
    void gotEnabled(bool value);

private slots:



};

#endif
