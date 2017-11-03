#include "customgraphicsscene.h"
#include "ui_mainwindow.h"
#include <QtWidgets>

CustomGraphicsScene::CustomGraphicsScene(QWidget *parent) : QGraphicsScene(parent)
{

}


/**********************************************************************************/
/* Method - CustomGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event) */
/*                                                                                */
/* Description - Intercepts mouse events in QGraphicsScene.                       */
/**********************************************************************************/

void CustomGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit CustomGraphicsScene::mousePress(event);
}
