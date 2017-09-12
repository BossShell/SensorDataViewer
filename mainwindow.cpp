#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>

#include <QVector>
#include <QString>
#include <QChar>

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include <iostream>
#include <oct.h>
#include <octave.h>
#include <parse.h>
#include <interpreter.h>
#include <str-vec.h>
class string_vector;

#include <sstream>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initializeGraph();

    ui->imageLabel->setBackgroundRole(QPalette::Base);
    ui->imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->imageLabel->setScaledContents(true);

    ui->scrollArea->setBackgroundRole(QPalette::Dark);
    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->setVisible(false);


    // Initialize octave
    string_vector argv (2);
    argv(0) = "embedded";
    argv(1) = "-q";

    //octave_main(2, argv.c_str_vec(), 1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::loadFile(const QString &fileName)
{
    /*QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    if (newImage.isNull()) {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                                             tr("Cannot load %1: %2").arg(QDir::toNativeSeparators(fileName), reader.errorString()));
            return false;
    }

    setImage(newImage);

    setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4").arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
    statusBar()->showMessage(message);*/
    return true;
}

void MainWindow::setImage(const QImage &newImage)
{
    /*image = newImage;
    ui->imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 0.1;

    ui->scrollArea->setVisible(true);
    //fitToWindowAct->setEnabled(true);
    //updateActions();

    //if (!fitToWindowAct->isChecked())
    ui->imageLabel->adjustSize();*/
    /*octave_value_list functionArguments;
    functionArguments(0) = 1;// Filename
    functionArguments(1) = 1;

    octave_value_list result = feval("ReadFile", functionArguments, 1);*/
}

//void MainWindow::zoomIn(){scaleImage(1.25);}
//void MainWindow::zoomOut(){scaleImage(0.8);}

/*void MainWindow::scaleImage(double factor)
{
    Q_ASSERT(ui->imageLabel->pixmap());
    scaleFactor *= factor;
    ui->imageLabel->resize(scaleFactor * ui->imageLabel->pixmap()->size());

    //adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    //adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    //zoomInAct->setEnabled(scaleFactor < 3.0);
    //zoomOutAct->setEnabled(scaleFactor > 0.333);
}*/

void MainWindow::initializeGraph()
{
    recenterGraph();
    ui->customPlot->axisRect()->setupFullAxesBox();
    ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables | QCP::iMultiSelect);

    /*
    // Adds a title to the top of the graph
    ui->customPlot->plotLayout()->insertRow(0);
    QCPTextElement *title = new QCPTextElement(ui->customPlot, "Graph Title", QFont("sans", 10, QFont::Normal));
    ui->customPlot->plotLayout()->addElement(0, 0, title);
    */

    // Sets up the legend
    QFont legendFont = font();
    legendFont.setPointSize(7);
    ui->customPlot->legend->setFont(legendFont);
    legendFont.setPointSize(10);
    ui->customPlot->legend->setSelectedFont(legendFont);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems);

    // Connects slot that ties some axis selections together
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    // Connect slots for holding selected axis stationary
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    // Transfers ranges from bottom and left axes to the top and right axes, respectively
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // Connect slots for some interactions
    //connect(ui->customPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    connect(ui->customPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));
    //connect(title, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(titleDoubleClick(QMouseEvent*)));

    // Shows a message in the status bar when a graph is clicked
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));

    // Sets up policy and connects slot for context menu popup
    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}

/*
// Allows the graph title to be chenged by double clicking it.
//
// If desired, slot must be uncommented in mainwindow.h as well as
// slot connection in MainWindow::initializeGraph()

void MainWindow::titleDoubleClick(QMouseEvent* event)
{
        Q_UNUSED(event)
        if (QCPTextElement *title = qobject_cast<QCPTextElement*>(sender()))
        {
                bool ok;
                QString newTitle = QInputDialog::getText(this, "QCustomPlot example", "New plot title:", QLineEdit::Normal, title->text(), &ok);
                if (ok)
                {
                        title->setText(newTitle);
                        ui->customPlot->replot();
                }
        }
}
*/

