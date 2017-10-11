#include "customgraphicsview.h"
#include "ui_mainwindow.h"
#include <QtWidgets>

CustomGraphicsView::CustomGraphicsView(QWidget *parent) : QGraphicsView(parent)
{

}

void CustomGraphicsView::wheelEvent(QWheelEvent *zoomEvent)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    static const double scaleFactor = 1.15;
    static double currentScale = 1.0;  // stores the current scale value.
    static const double scaleMin = 1.0; // defines the min scale limit.

    if(zoomEvent->delta() > 0)
    {
        scale(scaleFactor, scaleFactor);
        currentScale *= scaleFactor;
    }
    else if (currentScale > scaleMin) {
        scale(1/scaleFactor, 1/scaleFactor);
        currentScale /= scaleFactor;
    }
}

void CustomGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint point = event->pos();
   // emit mouseReleaseEvent(point);
}
