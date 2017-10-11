#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>

#include <QVector>
#include <QString>
#include <QChar>
#include <QMatrix>

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include <oct.h>
#include <octave.h>
#include <parse.h>
#include <toplev.h>

#include <sstream>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initializeGraph();

    ui->hyperspecView->setDragMode(QGraphicsView::ScrollHandDrag);
    //ui->hyperspecView->setCursor(Qt::ArrowCursor);

    connect(ui->verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(bandChange(int)));

    // Initialize octave
    string_vector argv(2);
    argv(0) = "embedded";
    argv(1) = "-q";

    octave_main(2, argv.c_str_vec(), 1);
    qDebug() << "Octave initialized.";
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::loadOctaveMatrix(QString &fileName)
{
    // Octave function to read hyperspectral:
    // function [x] = ReadFile(fileName, scale)

    // ReadFile loads a hyperspectral cube produced by Spectronon Pro software
    // and saves it in x. The function reads up to 40 bands at a time due to a
    // memory overflow crash. It outputs the time required to read each time it
    // gets 40 more bands.
    //
    // Arguments:
    //    fileName: A string containing the name of the file without extensions.
    //    scale:  A boolean value, where 1 indicates that the data should be
    //            converted to floating point values and scaled to 1 by dividing
    //            by the value indicated in the header file.
    // --------------------------------------------------------------------------

#define OCTAVE_DEBUG

    octave_value_list input;

    QByteArray ba = fileName.toLatin1();
    const char *fileNameChar = ba.data();
    input(0) = octave_value(fileNameChar);

    octave_value_list output = feval("readMat", input);

#ifdef OCTAVE_DEBUG
    qDebug() << "\nOctave Debug:\n";
    qDebug() << output.length() << "item(s) in output.";

    for(int i = 0; i < output.length(); i++)
    {
        if(output(i).is_matrix_type())
        {
            qDebug() << "Output:" << i
                     << " Type:" << QString::fromStdString(output(0).type_name())
                     << " Rows:" << output(i).rows()
                     << " Columns:" << output(i).columns();
        } else
        {
            qDebug() << "Output:" << i
                     << " Type:" << QString::fromStdString(output(0).type_name())
                     << " Length:" << output(i).length();
        }
    }
#endif

    Matrix outputMatrix = output(0).matrix_value();
    int matrixRows = outputMatrix.rows();
    int matrixCols = outputMatrix.columns();

    int numberChannels = matrixCols / output(0).columns();

    matrixCols = matrixCols / numberChannels;

    QImage bandImage(QSize(matrixCols, matrixRows), QImage::Format_Grayscale8);
    int temp;

    for(int k = 0; k < numberChannels; k++)
    {
        for(int i = 0; i < matrixRows; i++)
        {
            for(int j = 0; j < matrixCols; j++)
            {
                temp = 256 - outputMatrix.elem(i, (matrixCols * k) + j) / 2;
                if(temp < 0) temp = 0;
                bandImage.setPixelColor(j, i, qGray(temp, temp, temp));
            }
        }
        imageVector.append(bandImage);
    }

    ui->verticalSlider->setMinimum(0);
    ui->verticalSlider->setMaximum(numberChannels - 1);

#ifdef OCTAVE_DEBUG
    qDebug() << "\nimageVector:"
             << " Size:" << imageVector[0].size()
             << " Number of channels:" << imageVector.length();
#endif

    if(imageVector.length()) return true;
    else return false;
}

