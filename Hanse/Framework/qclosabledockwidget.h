#ifndef QCLOSABLE_H
#define QCLOSABLE_H

#include <QDockWidget>
#include <QSettings>

class QClosableDockWidget : public QDockWidget
{
public:
    QClosableDockWidget(QString title, QWidget *parent, QString uuid);

protected:
    void closeEvent(QCloseEvent* event);

    QSettings s;

};

#endif // QCLOSABLE_H
