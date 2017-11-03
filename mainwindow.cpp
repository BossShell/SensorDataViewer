#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "customgraphicsscene.h"

// Displays debug information if defined
#define OCTAVE_DEBUG
#define DIRECTORY_DEBUG

// Colormap mode
#define HSV_COLORMAP
//#define RB_COLORMAP


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Assigns stylesheet to the GUI
    QFile styleFile("style.qss");
    styleFile.open(QFile::ReadOnly);
    QString style(styleFile.readAll());
    this->setStyleSheet(style);

    initializeGraph();

    // Disable some of the user interactions while there is no image loaded
    ui->verticalSlider->setEnabled(false);
    ui->resetZoomButton->setEnabled(false);
    ui->addButton->setEnabled(false);

    connect(ui->verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(bandChange(int)));

    // Initialize octave
    string_vector argv(2);
    argv(0) = "embedded";
    argv(1) = "-q";

    octave_main(2, argv.c_str_vec(), 1);

    #ifdef OCTAVE_DEBUG
        qDebug() << "Octave initialized.";
    #endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

/********************************************************************************/
/* MainWindow::initializeGraphicsScene()                                        */
/*                                                                              */
/* Description - Initializes Graphics container which displays the hyperspec    */
/*               image                                                          */
/********************************************************************************/

void MainWindow::initializeGraphicsScene()
{
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(imageVector[ui->verticalSlider->value()]));
    scene = new CustomGraphicsScene(ui->hyperspecView);
    scene->addItem(item);

    ui->hyperspecView->setScene(scene);
    ui->hyperspecView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    ui->verticalSlider->setEnabled(true);
    ui->resetZoomButton->setEnabled(true);
    ui->addButton->setEnabled(true);
    ui->hyperspecView->setDragMode(QGraphicsView::ScrollHandDrag);
    xMax = imageVector.length() - 1;
    recenterGraph();
}


/********************************************************************************/
/* MainWindow::initializeGraph()                                                */
/*                                                                              */
/* Description - Initializes QCustomPlot widget which displays the spectral     */
/*               signature for selected points                                  */
/********************************************************************************/

void MainWindow::initializeGraph()
{
    // Desired axes ranges for graphs
    // Note: xMax is modified once an image is loaded.
    xMin = 0;
    xMax = 270;
    yMin = 0;
    yMax = 4096;

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

    // Connect slots for title double click if desired
    //connect(title, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(titleDoubleClick(QMouseEvent*)));

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

    // Connect slots for axis and legend double clicks
    //connect(ui->customPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    connect(ui->customPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));   

    // Shows a message in the status bar when a graph is clicked
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));

    // Sets up policy and connects slot for context menu popup
    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    // Bottom xAxis Styling
    ui->customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    ui->customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    ui->customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    ui->customPlot->xAxis->setTickLabelColor(Qt::white);
    ui->customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    ui->customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    ui->customPlot->xAxis->grid()->setSubGridVisible(true);
    ui->customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    ui->customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);

    // Left yAxis Styling
    ui->customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    ui->customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    ui->customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
    ui->customPlot->yAxis->setTickLabelColor(Qt::white);
    ui->customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    ui->customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    ui->customPlot->yAxis->grid()->setSubGridVisible(true);
    ui->customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
    ui->customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);

    // Top xAxis Styling
    ui->customPlot->xAxis2->setBasePen(QPen(Qt::white, 1));
    ui->customPlot->xAxis2->setTickPen(QPen(Qt::white, 1));
    ui->customPlot->xAxis2->setSubTickPen(QPen(Qt::white, 1));
    ui->customPlot->xAxis2->setTickLabelColor(Qt::white);
    ui->customPlot->xAxis2->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    ui->customPlot->xAxis2->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    ui->customPlot->xAxis2->grid()->setSubGridVisible(true);
    ui->customPlot->xAxis2->grid()->setZeroLinePen(Qt::NoPen);

    // Right yAxis Styling
    ui->customPlot->yAxis2->setBasePen(QPen(Qt::white, 1));
    ui->customPlot->yAxis2->setTickPen(QPen(Qt::white, 1));
    ui->customPlot->yAxis2->setSubTickPen(QPen(Qt::white, 1));
    ui->customPlot->yAxis2->setTickLabelColor(Qt::white);
    ui->customPlot->yAxis2->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    ui->customPlot->yAxis2->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    ui->customPlot->yAxis2->grid()->setSubGridVisible(true);
    ui->customPlot->yAxis2->grid()->setZeroLinePen(Qt::NoPen);

    ui->customPlot->setBackground(QColor(0x3a, 0x3a, 0x3a));

    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(0x80, 0x80, 0x80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));
    ui->customPlot->axisRect()->setBackground(axisRectGradient);
}


