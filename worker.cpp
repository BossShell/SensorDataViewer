#include "worker.h"
#include <QThread>
#include <QColor>

Worker::Worker()
{
    this->filename = "shutup";
}

void Worker::run()
{
    octave_value_list input;
    int matrixRows, matrixCols;

    QByteArray ba = this->filename.toLatin1();
    const char *fileNameChar = ba.data();
    input(0) = octave_value(fileNameChar);

    octave_value_list output = feval("readMat", input);

    Matrix outputMatrix = output(0).matrix_value();
    matrixRows = outputMatrix.rows();
    matrixCols = outputMatrix.columns();

    int numberChannels = matrixCols / output(0).columns();

    matrixCols = matrixCols / numberChannels;

    //QImage bandImage(QSize(matrixRows, matrixCols), QImage::Format_RGB888);
    int hValue;
    int sValue = 100;
    int lValue = 100;
    QColor color;

    for(int k = 0; k < numberChannels; k++)
    {
        for(int i = 0; i < matrixRows; i++)
        {
            for(int j = 0; j < matrixCols; j++)
            {
                // Value range: {0, 4096}
                hValue = 360 * ( outputMatrix.elem(i, (matrixCols * k) + j) / 4096);
                color.setHsv(hValue, sValue, lValue);

                //bandImage.setPixelColor(QPoint(i, j), color);
            }
        }
        //imageVector.append(bandImage);
    }
}
