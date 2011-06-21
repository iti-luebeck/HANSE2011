#ifndef TASKMIDWATERTARGETFORM_H
#define TASKMIDWATERTARGETFORM_H

#include <TaskMidwaterTarget/taskmidwatertarget.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskMidwaterTarget;
class Module_Simulation;

namespace Ui {
    class TaskMidwaterTargetForm;
}

class TaskMidwaterTargetForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskMidwaterTargetForm(TaskMidwaterTarget *tmt, QWidget *parent = 0);
    ~TaskMidwaterTargetForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskMidwaterTargetForm *ui;

    Log4Qt::Logger *logger;
    TaskMidwaterTarget* taskmidwatertarget;

signals:
    void updateTaskSettingsSignal();

public slots:
    void on_applyButton_clicked();

};

#endif // TASKMIDWATERTARGETFORM_H