/********************************************************************************/
/* MainWindow::loadOctaveMatrix(QString &fileName)                              */
/*                                                                              */
/* Description - Loads a .mat file using octave. Was used to import 3D image,   */
/*               might be replaced for implementation of all bands.             */
/********************************************************************************/

bool MainWindow::loadOctaveMatrix(QString &fileName)
{
    imageVector.clear();
    outputMatrix.clear();

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


    outputMatrix = output(0).matrix_value();
    matrixRows = outputMatrix.rows();
    matrixCols = outputMatrix.columns();

    int numberChannels = matrixCols / output(0).columns();

    matrixCols = matrixCols / numberChannels;

    QImage bandImage(QSize(matrixRows, matrixCols), QImage::Format_RGB888);
    int hValue, sValue, lValue;
    QColor color;


    #ifdef RB_COLORMAP

        #ifdef OCTAVE_DEBUG
            qDebug() << "\n\nBlue<->Red Colormap Debug:\n";
        #endif

        for(int k = 0; k < numberChannels; k++)
        {
            #ifdef OCTAVE_DEBUG
                QTime time;
                time.restart();
            #endif

            for(int i = 0; i < matrixRows; i++)
            {
                for(int j = 0; j < matrixCols; j++)
                {
                    // {0-4096}
                    if (outputMatrix.elem(i, (matrixCols * k) + j) > 2048)
                        outputMatrix.elem(i, (matrixCols * k) + j) = 2048;

                    if (outputMatrix.elem(i, (matrixCols * k) + j) < 1028) {
                        hValue = 240;
                        sValue = 100 * ( outputMatrix.elem(i, (matrixCols * k) + j) / 1028);
                        lValue = 100 - 50 * ( outputMatrix.elem(i, (matrixCols * k) + j) / 1028);
                    }
                    else {
                        hValue = 0;
                        sValue = 100 * ( outputMatrix.elem(i, (matrixCols * k) + j) - 1028) / 1028;
                        lValue = 100 - 50 * ( outputMatrix.elem(i, (matrixCols * k) + j) - 1028) / 1028;
                    }
                    color.setHsl(hValue, sValue, lValue);

                    bandImage.setPixelColor(QPoint(i, j), color);
                }
            }
            imageVector.append(bandImage);

            #ifdef OCTAVE_DEBUG
                qDebug() << "Band" << k
                         << "time:" << time.elapsed() << "ms";
            #endif
        }

    #endif

    #ifdef HSV_COLORMAP
        sValue =  100;
        lValue = 100;

        #ifdef OCTAVE_DEBUG
            qDebug() << "\nHSV Colormap Debug:\n";
        #endif

        for(int k = 0; k < numberChannels; k++)
        {
            #ifdef OCTAVE_DEBUG
                QTime time;
                time.restart();
            #endif

            for(int i = 0; i < matrixRows; i++)
            {
                for(int j = 0; j < matrixCols; j++)
                {
                    // Value range: {0, 4096}
                    hValue = 360 * ( outputMatrix.elem(i, (matrixCols * k) + j) / 4096);
                    color.setHsv(hValue, sValue, lValue);

                    bandImage.setPixelColor(QPoint(i, j), color);
                }
            }
            imageVector.append(bandImage);

            #ifdef OCTAVE_DEBUG
                qDebug() << "Band" << k
                         << "time:" << time.elapsed() << "ms";
            #endif
        }

    #endif

    ui->verticalSlider->setMinimum(0);
    ui->verticalSlider->setMaximum(numberChannels - 1);

    #ifdef OCTAVE_DEBUG
        qDebug() << "\nMainWindow::imageVector:"
                 << " Size:" << imageVector[0].size()
                 << " Number of channels:" << imageVector.length();
    #endif

    if(imageVector.length())
        return true;
    else
        return false;
}

