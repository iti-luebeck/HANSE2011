#ifndef TASKWALLFOLLOWINGFORM_H
#define TASKWALLFOLLOWINGFORM_H

#include <TaskWallFollowing/taskwallfollowing.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskWallFollowing;
class Module_Simulation;

namespace Ui {
    class TaskWallFollowingForm;
}

class TaskWallFollowingForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskWallFollowingForm(TaskWallFollowing *tt, QWidget *parent = 0);
    ~TaskWallFollowingForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskWallFollowingForm *ui;

    Log4Qt::Logger *logger;
    TaskWallFollowing* taskwallfollowing;

signals:
    void newSchDesSignal(QString taskName, QString newD);

public slots:
    void on_applyButton_clicked();

};

#endif // TASKWALLFOLLOWINGFORM_H
