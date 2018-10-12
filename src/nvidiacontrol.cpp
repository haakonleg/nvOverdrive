#include "include/nvidiacontrol.h"

QString NvidiaControl::xLibErr;

int NvidiaControl::xLibErrorHandler(Display* d, XErrorEvent* e) {
    char buffer[BUFSIZ];
    XGetErrorText(d, e->error_code, buffer, BUFSIZ);
    xLibErr = QString::fromUtf8(buffer);
    return 0;
}

QString NvidiaControl::getXlibErr() {
    if (!xLibErr.isNull()) {
        QString err = xLibErr;
        xLibErr = QString();
        return err;
    }
    return xLibErr;
}

NvidiaControl::NvidiaControl() {
    // Open X11 display
    dpy = XOpenDisplay(nullptr);
    if (dpy == nullptr)
        throw NvException("Failed to open X display, check if $DISPLAY is set");

    // Set error handler
    XSetErrorHandler(&xLibErrorHandler);

    // Check if XNVCtrl extension exists
    if (!XNVCTRLQueryExtension(dpy, &eventBase, &errorBase))
        throw NvException("NV-CONTROL X extension does not exist on " + QString(XDisplayName(nullptr)));

    // Get number of GPUs in the system
    int gpuCount = 0;
    if (!XNVCTRLQueryTargetCount(dpy, NV_CTRL_TARGET_TYPE_GPU, &gpuCount))
        throw NvException("Failed to query number of GPUs in the system");

    for (int i = 0; i < gpuCount; i++) {
        // Read gpu
        GPU newGpu;
        newGpu.id = i;
        newGpu.productName = queryStringAttribute(i, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_STRING_PRODUCT_NAME);
        newGpu.vBiosVer = queryStringAttribute(i, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_STRING_VBIOS_VERSION);
        newGpu.driverVer = queryStringAttribute(i, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_STRING_NVIDIA_DRIVER_VERSION);
        newGpu.UUID = queryStringAttribute(i, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_STRING_GPU_UUID);
        gpus.append(newGpu);
    }

    if (gpus.size() == 0)
        throw NvException("No NVIDIA GPUs found");
}

NvidiaControl::~NvidiaControl() {
    if (dpy != nullptr)
        XCloseDisplay(dpy);
}

const QVector<GPU>& NvidiaControl::getGpus() {
    return gpus;
}

const GPU& NvidiaControl::getGpu(int gpuId) {
    return gpus[gpuId];
}

ClockFreqRanges NvidiaControl::getMinMaxClockFreqs(int gpuId) {
    NVCTRLAttributeValidValuesRec core, mem;
    core = queryValidAttributes(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_NVCLOCK_OFFSET_ALL_PERFORMANCE_LEVELS);
    mem = queryValidAttributes(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_NVCLOCK_OFFSET_ALL_PERFORMANCE_LEVELS);

    ClockFreqRanges ranges;
    ranges.coreMax = core.u.range.max;
    ranges.coreMin = core.u.range.min;
    ranges.memMax = mem.u.range.max;
    ranges.memMin = mem.u.range.min;
    return ranges;
}

ClockFreqs NvidiaControl::getClocks(int gpuId) {
    ClockFreqs freqs;
    freqs.coreClock = queryAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_NVCLOCK_OFFSET_ALL_PERFORMANCE_LEVELS);
    freqs.memClock = queryAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_MEM_TRANSFER_RATE_OFFSET_ALL_PERFORMANCE_LEVELS);
    return freqs;
}

void NvidiaControl::setClocks(int gpuId, int coreClock, int memClock) {
    setAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_NVCLOCK_OFFSET_ALL_PERFORMANCE_LEVELS, coreClock);
    setAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_MEM_TRANSFER_RATE_OFFSET_ALL_PERFORMANCE_LEVELS, memClock);
}

int NvidiaControl::getCoreTemp(int gpuId) {
    return queryAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_CORE_TEMPERATURE);
}

