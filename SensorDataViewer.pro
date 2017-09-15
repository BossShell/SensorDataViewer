
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SensorDataViewer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        qcustomplot.cpp

HEADERS += \
        mainwindow.h \
        qcustomplot.h

FORMS += \
        mainwindow.ui

#LIBS += -LC:/Octave/Octave-4.0.2/lib/octave/4.0.2/ -lliboctinterp.dll -lliboctave.dll

INCLUDEPATH += C:/Octave/Octave-4.0.2/include/octave-4.0.2/octave
DEPENDPATH += C:/Octave/Octave-4.0.2/include/octave-4.0.2/octave

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/Octave/Octave-4.0.2/lib/octave/4.0.2/ -lliboctave.dll -lliboctinterp.dll
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/Octave/Octave-4.0.2/lib/octave/4.0.2/ -lliboctave.dll -lliboctinterp.dll

INCLUDEPATH += $$PWD/Octave/Octave-4.0.2/lib/octave/4.0.2
DEPENDPATH += $$PWD/Octave/Octave-4.0.2/lib/octave/4.0.2
