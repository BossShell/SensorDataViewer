#include "customgraphicsview.h"
#include "ui_mainwindow.h"
#include <QtWidgets>

CustomGraphicsView::CustomGraphicsView(QWidget *parent) : QGraphicsView(parent)
{

}

/********************************************************************************/
/* Method - CustomGraphicsView::wheelEvent(QWheelEvent *zoomEvent)              */
/*                                                                              */
/* Description - Intercepts mouse wheel events in QGraphicsView and scales      */
/* image accordingly.                                                           */
/********************************************************************************/

void CustomGraphicsView::wheelEvent(QWheelEvent *zoomEvent)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    static const double scaleFactor = 1.15;

    // Stores the current scale value.
    static double currentScale = 1.0;

    // Defines the minimum scale limit.
    static const double scaleMin = 1.0;

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