void MainWindow::initializeGraphicsScene()
{
    scene = new QGraphicsScene(this);

    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(imageVector[ui->verticalSlider->value()]));
    scene = new QGraphicsScene(this);
    scene->addItem(item);

    ui->hyperspecView->setScene(scene);
    ui->hyperspecView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    connect(ui->hyperspecView, SIGNAL(mouseReleaseEvent(QPointF)), this, SLOT(addPoint(QPointF)));
}

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
        bool okay;
        QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &okay);
        if (okay)
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

    if (ui->customPlot->legend->selectTest(pos, false) >= 0) // Legend
    {
        menu->addAction("New graph", this, SLOT(on_addButton_released()));

        if (ui->customPlot->selectedGraphs().size() > 1)
        {
            menu->addAction("Edit color of selected graphs", this, SLOT(changeGraphColor()));
            menu->addAction("Remove selected graphs", this, SLOT(removeSelectedGraphs()));
        }
        else if (ui->customPlot->selectedGraphs().size() == 1)
        {
            menu->addAction("Edit color of selected graph", this, SLOT(changeGraphColor()));
            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraphs()));
        }

        menu->addAction("Save graph window", this, SLOT(on_saveButton_released()));
    }
    else    // Not legend
    {
        menu->addAction("New graph", this, SLOT(contextMenuAddRequested()));

        if (ui->customPlot->selectedGraphs().size() > 1)
        {
            menu->addAction("Edit color of selected graphs", this, SLOT(changeGraphColor()));
            menu->addAction("Remove selected graphs", this, SLOT(removeSelectedGraphs()));
        }
        else if (ui->customPlot->selectedGraphs().size() == 1)
        {
            menu->addAction("Edit color of selected graph", this, SLOT(changeGraphColor()));
            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraphs()));
        }

        if (ui->customPlot->graphCount() > 0)
            menu->addAction("Save graph window", this, SLOT(on_saveButton_released()));
    }

    menu->popup(ui->customPlot->mapToGlobal(pos));
}

void MainWindow::removeSelectedGraphs()
{
    while (ui->customPlot->selectedGraphs().size() > 0)
    {
        ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
    }

    ui->customPlot->replot();
}

void MainWindow::changeGraphColor()
{
    for(int i = 0; i < ui->customPlot->selectedGraphs().size(); ++i)
    {
        //qDebug() << "changeGraphColor()" << i;
        QColor color = QColorDialog::getColor(Qt::black, this, "Graph Color",  QColorDialog::DontUseNativeDialog);
        ui->customPlot->selectedGraphs().at(i)->setPen(QPen(color));
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

void MainWindow::addPlot(QVector<double> pointVector)
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
void MainWindow::addPoint(QPointF pos)
{
    QPoint point = pos.toPoint();

    qDebug() << point.x() << point.y();
}

void MainWindow::bandChange(int newBand)
{
    scene->clear();
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(imageVector[newBand]));
    //scene->addItem(new QGraphicsPixmapItem(QPixmap::fromImage(imageVector[newBand])));
    scene->addItem(item);
    ui->hyperspecView->update();
}

void MainWindow::on_loadButton_released()
{
    bool okay;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString());

    okay = loadOctaveMatrix(fileName);

    if (okay) initializeGraphicsScene();
}

void MainWindow::on_addButton_toggled()
{
    bool checked = ui->addButton->isChecked();

    if (checked)
    {
        ui->hyperspecView->setDragMode(QGraphicsView::NoDrag);
        //connect(scene, )
    }
    else
    {
        ui->hyperspecView->setDragMode(QGraphicsView::ScrollHandDrag);
        //disconnect()
    }

    QVector<double> pointVector(xMax - xMin + 1);
    //addPoint(pointVector);
}

void MainWindow::contextMenuAddRequested()
{
    ui->addButton->setChecked(true);
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
    QFileDialog saveImageFileDialog;
    QString fileName = QDateTime::currentDateTime().toString("MMddyyyy_hhmmss")+".jpg";

    saveImageFileDialog.setNameFilter("Images (*.jpg)");
    saveImageFileDialog.setDirectory(QDir::currentPath());
    saveImageFileDialog.selectFile(fileName);

    if (saveImageFileDialog.exec())
    {
            fileName = fileName + ".jpg";
            ui->customPlot->saveJpg(fileName, 0, 0, 5.0);
    }
}

