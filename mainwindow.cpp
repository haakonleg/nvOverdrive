#include "mainwindow.h"

MainWindow::MainWindow(NvidiaControl& nvidia, Settings& settings, QWidget* parent) : QWidget(parent), nvidia(nvidia), settings(settings) {
    ui = std::make_unique<Ui::MainWindow>();
    ui->setupUi(this);

    // Connect UI elements
    connect(ui->sliderCoreClock, &QSlider::valueChanged, this, &MainWindow::sliderValChanged);
    connect(ui->sliderMemClock, &QSlider::valueChanged, this, &MainWindow::sliderValChanged);
    connect(ui->sliderFanSpeed, &QSlider::valueChanged, this, &MainWindow::sliderValChanged);
    connect(ui->btnApply, &QPushButton::clicked, this, &MainWindow::apply);
    connect(ui->radioFanAuto, &QRadioButton::toggled, this, &MainWindow::enableFanControl);
    connect(ui->editCoreClock, &QLineEdit::returnPressed, this, &MainWindow::valueEntered);
    connect(ui->editMemClock, &QLineEdit::returnPressed, this, &MainWindow::valueEntered);
    connect(ui->editFanSpeed, &QLineEdit::returnPressed, this, &MainWindow::valueEntered);
    connect(ui->btnAddProfile, &QPushButton::clicked, this, &MainWindow::newProfile);
    connect(ui->btnDeleteProfile, &QPushButton::clicked, this, &MainWindow::deleteProfile);
    connect(ui->btnSaveProfile, &QPushButton::clicked, this, &MainWindow::saveProfile);
    connect(ui->chkBoxApplyOnStart, &QCheckBox::toggled, this, &MainWindow::applyOnStart);

    loadGpu(0);
}

void MainWindow::loadGpu(int id) {
    selectedGPU = &nvidia.getGpu(id);

    ui->labelGpuName->setText(selectedGPU->productName);
    ui->labelDriverVer->setText(selectedGPU->driverVer);

    // Add min and max clock ranges to sliders
    ClockFreqRanges freqRanges = nvidia.getMinMaxClockFreqs(selectedGPU->id);
    ui->sliderCoreClock->setMaximum(freqRanges.coreMax);
    ui->sliderCoreClock->setMinimum(freqRanges.coreMin);
    ui->sliderMemClock->setMaximum(freqRanges.memMax);
    ui->sliderMemClock->setMinimum(freqRanges.memMin);

    // Add set clock frequencies
    ClockFreqs freqs = nvidia.getClocks(selectedGPU->id);
    ui->sliderCoreClock->setValue(freqs.coreClock);
    ui->sliderMemClock->setValue(freqs.memClock);

    // Add cooler info
    CoolerInfo cooler = nvidia.getCoolerInfo(selectedGPU->id);
    ui->radioFanAuto->setChecked(!cooler.isManual);
    ui->sliderFanSpeed->setValue(cooler.targetLevel);

    // Add GPU profiles
    const auto& profiles = settings.getGPUProfiles(selectedGPU->UUID);
    for (auto it = profiles.cbegin(); it != profiles.cend(); ++it) {
        ui->cmbBoxProfile->addItem(it.key());
    }

    // Add charts
    hwMon = new HardwareMonitor(nvidia, selectedGPU->id, this);
    ui->hwMonLayout->addWidget(hwMon);
    hwMon->addChart(GPU_TEMP);
    hwMon->addChart(CORE_CLOCK);
    hwMon->addChart(MEM_CLOCK);
    hwMon->addChart(FAN_SPEED);
}

void MainWindow::sliderValChanged(int value) {
    QString text = QString::number(value);
    const QSlider* slider = static_cast<QSlider*>(sender());

    if (value > 0 && (slider == ui->sliderCoreClock || slider == ui->sliderMemClock)) {
        text.prepend('+');
    }

    if (slider == ui->sliderCoreClock) {
        ui->editCoreClock->setText(text);
    } else if (slider == ui->sliderMemClock) {
        ui->editMemClock->setText(text);
    } else if (slider == ui->sliderFanSpeed) {
        ui->editFanSpeed->setText(text);
    }
}

void MainWindow::valueEntered() {
    QSlider* slider;
    const QLineEdit* edit = static_cast<QLineEdit*>(sender());

    if (edit == ui->editCoreClock) {
        slider = ui->sliderCoreClock;
    } else if (edit == ui->editMemClock) {
        slider = ui->sliderMemClock;
    } else if (edit == ui->editFanSpeed) {
        slider = ui->sliderFanSpeed;
    }

    bool ok;
    int val = edit->text().toInt(&ok);
    if (!ok || val < slider->minimum() || val > slider->maximum())
        QMessageBox::critical(this, "Error", "Invalid value");
    else
        slider->setValue(val);
}

void MainWindow::apply() {
    int coreClock = ui->sliderCoreClock->value();
    int memClock = ui->sliderMemClock->value();
    int fanSpeed = ui->sliderFanSpeed->value();

    nvidia.setClocks(selectedGPU->id, coreClock, memClock);

    if (!ui->radioFanAuto->isChecked())
        nvidia.setManualFanSpeed(selectedGPU->id, fanSpeed);
    else
        nvidia.setFanSpeedAuto(selectedGPU->id);
}

void MainWindow::enableFanControl(bool enable) {
    ui->sliderFanSpeed->setEnabled(!enable);
    ui->editFanSpeed->setEnabled(!enable);
}

void MainWindow::newProfile() {
    bool ok;
    QString name = QInputDialog::getText(this, "New Profile", "Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty())
        return;

    try {
        settings.newProfile(selectedGPU->UUID, name);
        ui->cmbBoxProfile->addItem(name);
        ui->cmbBoxProfile->setCurrentIndex(ui->cmbBoxProfile->count()-1);
    } catch (SettingsException& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void MainWindow::deleteProfile() {
    int index = ui->cmbBoxProfile->currentIndex();
    QString profileName = ui->cmbBoxProfile->currentText();

    if (QMessageBox::question(this, "Delete profile?",
                              QString("Are you sure you want to delete profile %1?").arg(profileName),
                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        settings.deleteProfile(selectedGPU->UUID, profileName);
        ui->cmbBoxProfile->removeItem(index);
    }
}

void MainWindow::saveProfile() {
    QString profileName = ui->cmbBoxProfile->currentText();

    GPUProfile profile;
    profile.coreClock = ui->sliderCoreClock->value();
    profile.memClock = ui->sliderMemClock->value();
    profile.manualFanControl = !ui->radioFanAuto->isChecked();

    profile.fanSpeed = 0;
    if (profile.manualFanControl)
        profile.fanSpeed = ui->sliderFanSpeed->value();

    settings.editProfile(selectedGPU->UUID, profile, profileName);
}

void MainWindow::applyOnStart() {
    QString profileName = ui->cmbBoxProfile->currentText();
    settings.setApplyOnStart(selectedGPU->UUID, profileName);
}
