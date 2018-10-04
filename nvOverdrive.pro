QT       += core gui widgets charts

TARGET = nvOverdrive
TEMPLATE = app

CONFIG += c++14

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        src/main.cpp \
    src/nvidiacontrol.cpp \
    src/gpuchart.cpp \
    src/settings.cpp \
    src/hardwaremonitor.cpp \
    src/panel.cpp

HEADERS += \
    include/nvidiacontrol.h \
    include/gpuchart.h \
    include/settings.h \
    include/hardwaremonitor.h \
    include/panel.h

FORMS += \
    include/ui/hardwaremonitor.ui \
    include/ui/panel.ui

unix {
    LIBS += -lX11
    LIBS += -lXNVCtrl
}
