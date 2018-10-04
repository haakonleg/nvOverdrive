#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFile>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <memory>

class SettingsException : public std::exception {
private:
    QString message;
public:
    SettingsException(const QString &message) { this->message = "Settings: " + message; }
    const char* what() const throw() override { return message.toLatin1().data(); }
};

struct GPUProfile {
    int powerLimit;
    int coreClock;
    int memClock;
    bool manualFanControl;
    int fanSpeed;

    GPUProfile(int powerLimit = 100, int coreClock = 0, int memClock = 0, bool manualFanControl = false, int fanSpeed = 0);
    GPUProfile(const QJsonObject& json);
    QJsonObject serialize() const;
};

class Settings {
private:
    QMap<QString, QString> applyOnStart;
    QMap<QString, QMap<QString, GPUProfile>> gpuProfiles;
    std::unique_ptr<QFile> configFile;

    void createDefaultSettings();
    void writeSettings();
    void writeAppSettings(QJsonObject& json);
    void writeProfiles(QJsonObject& json);
    void readSettings();
    void readAppSettings(const QJsonObject& json);
    void readProfiles(const QJsonObject& json);
public:
    Settings();

    const QMap<QString, GPUProfile>& getGPUProfiles(const QString& gpuUUID);
    void newProfile(const QString& gpuUUID, const QString& profileName);
    void deleteProfile(const QString& gpuUUID, const QString& profileName);
    void editProfile(const QString& gpuUUID, GPUProfile edited, const QString& profileName);
    void setApplyOnStart(const QString& gpuUUID, const QString& profileName);
};

#endif // SETTINGS_H
