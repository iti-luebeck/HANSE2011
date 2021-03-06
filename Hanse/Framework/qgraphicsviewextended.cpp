#include "qgraphicsviewextended.h"
#include <QtGui>

QGraphicsViewExtended::QGraphicsViewExtended( QWidget* parent )
    : QGraphicsView(parent)
{
    this->setBackgroundBrush(QBrush(QColor("black")));
    this->setDragMode(QGraphicsView::ScrollHandDrag);
}

QGraphicsViewExtended::~QGraphicsViewExtended()
{
}

void QGraphicsViewExtended::wheelEvent(QWheelEvent *event)
{
    this->setTransformationAnchor( QGraphicsView::AnchorUnderMouse );

    // Scale view.
    qreal d = event->delta() / 100.0;
    if ( d>0 && this->transform().m22()<2000 )
        this->scale(d,d);
    else if ( d<0 && this->transform().m22()>0.1 )
        this->scale(-1/d,-1/d);

    event->accept();
}

void QGraphicsViewExtended::keyPressEvent( QKeyEvent *event )
{
    QGraphicsView::keyPressEvent(event);
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
    case Qt::Key_Plus:
        if ( this->transform().m22()<2000 )
            this->scale(1.1,1.1);
        break;
    case Qt::Key_Minus:
        if ( this->transform().m22()>0.1 )
            this->scale(1/1.1,1/1.1);
        break;

    }
}

void QGraphicsViewExtended::mouseDoubleClickEvent( QMouseEvent *event )
{
    QGraphicsView::mouseDoubleClickEvent(event);
    mouseDoubleClickEventAt( this->mapToScene( event->pos() ) );
}

void QGraphicsViewExtended::mouseReleaseEvent( QMouseEvent *event )
{
    QGraphicsView::mouseReleaseEvent(event);
    mouseReleaseEventAt( this->mapToScene( event->pos() ) );
}
