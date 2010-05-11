#include "qsonarview.h"

#include <QtGui>

QSonarView::QSonarView(QWidget* parent)
    : QGraphicsView(parent)
{
    logger = Log4Qt::Logger::logger("SonarRecorder");
}

void QSonarView::wheelEvent(QWheelEvent *event)
{
    qreal d = event->delta() / 100.0;
    int sign = (d > 0) - (d < 0);
    qreal scale = sign*1.01;
    logger->debug(QString("scaling by %1").arg(scale));
    //this->scale(scale, scale);
    event->accept();
}