void MainWindow::loadOctaveDirectory(QVector<QString> *filePaths)
{
    for (int i = 0; i < filePaths->length(); i++)
    {
        QString filePath = filePaths[i];
        QString filename = filename.right(filename.length() - (filename.lastIndexOf("/") + 1));
        filename = filename.left(filename.indexOf("."));
        int bandNumber = filename.right(3).toInt();

        #ifdef DIRECTORY_DEBUG
            qDebug() << "File Path:" << filePath
                     << " Filename:" << filename
                     << " Band Number:" << bandNumber;
        #endif
    }
}



/********************************************************************************/
/* MainWindow::titleDoubleClick(QMouseEvent* event)                             */
/*                                                                              */
/* Description - Allows the graph title to be changed by double clicking it.    */
/*             - If desired, title initialization and slot connection in        */
/*               MainWindow::initializeGraph() located in this file             */
/********************************************************************************/

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


/********************************************************************************/
/* MainWindow::legendDoubleClick(QCPLegend*, QCPAbstractLegendItem*)            */
/*                                                                              */
/* Description - Allows items in the legend to be double clicked in order to    */
/*               change their name.                                             */
/********************************************************************************/

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


/********************************************************************************/
/* MainWindow::selectionChanged()                                               */
/*                                                                              */
/* Description - Makes axes synchronously selectable and groups axis parts      */
/*             - Synchronizes graph selection with corresponding legend item    */
/********************************************************************************/

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


/********************************************************************************/
/* MainWindow::mousePress()                                                     */
/*                                                                              */
/* Description - Loads a .mat file using octave. Was used to import 3D image,   */
/*               might be replaced.                                             */
/********************************************************************************/

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


/********************************************************************************/
/* MainWindow::mouseWheel()                                                     */
/*                                                                              */
/* Description - Loads a .mat file using octave. Was used to import 3D image,   */
/*               might be replaced.                                             */
/********************************************************************************/

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


/********************************************************************************/
/* MainWindow::contextMenuRequest(QPoint pos)                                   */
/*                                                                              */
/* Description - Handles and populates context menus for qCustomPlot.           */
/********************************************************************************/

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


/********************************************************************************/
/* MainWindow::removeSelectedGraphs()                                           */
/*                                                                              */
/* Description - Removes the selected plots from qCustomPlot                    */
/********************************************************************************/

void MainWindow::removeSelectedGraphs()
{
    while (ui->customPlot->selectedGraphs().size() > 0)
    {
        ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
    }

    ui->customPlot->replot();
}


/********************************************************************************/
/* MainWindow::changeGraphColor()                                               */
/*                                                                              */
/* Description - Changes the color of the selected plots from qCustomPlot       */
/********************************************************************************/

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


/********************************************************************************/
/* MainWindow::recenterGraph()                                                  */
/*                                                                              */
/* Description - Recenters and realigns qCustomPlot.                            */
/********************************************************************************/

void MainWindow::recenterGraph()
{
    ui->customPlot->xAxis->setRange(xMin, xMax);
    ui->customPlot->yAxis->setRange(yMin, yMax);
    ui->customPlot->replot();

    if (ui->customPlot->graphCount() == 0)
        ui->customPlot->legend->setVisible(false);
}


/********************************************************************************/
/* MainWindow::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)     */
/*                                                                              */
/* Description - Handles the event in which a plot is clicked.                  */
/********************************************************************************/

void MainWindow::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    // Since we know we only have QCPGraphs in the plot, we can immediately access interface1D().
    // Usually it's better to first check whether interface1D() returns non-zero, and only then use it.
    double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
    ui->statusBar->showMessage(message, 2500);
}


/********************************************************************************/
/* MainWindow::addPlot(QVector<double> pointVector)                             */
/*                                                                              */
/* Description - Adds a plot (passed as QVector<double> argument to             */
/*               qCustomPlot.                                                   */
/********************************************************************************/

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


/********************************************************************************/
/* MainWindow::addPoint(QGraphicsSceneMouseEvent *event)                        */
/*                                                                              */
/* Description - Adds spectral signature for pixel in image when clicked.       */
/********************************************************************************/

void MainWindow::addPoint(QGraphicsSceneMouseEvent *event)
{ 
    int x = nearbyint(event->scenePos().x());
    int y = nearbyint(event->scenePos().y());
    QPoint point(x, y);

    QVector<double> pointVector;

    for(int i = 0; i < imageVector.length(); i++)
    {
        pointVector.append(outputMatrix.elem(x, (matrixCols * i) + y));
    }

    qDebug() << "\naddPoint Debug:"
             << "\n(x , y):" << point.x()<< "," << point.y()
             << "pointVector:" << pointVector;

    addPlot(pointVector);
}


