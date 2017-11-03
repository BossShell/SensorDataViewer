#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qcustomplot.h>
#include <QtGui>

/********************/
/* Octave Libraries */
/********************/
#include <oct.h>
#include <octave.h>
#include <parse.h>
#include <toplev.h>


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
    /***************/
    /* QCustomPlot */
    /***************/

    // Mouse handlers for qCustomPlot
    void mousePress();
    void mouseWheel();
    void graphClicked(QCPAbstractPlottable*,int);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void titleDoubleClick(QMouseEvent* event);

    // Context menu slots for qCustomPlot
    void contextMenuRequest(QPoint pos);
    void contextMenuAddRequested();

    void selectionChanged();
    void removeSelectedGraphs();
    void changeGraphColor();


    /*************************/
    /* QGraphicsView Buttons */
    /*************************/

    void on_loadButton_released();
    void on_resetZoomButton_released();


    /***********************/
    /* QCustomPlot Buttons */
    /***********************/

    void on_saveButton_released();
    void on_addButton_toggled();
    void on_clearButton_released();
    void on_centerGraphButton_released();


    /***************/
    /* Other Slots */
    /***************/

    void bandChange(int newBand);
    void addPoint(QGraphicsSceneMouseEvent *event);

private:
    Ui::MainWindow *ui;
    bool temp;


    /******************/
    /* Octave Members */
    /******************/

    Matrix outputMatrix;
    int matrixRows, matrixCols;
    bool loadOctaveMatrix(QString &fileName);
    bool iterateDirectory(QString &directoryName, QVector<QString> *filenames);
    void initializeGraphicsScene();


    /***********************************/
    /* Private members for QCustomPlot */
    /***********************************/

    void initializeGraph();
    int xMin, xMax, yMin, yMax;
    void recenterGraph();
    void addPlot(QVector<double> pointVector);


    /*********************/
    /* Members for Image */
    /*********************/

    QGraphicsScene *scene;
    QVector<QImage> imageVector;
    //void setImage(const QImage &newImage);
};

#endif // MAINWINDOW_H