ClockFreqs NvidiaControl::getCurrentClocks(int gpuId) {
    int packedClocks = queryAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_CURRENT_CLOCK_FREQS);

    ClockFreqs freqs;
    freqs.coreClock = (packedClocks >> 16) & 0xFFFF;
    freqs.memClock = packedClocks & 0xFFFF;
    return freqs;
}

CoolerInfo NvidiaControl::getCoolerInfo(int gpuId) {
    CoolerInfo info;
    info.isManual = queryAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_COOLER_MANUAL_CONTROL);
    info.targetLevel = queryAttribute(gpuId, NV_CTRL_TARGET_TYPE_COOLER, NV_CTRL_THERMAL_COOLER_LEVEL);
    info.currentLevel = queryAttribute(gpuId, NV_CTRL_TARGET_TYPE_COOLER, NV_CTRL_THERMAL_COOLER_CURRENT_LEVEL);
    return info;
}

void NvidiaControl::setManualFanSpeed(int gpuId, int speed) {
    // Set manual control if needed
    if (queryAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_COOLER_MANUAL_CONTROL) == NV_CTRL_GPU_COOLER_MANUAL_CONTROL_FALSE) {
        setAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_COOLER_MANUAL_CONTROL, NV_CTRL_GPU_COOLER_MANUAL_CONTROL_TRUE);
    }
    setAttribute(gpuId, NV_CTRL_TARGET_TYPE_COOLER, NV_CTRL_THERMAL_COOLER_LEVEL, speed);
}

void NvidiaControl::setFanSpeedAuto(int gpuId) {
    setAttribute(gpuId, NV_CTRL_TARGET_TYPE_GPU, NV_CTRL_GPU_COOLER_MANUAL_CONTROL, NV_CTRL_GPU_COOLER_MANUAL_CONTROL_FALSE);
}

QString NvidiaControl::queryStringAttribute(int gpuID, int targetType, unsigned int nvAttribute) {
    char* str;
    bool ok = XNVCTRLQueryTargetStringAttribute(dpy, targetType, gpuID, 0, nvAttribute, &str);
    QString xlib = getXlibErr();
    if (!xlib.isNull() || !ok) {
        XFree(str);
        throw NvException(QString("queryStringAttribute %1 xlib: %2").arg(nvAttribute).arg(xlib));
    }
    QString returnStr = QString::fromUtf8(str);
    XFree(str);
    return returnStr;
}

int NvidiaControl::queryAttribute(int gpuID, int targetType, unsigned int nvAttribute) {
    int res;
    bool ok = XNVCTRLQueryTargetAttribute(dpy, targetType, gpuID, 0, nvAttribute, &res);
    QString xlib = getXlibErr();
    if (!xlib.isNull() || !ok) {
        throw NvException(QString("queryAttribute %1 xlib: %2").arg(nvAttribute).arg(xlib));
    }
    return res;
}

NVCTRLAttributeValidValuesRec NvidiaControl::queryValidAttributes(int gpuID, int targetType, unsigned int nvAttribute) {
    NVCTRLAttributeValidValuesRec validAttrs;
    bool ok = XNVCTRLQueryValidTargetAttributeValues(dpy, targetType, gpuID, 0, nvAttribute, &validAttrs);
    QString xlib = getXlibErr();
    if (!xlib.isNull() || !ok) {
        throw NvException(QString("queryValidAttributes %1 xlib: %2").arg(nvAttribute).arg(xlib));
    }
    return validAttrs;
}

void NvidiaControl::setAttribute(int gpuID, int targetType, unsigned int nvAttribute, int value) {
    bool ok = XNVCTRLSetTargetAttributeAndGetStatus(dpy, targetType, gpuID, 0, nvAttribute, value);
    QString xlib = getXlibErr();
    if (!xlib.isNull() || !ok) {
        throw NvException(QString("setAttribute %1 %2 xlib: %3").arg(nvAttribute).arg(value).arg(xlib));
    }
}
