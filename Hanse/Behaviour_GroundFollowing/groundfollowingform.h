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

protected:
    void changeEvent(QEvent *e);

private:
    Ui::GroundFollowingForm *ui;
    Behaviour_GroundFollowing *groundfollow;

private slots:
    void on_saveButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();

signals:
   void startGroundFollow();
   void stopGroundFollow();

};

#endif // GROUNDFOLLOWINGFORM_H
