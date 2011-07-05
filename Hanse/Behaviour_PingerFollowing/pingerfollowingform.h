#ifndef PINGERFOLLOWINGFORM_H
#define PINGERFOLLOWINGFORM_H

#include <Behaviour_PingerFollowing/behaviour_pingerfollowing.h>

#include <QWidget>
#include "Module_Pinger/module_pinger.h"

class Behaviour_PingerFollowing;

namespace Ui {
    class PingerFollowingForm;
}

class PingerFollowingForm : public QWidget {
    Q_OBJECT
public:
    PingerFollowingForm(QWidget *parent, Behaviour_PingerFollowing *pingerfollowing);
    ~PingerFollowingForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::PingerFollowingForm *ui;
    Behaviour_PingerFollowing *pingerfollow;


signals:
    void startBehaviour();
    void stopBehaviour();

public slots:



private slots:
    void on_stopButton_clicked();
    void on_startButton_clicked();
    void on_applyButton_clicked();
};

#endif
