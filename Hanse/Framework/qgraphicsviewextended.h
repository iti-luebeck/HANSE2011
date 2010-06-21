#ifndef QGRAPHICSVIEWEXTENDED_H
#define QGRAPHICSVIEWEXTENDED_H

#include <QGraphicsView>
#include <QMutex>

class QGraphicsViewExtended : public QGraphicsView
{
    Q_OBJECT
public:
    QGraphicsViewExtended( QWidget* parent );
    ~QGraphicsViewExtended();

    void wheelEvent( QWheelEvent *event );
    void keyPressEvent( QKeyEvent *event );
    void mouseDoubleClickEvent( QMouseEvent *event );

signals:
    void mouseEventAt( QPointF point );

};

#endif // QGRAPHICSVIEWEXTENDED_H
