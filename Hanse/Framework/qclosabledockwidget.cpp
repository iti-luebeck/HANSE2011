#include "qclosabledockwidget.h"
#include <QtCore>

QClosableDockWidget::QClosableDockWidget(QString title, QWidget *parent, QString uuid)
    : QDockWidget(title, parent)
{
    if (uuid.length()>0)
        setObjectName(uuid);
    else
        setObjectName(QUuid::createUuid());

    setAllowedAreas(Qt::BottomDockWidgetArea |
                                Qt::TopDockWidgetArea |
                                Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);
}

void QClosableDockWidget::closeEvent(QCloseEvent *event)
{
    QDockWidget::closeEvent(event);
    QSettings s;
    s.beginGroup("docks");
    QStringList ds = s.value("openDataWidgets").toStringList();
    ds.removeAll(objectName());
    s.setValue("openDataWidgets",ds);

    ds = s.value("openHealthWidgets").toStringList();
    ds.removeAll(objectName());
    s.setValue("openHealthWidgets",ds);

    s.remove(objectName());
}
