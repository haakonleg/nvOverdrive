#include "mainwindow.h"
#include "nvidiacontrol.h"
#include "settings.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    try {
        NvidiaControl nvidia;
        Settings settings;

        MainWindow mainWindow(nvidia, settings);
        mainWindow.show();
        return app.exec();
    } catch (std::exception &e) {
        QMessageBox::critical(nullptr, "Uncaught exception", e.what());
        return 1;
    }
}
