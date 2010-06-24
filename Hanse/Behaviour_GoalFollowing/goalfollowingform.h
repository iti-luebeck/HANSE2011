#ifndef GoalFollowingFORM_H
#define GoalFollowingFORM_H

#include <Behaviour_GoalFollowing/behaviour_goalfollowing.h>

#include <QWidget>

class Behaviour_GoalFollowing;

namespace Ui {
    class GoalFollowingForm;
}

class GoalFollowingForm : public QWidget {
    Q_OBJECT
public:
    GoalFollowingForm(QWidget *parent, Behaviour_GoalFollowing *GoalFollowing);
    ~GoalFollowingForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::GoalFollowingForm *ui;
    Behaviour_GoalFollowing *goalfollow;

private slots:



private slots:
    void on_saveAndApplyButton_clicked();
    void on_stopGoalFollowingButton_clicked();
    void on_startGoalFollwoingButton_clicked();
};

#endif // GoalFollowingFORM_H
