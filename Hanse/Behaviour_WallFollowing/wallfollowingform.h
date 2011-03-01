#ifndef WALLFOLLOWINGFORM_H
#define WALLFOLLOWINGFORM_H

#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>

#include <QWidget>

class Behaviour_WallFollowing;

namespace Ui {
    class WallFollowingForm;
}

class WallFollowingForm : public QWidget {
    Q_OBJECT
public:
    WallFollowingForm(QWidget *parent, Behaviour_WallFollowing *wallfollowing);
    ~WallFollowingForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::WallFollowingForm *ui;
    Behaviour_WallFollowing *wallfollowing;

signals:
    void startBehaviour();
    void stopBehaviour();
private slots:
    void on_stopButton_clicked();
    void on_startButton_clicked();
};

#endif // WALLFOLLOWINGFORM_H
