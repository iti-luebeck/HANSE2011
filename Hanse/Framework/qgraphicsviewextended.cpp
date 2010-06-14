#include "qgraphicsviewextended.h"
#include <QtGui>

QGraphicsViewExtended::QGraphicsViewExtended( QWidget* parent )
    : QGraphicsView(parent)
{
    this->setBackgroundBrush(QBrush(QColor("black")));
}

void QGraphicsViewExtended::wheelEvent(QWheelEvent *event)
{
    this->setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
    // Center view on the current mouse position.
    //this->centerOn( this->mapToScene( event->pos() ) );

    // Scale view.
    qreal d = event->delta() / 100.0;
    if (d>0)
        this->scale(d,d);
    else
        this->scale(-1/d,-1/d);
    event->accept();
}

void QGraphicsViewExtended::keyPressEvent( QKeyEvent *event )
{
    this->setTransformationAnchor( QGraphicsView::NoAnchor );
    switch( event->key() )
    {
    case Qt::Key_Up:
        this->translate( 0, 10 );
        break;
    case Qt::Key_Down:
        this->translate( 0, -10 );
        break;
    case Qt::Key_Left:
        this->translate( 10, 0 );
        break;
    case Qt::Key_Right:
        this->translate( -10, 0 );
        break;
    }
}
