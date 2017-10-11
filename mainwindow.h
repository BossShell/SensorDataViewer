#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QImage>
#include <QAction>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>

#include <qcustomplot.h>

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QtGui>

class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QLineSeries;
class QChart;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    bool loadFile(const QString &);
    ~MainWindow();

private slots:
    void on_loadButton_released();

    // Slots for QCustomPlot
    void mousePress();
    void mouseWheel();
    void selectionChanged();
    void graphClicked(QCPAbstractPlottable*,int);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    //void titleDoubleClick(QMouseEvent* event);
    void contextMenuRequest(QPoint pos);
    void contextMenuAddRequested();
    void removeSelectedGraphs();
    void changeGraphColor();

    void bandChange(int newBand);
    void addPlot(QVector<double> pointVector);

    void on_saveButton_released();
    void on_addButton_toggled();
    void on_clearButton_released();
    void on_centerGraphButton_released();

private:
    Ui::MainWindow *ui;
    //void createActions();
    void setImage(const QImage &newImage);
    //void scaleImage(double factor);

    bool loadOctaveMatrix(QString &fileName);
    void initializeGraphicsScene();
    // Private members for QCustomPlot
    void initializeGraph();
    int xMin, xMax, yMin, yMax;
    void recenterGraph();

    //QGraphicsItem *item;
    QGraphicsScene *scene;
    QVector<QImage> imageVector;
    void addPoint(QPointF pos);

//    QImage image;
//    double scaleFactor;


    bool temp;
};

#endif // MAINWINDOW_H
