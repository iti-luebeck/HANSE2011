#ifndef WAYPOINTDIALOG_H
#define WAYPOINTDIALOG_H

#include <QDialog>
#include <Framework/position.h>

namespace Ui {
    class WaypointDialog;
}

class WaypointDialog : public QDialog {
    Q_OBJECT
public:
    WaypointDialog( QString name, double x, double y, double depth, double arrivalAngle, double exitAngle,
                    QWidget *parent = 0);
    ~WaypointDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::WaypointDialog *ui;

signals:
    void createdWaypoint( QString name, Position pos );

private slots:
    void on_buttonBox_accepted();
};

#endif // WAYPOINTDIALOG_H
