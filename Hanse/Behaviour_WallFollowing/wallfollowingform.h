#ifndef WALLFOLLOWINGFORM_H
#define WALLFOLLOWINGFORM_H

#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>

#include <QWidget>
#include "Module_EchoSounder/module_echosounder.h"
#include <QGraphicsScene>
#include <log4qt/logger.h>

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
    Behaviour_WallFollowing *wallfollow;
    QGraphicsScene scene;
    QQueue<QLinearGradient> dataQueue;
    QQueue<QGraphicsRectItem*> ritQueue;


signals:
    void startBehaviour();
    void stopBehaviour();

public slots:
    // Behaviour_WallFollowing -> WallFollowingForm
    void updateWallUi(const EchoReturnData data,float dist);
    void updateWallCase(QString caseW);
    void updateUiView();


private slots:
    void on_stopButton_clicked();
    void on_startButton_clicked();
};

#endif // WALLFOLLOWINGFORM_H