/********************************************************************************/
/* MainWindow::bandChange(int newBand)                                          */
/*                                                                              */
/* Description - Changes displayed band when bandSelector scrollBar is          */
/*               adjusted.                                                      */
/********************************************************************************/

void MainWindow::bandChange(int newBand)
{
    if(imageVector.length() > 1)
    {
        // FIXME: Add check for when no image is loaded
        scene->clear();
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(imageVector[newBand]));
        scene->addItem(item);
        ui->hyperspecView->update();
    }
}


/********************************************************************************/
/* MainWindow::on_loadButton_released()                                         */
/*                                                                              */
/* Description - When the load button is released, opens file dialogue and      */
/*               calls methods necessary to map image to container.             */
/********************************************************************************/

void MainWindow::on_loadButton_released()
{
    bool okay;
    QString directoryName = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QString());

    QVector<QString> filePaths(270);
    iterateDirectory(directoryName, filePaths);

    if (okay) initializeGraphicsScene();
}


/********************************************************************************/
/* MainWindow::iterateDirectory(QString &directoryName)                         */
/*                                                                              */
/* Description - Iterates the selected directory.                               */
/********************************************************************************/

void MainWindow::iterateDirectory(QString &directoryName, QVector<QString> *filenames)
{
    #ifdef DIRECTORY_DEBUG
        qDebug() << "\nDirectory Iterator Debug:\n";
        qDebug() << directoryName;
    #endif

    QDirIterator directory(directoryName, QDirIterator::NoIteratorFlags);

    QTime time;
    time.restart();

    while (directory.hasNext())
    {
        QString nextEntry = directory.next();
        if (nextEntry.right(4) == ".mat")
        {
            filenames->append(nextEntry);
        }
    }
}


/********************************************************************************/
/* MainWindow::on_addButton_toggled()                                           */
/*                                                                              */
/* Description - Changes between panning mode and selecting mode for image when */
/*               the add button is toggled.                                     */
/********************************************************************************/

void MainWindow::on_addButton_toggled()
{
    bool checked = ui->addButton->isChecked();

    if (checked)
    {
        ui->hyperspecView->setDragMode(QGraphicsView::NoDrag);
        connect(scene, SIGNAL(mousePress(QGraphicsSceneMouseEvent*)),
                this, SLOT(addPoint(QGraphicsSceneMouseEvent*)));
    }
    else
    {
        ui->hyperspecView->setDragMode(QGraphicsView::ScrollHandDrag);
        disconnect(scene, SIGNAL(mousePress(QGraphicsSceneMouseEvent*)),
                this, SLOT(addPoint(QGraphicsSceneMouseEvent*)));
    }
}


/********************************************************************************/
/* MainWindow::contextMenuAddRequested()                                        */
/*                                                                              */
/* Description - Toggles add button when "Add Point" is selected from the       */
/*               context menu.                                                  */
/********************************************************************************/

void MainWindow::contextMenuAddRequested()
{
    ui->addButton->setChecked(true);
}


/********************************************************************************/
/* MainWindow::on_centerGraphButton_released()                                  */
/*                                                                              */
/* Description - Recenters and realigns qCustomPlot.                            */
/********************************************************************************/

void MainWindow::on_centerGraphButton_released()
{
    recenterGraph();
}


/********************************************************************************/
/* MainWindow::on_clearButton_released()                                        */
/*                                                                              */
/* Description - Clears all plots from qCustomPlot as well as recenters and     */
/*               realigns qCustomPlot                                           */
/********************************************************************************/

void MainWindow::on_clearButton_released()
{
    ui->customPlot->clearGraphs();
    ui->customPlot->legend->setVisible(false);
    ui->addButton->setChecked(false);
    recenterGraph();
}


/********************************************************************************/
/* MainWindow::on_saveButton_released()                                         */
/*                                                                              */
/* Description - Saves qCustomPlot window to a .jpg file.                       */
/********************************************************************************/

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


/********************************************************************************/
/* MainWindow::on_resetZoomButton_released()                                    */
/*                                                                              */
/* Description - Resets the zoom of the image window while keeping the aspect   */
/*               ratio.                                                         */
/********************************************************************************/

void MainWindow::on_resetZoomButton_released()
{
    ui->hyperspecView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}
