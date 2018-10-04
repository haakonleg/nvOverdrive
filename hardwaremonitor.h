#ifndef HARDWAREMONITOR_H
#define HARDWAREMONITOR_H

#include <QWidget>
#include <memory>
#include "ui_hardwaremonitor.h"
#include "gpuchart.h"
#include "nvidiacontrol.h"

enum CHARTS {
    GPU_TEMP, CORE_CLOCK,
    MEM_CLOCK, FAN_SPEED
};

namespace Ui {
class HardwareMonitor;
}

class HardwareMonitor : public QWidget {
    Q_OBJECT

public:
    explicit HardwareMonitor(NvidiaControl& nvidia, int gpuId, QWidget *parent = 0);

    NvidiaControl& nvidia;
    QVBoxLayout* chartsLayout;
    QTimer* updater;
    QMap<CHARTS, GPUChart*> charts;

    void addChart(CHARTS chart);
private:
    int gpuId;
    std::unique_ptr<Ui::HardwareMonitor> ui;

    void updateCharts();
};

#endif // HARDWAREMONITOR_H
