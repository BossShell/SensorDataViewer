#include "customgraphicsscene.h"
#include "ui_mainwindow.h"
#include <QtWidgets>

CustomGraphicsScene::CustomGraphicsScene(QWidget *parent) : QGraphicsScene(parent)
{

}

void CustomGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit CustomGraphicsScene::mousePress(event);
}
