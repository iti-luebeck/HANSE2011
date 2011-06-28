#ifndef BallFollowingFORM_H
#define BallFollowingFORM_H

#include <Behaviour_BallFollowing/behaviour_ballfollowing.h>
#include <opencv/cxcore.h>

#include <QWidget>

class Behaviour_BallFollowing;

namespace Ui {
    class BallFollowingForm;
}

class BallFollowingForm : public QWidget {
    Q_OBJECT
public:
    BallFollowingForm(QWidget *parent, Behaviour_BallFollowing *BallFollowing);
    ~BallFollowingForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::BallFollowingForm *ui;
    Behaviour_BallFollowing *ballfollow;

private slots:
    void on_testVideoButton_clicked();
    void on_saveAndApplyButton_clicked();
    void on_stopBallFollowingButton_clicked();
    void on_startBallFollwoingButton_clicked();

signals:
    void doTest( QString path );
};

#endif // BallFollowingFORM_H
