QT       += core gui widgets charts

TARGET = nvOverdrive
TEMPLATE = app

CONFIG += c++14

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    nvidiacontrol.cpp \
    gpuchart.cpp \
    settings.cpp \
    hardwaremonitor.cpp

HEADERS += \
        mainwindow.h \
    nvidiacontrol.h \
    gpuchart.h \
    settings.h \
    hardwaremonitor.h

FORMS += \
        mainwindow.ui \
    hardwaremonitor.ui

unix {
    LIBS += -lX11
    LIBS += -lXNVCtrl
}
