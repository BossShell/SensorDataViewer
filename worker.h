#ifndef WORKER_H
#define WORKER_H

#include <QRunnable>
#include <QString>

#include <oct.h>
#include <octave.h>
#include <parse.h>
#include <toplev.h>


class Worker : public QRunnable
{
public:
    Worker();
    QString filename;

public:
    void run();
};

#endif // WORKER_H
