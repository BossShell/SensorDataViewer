#ifndef CUSTOMGRAPHICSSCENE_H
#define CUSTOMGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QtWidgets>
#include <QWidget>

class CustomGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    CustomGraphicsScene(QWidget *parent = 0);

signals:
    void mousePress(QGraphicsSceneMouseEvent *event);

public:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

#endif // CUSTOMGRAPHICSSCENE_H
