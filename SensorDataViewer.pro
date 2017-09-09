#-------------------------------------------------
#
# Project created by QtCreator 2017-09-09T00:22:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SensorDataViewer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        qcustomplot.cpp

HEADERS += \
        mainwindow.h \
        qcustomplot.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += C:\Octave\Octave-4.2.1\include\octave-4.2.1\octave
DEPENDPATH += C:\Octave\Octave-4.2.1\include\octave-4.2.1\octave

INCLUDEPATH += C:\Octave\Octave-4.2.1\lib\octave\4.2.1
DEPENDPATH += C:\Octave\Octave-4.2.1\lib\octave\4.2.1

LIBS += -LC:\Octave\Octave-4.2.1\lib\octave\4.2.1\ -lliboctinterp.dll -lliboctave.dll
