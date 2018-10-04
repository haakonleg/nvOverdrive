#include "include/hardwaremonitor.h"

HardwareMonitor::HardwareMonitor(NvidiaControl& nvidia, int gpuId, QWidget *parent) : QWidget(parent), nvidia(nvidia), gpuId(gpuId) {
    ui = std::make_unique<Ui::HardwareMonitor>();
    ui->setupUi(this);

    chartsLayout = new QVBoxLayout(ui->scrollAreaWidget);
    chartsLayout->setSpacing(0);
    chartsLayout->setMargin(0);
    chartsLayout->setContentsMargins(0,0,0,0);

    updater = new QTimer(this);
    connect(updater, &QTimer::timeout, this, &HardwareMonitor::updateCharts);
}

void HardwareMonitor::addChart(CHARTS chart) {
    // Check if chart already added
    if (charts.contains(chart))
        return;

    switch (chart) {
    case GPU_TEMP:
        charts[GPU_TEMP] = new GPUChart(u8"GPU Temperature (\u2103)", 100, this);
        break;
    case CORE_CLOCK:
        charts[CORE_CLOCK] = new GPUChart("Core Clock (MHz)", 3000, this);
        break;
    case MEM_CLOCK:
        charts[MEM_CLOCK] = new GPUChart("Memory Clock (MHz)", 6000, this);
        break;
    case FAN_SPEED:
        charts[FAN_SPEED] = new GPUChart("Fan Speed (%)", 100, this);
        break;
    default:
        return;
    }

    chartsLayout->addWidget(charts[chart]);

    if (charts.size() == 1)
        updater->start(1000);

    ui->scrollAreaWidget->setMinimumHeight(charts[chart]->maximumHeight() * charts.size());
}

void HardwareMonitor::updateCharts() {
    ClockFreqs freqs = nvidia.getCurrentClocks(gpuId);
    CoolerInfo cooler = nvidia.getCoolerInfo(gpuId);
    int coreTemp = nvidia.getCoreTemp(gpuId);

    // Update charts
    for (auto it = charts.cbegin(); it != charts.cend(); ++it) {
        switch (it.key()) {
        case GPU_TEMP:
            charts[GPU_TEMP]->addValue(coreTemp);
            break;
        case CORE_CLOCK:
            charts[CORE_CLOCK]->addValue(freqs.coreClock);
            break;
        case MEM_CLOCK:
            charts[MEM_CLOCK]->addValue(freqs.memClock);
            break;
        case FAN_SPEED:
            charts[FAN_SPEED]->addValue(cooler.currentLevel);
            break;
        }
    }
}
