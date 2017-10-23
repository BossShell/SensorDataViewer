#ifndef CUSTOMGRAPHICSSCENE_H
#define CUSTOMGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QtWidgets>
#include <QWidget>

class CustomGraphicsScene : public QGraphicsScene
{
public:
    CustomGraphicsScene(QWidget *parent = 0);

signals:
    //void mouseReleaseEvent(QPoint point);

public:
    //virtual void wheelEvent(QWheelEvent *event);
    //virtual void mousePressEvent(QMouseEvent *event);
    //virtual void mouseReleaseEvent(QMouseEvent *event);
    //virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    //virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};


#endif // CUSTOMGRAPHICSSCENE_H
