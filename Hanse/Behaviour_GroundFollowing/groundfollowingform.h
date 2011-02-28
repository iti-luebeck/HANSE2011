#ifndef GROUNDFOLLOWINGFORM_H
#define GROUNDFOLLOWINGFORM_H

#include <Behaviour_GroundFollowing/behaviour_groundfollowing.h>

#include <QWidget>

class Behaviour_GroundFollowing;

namespace Ui {
    class GroundFollowingForm;
}

class GroundFollowingForm : public QWidget
{
    Q_OBJECT
public:
    explicit GroundFollowingForm(QWidget *parent, Behaviour_GroundFollowing *groundfollowing);
    ~GroundFollowingForm();

private:
    Ui::GroundFollowingForm *ui;
    Behaviour_GroundFollowing *groundfollow;
};

#endif // GROUNDFOLLOWINGFORM_H
