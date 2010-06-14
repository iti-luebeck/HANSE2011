#ifndef QGRAPHICSVIEWEXTENDED_H
#define QGRAPHICSVIEWEXTENDED_H

#include <QGraphicsView>

class QGraphicsViewExtended : public QGraphicsView
{
public:
    QGraphicsViewExtended( QWidget* parent );

    void wheelEvent( QWheelEvent *event );
    void keyPressEvent( QKeyEvent *event );

};

#endif // QGRAPHICSVIEWEXTENDED_H
