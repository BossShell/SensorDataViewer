#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>
//#include <QtWidgets>
//#include <QWidget>

class CustomGraphicsView : public QGraphicsView
{
public:
    CustomGraphicsView(QWidget* parent = 0);

signals:

public:
    virtual void wheelEvent(QWheelEvent *event);
};

#endif // CUSTOMGRAPHICSVIEW_H
