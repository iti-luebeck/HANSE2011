#include "qsonarview.h"

#include <QtGui>

QSonarView::QSonarView(QWidget* parent)
    : QGraphicsView(parent)
{
    logger = Log4Qt::Logger::logger("SonarRecorder");
    this->setBackgroundBrush(QBrush(QColor("black")));
}

void QSonarView::wheelEvent(QWheelEvent *event)
{
    qreal d = event->delta() / 100.0;
    if (d>0)
        this->scale(d,d);
    else
        this->scale(-1/d,-1/d);
    event->accept();
}
