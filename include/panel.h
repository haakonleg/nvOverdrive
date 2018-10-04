#ifndef PANEL_H
#define PANEL_H

#include <QMainWindow>
#include <QMessageBox>
#include <QInputDialog>
#include <memory>

#include "ui_panel.h"
#include "settings.h"
#include "hardwaremonitor.h"
#include "nvidiacontrol.h"

namespace Ui {
class Panel;
}

class Panel : public QMainWindow {
    Q_OBJECT

public:
    explicit Panel(NvidiaControl& nvidia, Settings& settings, QWidget *parent = 0);

private:
    std::unique_ptr<Ui::Panel> ui;
    NvidiaControl& nvidia;
    Settings& settings;
    const GPU* selectedGPU;
    HardwareMonitor* hwMon = nullptr;

    void loadGpu(int id);
    void sliderValChanged(int value);
    void valueEntered();
    void apply();
    void updateUI();
    void enableFanControl(bool enable);
    void newProfile();
    void deleteProfile();
    void saveProfile();
    void applyOnStart();
};

#endif // PANEL_H
