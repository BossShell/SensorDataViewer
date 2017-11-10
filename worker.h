#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QString>
#include <QImage>

#include <oct.h>
#include <octave.h>
#include <parse.h>
#include <toplev.h>


class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(int band);
    //~Worker();

public slots:
    void process();

signals:
    void finished();
    // Might need more

private:
    int bandNumber;
    QString filename;
};

#endif // WORKER_H
