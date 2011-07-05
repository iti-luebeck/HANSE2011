#ifndef TASKTIMERSUBMERGEDFORM_H
#define TASKTIMERSUBMERGEDFORM_H

#include <TaskTimerSubmerged/tasktimersubmerged.h>

#include <QWidget>
#include <log4qt/logger.h>


class Module_Simulation;

namespace Ui {
    class TaskTimerSubmergedForm;
}

class TaskTimerSubmergedForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskTimerSubmergedForm(TaskTimerSubmerged *tts, QWidget *parent = 0);
    ~TaskTimerSubmergedForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskTimerSubmergedForm *ui;

    Log4Qt::Logger *logger;
    TaskTimerSubmerged* tasktimersubmerged;

public slots:
    void on_applyButton_clicked();


};

#endif // TASKTIMERSUBMERGEDFORM_H
