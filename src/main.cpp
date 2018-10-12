#include "include/panel.h"
#include "include/nvidiacontrol.h"
#include "include/settings.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    try {
        NvidiaControl nvidia;
        Settings settings;

        // Check for profiles to apply at start
        const auto& gpus = nvidia.getGpus();
        for (const GPU& gpu : gpus) {
            const QString profileName = settings.getApplyOnStart(gpu.UUID);
            if (profileName.isEmpty())
                continue;

            // Apply the profile
            qDebug() << "Applying profile " << profileName;
            const GPUProfile& profile = settings.getProfile(gpu.UUID, profileName);

            nvidia.setClocks(gpu.id, profile.coreClock, profile.memClock);
            profile.manualFanControl ?
                nvidia.setManualFanSpeed(gpu.id, profile.fanSpeed) :
                nvidia.setFanSpeedAuto(gpu.id);
        }

        Panel panel(nvidia, settings);
        panel.show();
        return app.exec();
    } catch (std::exception &e) {
        QMessageBox::critical(nullptr, "Uncaught exception", e.what());
        return 1;
    }
}
