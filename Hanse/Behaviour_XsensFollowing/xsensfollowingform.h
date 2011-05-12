#ifndef XSENSFOLLOWINGFORM_H
#define XSENSFOLLOWINGFORM_H

#include <Behaviour_XsensFollowing/behaviour_xsensfollowing.h>

#include <QWidget>

class Behaviour_XsensFollowing;

namespace Ui {
    class XsensFollowingForm;
}

class XsensFollowingForm : public QWidget {
    Q_OBJECT
public:
    XsensFollowingForm(QWidget *parent, Behaviour_XsensFollowing *comp);
    ~XsensFollowingForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::XsensFollowingForm *ui;
    Behaviour_XsensFollowing *xsens;

signals:
    void startBehaviour();
    void stopBehaviour();
    void refreshHeading();
private slots:
    void on_setHeading_clicked();
    void on_apply_clicked();
    void on_stopButton_clicked();
    void on_startButton_clicked();
};

#endif // XSENSFOLLOWINGFORM_H
