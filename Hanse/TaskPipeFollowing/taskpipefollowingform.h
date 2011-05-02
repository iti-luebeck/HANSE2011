#ifndef TASKPIPEFOLLOWINGFORM_H
#define TASKPIPEFOLLOWINGFORM_H

#include <TaskPipeFollowing/taskpipefollowing.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskPipeFollowing;
class Module_Simulation;

namespace Ui {
    class TaskPipeFollowingForm;
}

class TaskPipeFollowingForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskPipeFollowingForm(TaskPipeFollowing *tpt, QWidget *parent = 0);
    ~TaskPipeFollowingForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskPipeFollowingForm *ui;

    Log4Qt::Logger *logger;
    TaskPipeFollowing* taskpipefollowing;

signals:
    void newSchDesSignal(QString taskName, QString newD);

public slots:
    void on_applyButton_clicked();

};

#endif // TASKPIPEFOLLOWINGFORM_H
