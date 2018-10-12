#include "include/panel.h"

#define SB_TEMP_MSG 2000

Panel::Panel(NvidiaControl& nvidia, Settings& settings, QWidget* parent) : QMainWindow(parent), nvidia(nvidia), settings(settings) {
    ui = std::make_unique<Ui::Panel>();
    ui->setupUi(this);

    // Connect UI elements
    connect(ui->sliderCoreClock, &QSlider::valueChanged, this, &Panel::sliderValChanged);
    connect(ui->sliderMemClock, &QSlider::valueChanged, this, &Panel::sliderValChanged);
    connect(ui->sliderFanSpeed, &QSlider::valueChanged, this, &Panel::sliderValChanged);
    connect(ui->btnApply, &QPushButton::clicked, this, &Panel::apply);
    connect(ui->radioFanAuto, &QRadioButton::toggled, this, &Panel::enableFanControl);
    connect(ui->editCoreClock, &QLineEdit::returnPressed, this, &Panel::valueEntered);
    connect(ui->editMemClock, &QLineEdit::returnPressed, this, &Panel::valueEntered);
    connect(ui->editFanSpeed, &QLineEdit::returnPressed, this, &Panel::valueEntered);
    connect(ui->btnAddProfile, &QPushButton::clicked, this, &Panel::newProfile);
    connect(ui->btnDeleteProfile, &QPushButton::clicked, this, &Panel::deleteProfile);
    connect(ui->btnSaveProfile, &QPushButton::clicked, this, &Panel::saveProfile);
    connect(ui->chkBoxApplyOnStart, &QCheckBox::toggled, this, &Panel::applyOnStart);
    connect(ui->cmbBoxProfile, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &Panel::profileChanged);

    loadGpu(0);
}

void Panel::loadGpu(int id) {
    selectedGPU = &nvidia.getGpu(id);

    ui->labelGpuName->setText(selectedGPU->productName);
    ui->labelDriverVer->setText(selectedGPU->driverVer);

    // Add min and max clock ranges to sliders
    ClockFreqRanges freqRanges = nvidia.getMinMaxClockFreqs(selectedGPU->id);
    ui->sliderCoreClock->setMaximum(freqRanges.coreMax);
    ui->sliderCoreClock->setMinimum(freqRanges.coreMin);
    ui->sliderMemClock->setMaximum(freqRanges.memMax);
    ui->sliderMemClock->setMinimum(freqRanges.memMin);

    // Add the set clock frequencies
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
    centralWidget()->layout()->addWidget(hwMon);
    hwMon->addChart(GPU_TEMP);
    hwMon->addChart(CORE_CLOCK);
    hwMon->addChart(MEM_CLOCK);
    hwMon->addChart(FAN_SPEED);
}

void Panel::sliderValChanged(int value) {
    QString text = QString::number(value);
    const QSlider* slider = static_cast<QSlider*>(sender());

    if (value > 0 && (slider == ui->sliderCoreClock || slider == ui->sliderMemClock))
        text.prepend('+');

    if (slider == ui->sliderCoreClock)
        ui->editCoreClock->setText(text);
    else if (slider == ui->sliderMemClock)
        ui->editMemClock->setText(text);
    else if (slider == ui->sliderFanSpeed)
        ui->editFanSpeed->setText(text);
}

void Panel::valueEntered() {
    QSlider* slider;
    const QLineEdit* edit = static_cast<QLineEdit*>(sender());

    if (edit == ui->editCoreClock)
        slider = ui->sliderCoreClock;
    else if (edit == ui->editMemClock)
        slider = ui->sliderMemClock;
    else if (edit == ui->editFanSpeed)
        slider = ui->sliderFanSpeed;

    bool ok;
    int val = edit->text().toInt(&ok);
    if (!ok || val < slider->minimum() || val > slider->maximum()) {
        QMessageBox::critical(this, "Error", "Invalid value");
        // Stupid way to reset the text input to what it was before
        slider->valueChanged(slider->value());
    } else {
        slider->setValue(val);
    }
}

void Panel::apply() {
    int coreClock = ui->sliderCoreClock->value();
    int memClock = ui->sliderMemClock->value();
    int fanSpeed = ui->sliderFanSpeed->value();

    try {
        nvidia.setClocks(selectedGPU->id, coreClock, memClock);
        ui->radioFanAuto->isChecked() ?
                    nvidia.setFanSpeedAuto(selectedGPU->id) :
                    nvidia.setManualFanSpeed(selectedGPU->id, fanSpeed);

        statusBar()->showMessage("Settings successfully applied", SB_TEMP_MSG);
    } catch (NvException &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void Panel::enableFanControl(bool enable) {
    ui->sliderFanSpeed->setEnabled(!enable);
    ui->editFanSpeed->setEnabled(!enable);
}

void Panel::newProfile() {
    bool ok;
    QString name = QInputDialog::getText(this, "New Profile", "Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty())
        return;

    try {
        settings.newProfile(selectedGPU->UUID, name);
        ui->cmbBoxProfile->addItem(name);
        ui->cmbBoxProfile->setCurrentIndex(ui->cmbBoxProfile->count()-1);
        statusBar()->showMessage(QString("Created profile \"%1\"").arg(name), SB_TEMP_MSG);
    } catch (SettingsException& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void Panel::deleteProfile() {
    int index = ui->cmbBoxProfile->currentIndex();
    QString profileName = ui->cmbBoxProfile->currentText();

    if (QMessageBox::question(this, "Delete profile?",
                              QString("Are you sure you want to delete profile %1?").arg(profileName),
                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        settings.deleteProfile(selectedGPU->UUID, profileName);
        ui->cmbBoxProfile->removeItem(index);
        statusBar()->showMessage(QString("Deleted profile \"%1\"").arg(profileName), SB_TEMP_MSG);
    }
}

void Panel::saveProfile() {
    QString profileName = ui->cmbBoxProfile->currentText();

    GPUProfile profile;
    profile.coreClock = ui->sliderCoreClock->value();
    profile.memClock = ui->sliderMemClock->value();
    profile.manualFanControl = !ui->radioFanAuto->isChecked();
    profile.manualFanControl ?
                profile.fanSpeed = ui->sliderFanSpeed->value() :
                profile.fanSpeed = 0;

    settings.editProfile(selectedGPU->UUID, profile, profileName);
    statusBar()->showMessage(QString("Saved profile \"%1\"").arg(profileName), SB_TEMP_MSG);
}

void Panel::applyOnStart(bool enable) {
    QString profileName = ui->cmbBoxProfile->currentText();
    settings.setApplyOnStart(selectedGPU->UUID, profileName, enable);
}

void Panel::profileChanged(const QString& profileName) {
    // Set apply on start checkbox, must block signals here
    bool state = ui->chkBoxApplyOnStart->blockSignals(true);
    settings.getApplyOnStart(selectedGPU->UUID) == profileName ?
                ui->chkBoxApplyOnStart->setChecked(true) :
                ui->chkBoxApplyOnStart->setChecked(false);
    ui->chkBoxApplyOnStart->blockSignals(state);

    // Set the profile settings
    const GPUProfile& profile = settings.getProfile(selectedGPU->UUID, profileName);
    ui->sliderCoreClock->setValue(profile.coreClock);
    ui->sliderMemClock->setValue(profile.memClock);
    profile.manualFanControl ?
                ui->radioFanAuto->setChecked(false) :
                ui->radioFanAuto->setChecked(true);
    ui->sliderFanSpeed->setValue(profile.fanSpeed);
}

