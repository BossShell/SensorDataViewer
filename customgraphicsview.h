#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QtWidgets>
#include <QWidget>

class CustomGraphicsView : public QGraphicsView
{
public:
    CustomGraphicsView(QWidget* parent = 0);

protected:
    virtual void wheelEvent(QWheelEvent *event);
//    virtual void mouseMoveEvent(QMouseEvent *event);
//    virtual void mousePressEvent(QMouseEvent *event);
//    virtual void mouseReleaseEvent(QMouseEvent *event);
};

#endif // CUSTOMGRAPHICSVIEW_H