void MainWindow::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    Q_UNUSED(legend)
    if (item)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        bool ok;
        QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
        if (ok)
        {
                plItem->plottable()->setName(newName);
                ui->customPlot->replot();
        }
    }
}

void MainWindow::selectionChanged()
{
    // Makes top and bottom axes synchronously selectable and groups axis selectable parts
    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->customPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // Makes left and right axes synchronously selectable and groups axis selectable parts
    if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // Synchronizes graph selection with corresponding legend item
    for (int i=0; i<ui->customPlot->graphCount(); ++i)
    {
        QCPGraph *graph = ui->customPlot->graph(i);
        QCPPlottableLegendItem *item = ui->customPlot->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
        }
    }
}

void MainWindow::mousePress()
{
    // If an axis is selected, only allow the direction of that axis to be dragged
    // If no axis is selected, both directions may be dragged

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
    else
        ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
    // If an axis is selected, only allow the direction of that axis to be zoomed
    // If no axis is selected, both directions may be zoomed

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
    else
        ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
    {
        menu->addAction("New graph", this, SLOT(on_addButton_released()));
        menu->addAction("Remove selected graphs", this, SLOT(removeSelectedGraphs()));
        menu->addAction("Change graph color", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction("Save all graphs", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    } else    // general context menu on graphs requested
    {
        menu->addAction("New graph", this, SLOT(on_addButton_released()));
        if (ui->customPlot->selectedGraphs().size() > 0)
            menu->addAction("Remove selected graphs", this, SLOT(removeSelectedGraphs()));
        if (ui->customPlot->graphCount() > 0)
            menu->addAction("Save all graphs", this, SLOT(removeAllGraphs()));
    }

    menu->popup(ui->customPlot->mapToGlobal(pos));
}

void MainWindow::removeSelectedGraphs()
{
    while (ui->customPlot->selectedGraphs().size() > 0)
    {
        ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
        //ui->customPlot->replot();
    }

    ui->customPlot->replot();
}

void MainWindow::recenterGraph()
{
    // Desired axes ranges for graphs
    xMin = 0;
    xMax = 269;
    yMin = -2;
    yMax = 2;

    ui->customPlot->xAxis->setRange(xMin, xMax);
    ui->customPlot->yAxis->setRange(yMin, yMax);
    ui->customPlot->replot();

    if (ui->customPlot->graphCount() == 0)
        ui->customPlot->legend->setVisible(false);
}

void MainWindow::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    // Since we know we only have QCPGraphs in the plot, we can immediately access interface1D().
    // Usually it's better to first check whether interface1D() returns non-zero, and only then use it.
    double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
    ui->statusBar->showMessage(message, 2500);
}

void MainWindow::addPoint(QVector<double> pointVector)
{
    // Retrieves current number of graphs in ui->customPlot
    int graphCount = ui->customPlot->graphCount();

    ui->customPlot->addGraph();

    QVector<double> band(xMax - xMin + 1);
    for (int i = xMin; i <= xMax; ++i)
        band[i] = i;

    ui->customPlot->graph(graphCount)->setData(band, pointVector);
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->replot();
}

void MainWindow::on_loadButton_released()
{
    /*QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString());

    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    setImage(newImage);
    setWindowFilePath(fileName);

    this->update();*/
}

void MainWindow::on_addButton_released()
{
    QVector<double> pointVector(xMax - xMin + 1);
    int graphCount = ui->customPlot->graphCount();

    if(graphCount == 1)
    {
        for (int i = xMin; i <= xMax; ++i)
            pointVector[i] = qSin(.1*i);
        temp = false;
    } else
    {
        for (int i = xMin; i <= xMax; ++i)
            pointVector[i] = graphCount;
        temp = true;
    }

    addPoint(pointVector);
}

void MainWindow::on_centerGraphButton_released()
{
    recenterGraph();
}

void MainWindow::on_clearButton_released()
{
    ui->customPlot->clearGraphs();
    ui->customPlot->legend->setVisible(false);
    recenterGraph();
}

void MainWindow::on_saveButton_released()
{
    recenterGraph();
    ui->customPlot->saveJpg("I dont care.jpg", 0, 0, 5.0);
}

