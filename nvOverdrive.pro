QT       += core gui widgets charts

TARGET = nvOverdrive
TEMPLATE = app

CONFIG += c++14

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
    nvidiacontrol.cpp \
    gpuchart.cpp \
    settings.cpp \
    hardwaremonitor.cpp \
    panel.cpp

HEADERS += \
    nvidiacontrol.h \
    gpuchart.h \
    settings.h \
    hardwaremonitor.h \
    panel.h

FORMS += \
    hardwaremonitor.ui \
    panel.ui

unix {
    LIBS += -lX11
    LIBS += -lXNVCtrl
}
