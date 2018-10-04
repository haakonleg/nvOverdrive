#include "include/panel.h"
#include "include/nvidiacontrol.h"
#include "include/settings.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    try {
        NvidiaControl nvidia;
        Settings settings;

        Panel panel(nvidia, settings);
        panel.show();
        return app.exec();
    } catch (std::exception &e) {
        QMessageBox::critical(nullptr, "Uncaught exception", e.what());
        return 1;
    }
}
