#ifndef NVIDIACONTROL_H
#define NVIDIACONTROL_H

#include <QString>
#include <QVector>
#include <X11/Xlib.h>
#include <NVCtrl/NVCtrl.h>
#include <NVCtrl/NVCtrlLib.h>

class NvException : public std::exception {
private:
    QString message;
public:
    NvException(const QString &message) { this->message = "NvidiaControl: " + message; }
    const char* what() const throw() override { return message.toLatin1().data(); }
};

struct GPU {
    int id;
    QString productName;
    QString vBiosVer;
    QString driverVer;
    QString UUID;
};

struct ClockFreqs {
    int coreClock;
    int memClock;
};

struct CoolerInfo {
    bool isManual;
    int targetLevel;
    int currentLevel;
};

struct ClockFreqRanges {
    int coreMax;
    int coreMin;
    int memMax;
    int memMin;
};

class NvidiaControl {
private:
    // These must be static because xlib is a C api
    static QString xLibErr;
    static int xLibErrorHandler(Display* d, XErrorEvent* e);
    static QString getXlibErr();

    QVector<GPU> gpus;
    Display *dpy = nullptr;
    int eventBase, errorBase;

    QString queryStringAttribute(int gpuId, int targetType, unsigned int nvAttribute);
    int queryAttribute(int gpuId, int targetType, unsigned int nvAttribute);
    NVCTRLAttributeValidValuesRec queryValidAttributes(int gpuId, int targetType, unsigned int nvAttribute);
    void setAttribute(int gpuId, int targetType, unsigned int nvAttribute, int value);

public:
    NvidiaControl();
    ~NvidiaControl();

    const QVector<QString> getGpuUUIDs();
    const GPU& getGpu(int gpuId);
    ClockFreqRanges getMinMaxClockFreqs(int gpuId);
    ClockFreqs getClocks(int gpuId);
    void setClocks(int gpuId, int coreClock, int memClock);
    int getCoreTemp(int gpuId);
    ClockFreqs getCurrentClocks(int gpuId);
    CoolerInfo getCoolerInfo(int gpuId);
    void setManualFanSpeed(int gpuId, int speed);
    void setFanSpeedAuto(int gpuId);
};

#endif // NVIDIACONTROL_H
