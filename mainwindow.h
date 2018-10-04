#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>

#include "ui_mainwindow.h"
#include "settings.h"
#include "hardwaremonitor.h"
#include "nvidiacontrol.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(NvidiaControl& nvidia, Settings& settings, QWidget* parent = 0);
private:
    std::unique_ptr<Ui::MainWindow> ui;
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

#endif // MAINWINDOW_H
