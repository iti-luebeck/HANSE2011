#ifndef QSONARVIEW_H
#define QSONARVIEW_H

#include <QGraphicsView>
#include <log4qt/logger.h>

class QSonarView : public QGraphicsView
{
public:
    QSonarView(QWidget* parent);

    void wheelEvent(QWheelEvent *event);

private:
    Log4Qt::Logger *logger;

};

#endif // QSONARVIEW_H
