#ifndef FORM_NAVIGATION_H
#define FORM_NAVIGATION_H

#include <QWidget>
#include <QListWidgetItem>
#include <QGraphicsScene>
#include <Framework/position.h>
#include <Module_Navigation/module_navigation.h>

namespace Ui {
    class Form_Navigation;
}

class Form_Navigation : public QWidget {
    Q_OBJECT
public:
    Form_Navigation( Module_Navigation *nav, QWidget *parent = 0);
    ~Form_Navigation();
    QMutex *getSceneMutex();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_Navigation *ui;
    Module_Navigation *nav;
    QMap<QString, Waypoint> waypoints;

signals:
    void removedWaypoint( QString name );
    void goToPosition(QString name);

private slots:
    void on_resumeButton_clicked();
    void on_pauseButton_clicked();
    void on_clearGoalButton_clicked();
    void on_gotoButton_clicked();
    void on_applyButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_removeButton_clicked();
    void on_addButton_clicked();
    void on_listWidget_itemDoubleClicked(QListWidgetItem* item);

public slots:
    void updateList( QMap<QString, Waypoint> waypoints );
};

#endif // FORM_NAVIGATION_H
