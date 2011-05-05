#ifndef TASKHANDCONTROLFORM_H
#define TASKHANDCONTROLFORM_H

#include <TaskHandControl/taskhandcontrol.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskHandControl;
class Module_Simulation;

namespace Ui {
    class TaskHandControlForm;
}

class TaskHandControlForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskHandControlForm(TaskHandControl *thc, QWidget *parent = 0);
    ~TaskHandControlForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskHandControlForm *ui;

    Log4Qt::Logger *logger;
    TaskHandControl* taskhandcontrol;

public slots:
    void on_applyButton_clicked();

signals:
     void handControlFinishedSignal();

};

#endif // TASKHANDCONTROLFORM_H
