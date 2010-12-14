#ifndef CompassFollowingFORM_H
#define CompassFollowingFORM_H

#include <Behaviour_CompassFollowing/behaviour_compassfollowing.h>

#include <QWidget>

class Behaviour_CompassFollowing;

namespace Ui {
    class CompassFollowingForm;
}

class CompassFollowingForm : public QWidget {
    Q_OBJECT
public:
    CompassFollowingForm(QWidget *parent, Behaviour_CompassFollowing *comp);
    ~CompassFollowingForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CompassFollowingForm *ui;
    Behaviour_CompassFollowing *comp;

signals:
    void startBehaviour();
    void stopBehaviour();
private slots:
    void on_stopButton_clicked();
    void on_startButton_clicked();
};

#endif // CompassFollowingFORM_H
