#include "include/settings.h"

#define APP "App"
#define APPLY_ON_START "apply_on_start"
#define PROFILES "Profiles"
#define POWERLIMIT "powerLimit"
#define CORECLOCK "coreClock"
#define MEMCLOCK "memClock"
#define MAN_FAN_CONTROL "manualFanControl"
#define FANSPEED "fanSpeed"

GPUProfile::GPUProfile(int powerLimit, int coreClock, int memClock, bool manualFanControl, int fanSpeed) {
    this->powerLimit = powerLimit;
    this->coreClock = coreClock;
    this->memClock = memClock;
    this->manualFanControl = manualFanControl;
    this->fanSpeed = fanSpeed;
}

GPUProfile::GPUProfile(const QJsonObject &json) {
    powerLimit = json[POWERLIMIT].toInt();
    coreClock = json[CORECLOCK].toInt();
    memClock = json[MEMCLOCK].toInt();
    manualFanControl = json[MAN_FAN_CONTROL].toBool();
    fanSpeed = json[FANSPEED].toInt();
}

QJsonObject GPUProfile::serialize() const {
    QJsonObject json;
    json[POWERLIMIT] = powerLimit;
    json[CORECLOCK] = coreClock;
    json[MEMCLOCK] = memClock;
    json[MAN_FAN_CONTROL] = manualFanControl;
    json[FANSPEED] = fanSpeed;
    return json;
}

Settings::Settings() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configDir.isEmpty())
        throw SettingsException("Cannot determine a directory to save configuration to");

    QString fileName = configDir + "/nvOverdrive/nvOverdrive.config";
    configFile = std::make_unique<QFile>(fileName);

    // Read settings if exist
    if (configFile->exists())
        readSettings();
}

void Settings::writeSettings() {
    if (!configFile->open(QIODevice::WriteOnly|QIODevice::Text))
        throw SettingsException("Failed to open config file in write mode");

    QJsonObject settingsObj;
    writeAppSettings(settingsObj);
    writeProfiles(settingsObj);
    configFile->write(QJsonDocument(settingsObj).toJson());
    configFile->close();
}

void Settings::writeAppSettings(QJsonObject& json) {
    QJsonObject appSettingsObj;

    QJsonObject applyOnStartObj;
    for (auto it = applyOnStart.cbegin(); it != applyOnStart.cend(); ++it) {
        applyOnStartObj[it.key()] = it.value();
    }
    appSettingsObj[APPLY_ON_START] = applyOnStartObj;

    json[APP] = appSettingsObj;
}

void Settings::writeProfiles(QJsonObject& json) {
    QJsonObject profilesObj;

    // Create objects for each GPU
    for (auto it = gpuProfiles.cbegin(); it != gpuProfiles.cend(); ++it) {
        const QString& gpuUUID = it.key();
        const auto& profiles = it.value();

        // Add each profile
        QJsonObject gpuProfilesObj;
        for (auto it2 = profiles.cbegin(); it2 != profiles.cend(); ++it2) {
            const QString& profileName = it2.key();
            const GPUProfile& profile = it2.value();

            gpuProfilesObj[profileName] = profile.serialize();
        }
        profilesObj[gpuUUID] = gpuProfilesObj;
    }

    json[PROFILES] = profilesObj;
}

void Settings::readSettings() {
    if (!configFile->open(QIODevice::ReadOnly|QIODevice::Text))
        throw SettingsException("Failed to open config file in read mode");

    auto data = configFile->readAll();
    QJsonDocument settingsDoc = QJsonDocument::fromJson(data);
    readAppSettings(settingsDoc.object());
    readProfiles(settingsDoc.object());
    configFile->close();
}

void Settings::readAppSettings(const QJsonObject& json) {
    QJsonObject settingsObj = json[APP].toObject();

    QJsonObject applyOnStartObj = json[APPLY_ON_START].toObject();
    for (auto it = applyOnStartObj.constBegin(); it != applyOnStartObj.constEnd(); ++it) {
        applyOnStart[it.key()] = it.value().toString();
    }
}

void Settings::readProfiles(const QJsonObject& json) {
    QJsonObject profilesObj = json[PROFILES].toObject();

    // Read all gpu UUID entries
    for (auto it = profilesObj.constBegin(); it != profilesObj.constEnd(); ++it) {
        const QString& gpuUUID = it.key();
        QJsonObject profiles = it.value().toObject();

        if (profiles.size() > 0)
            gpuProfiles[gpuUUID] = QMap<QString, GPUProfile>();

        // Now read all profiles for the entry
        for (auto it2 = profiles.constBegin(); it2 != profiles.constEnd(); ++it2)
            gpuProfiles[gpuUUID].insert(it2.key(), GPUProfile(it2.value().toObject()));
    }
}

// If there are no profiles for a GPU, just create a temporary profile
const QMap<QString, GPUProfile>& Settings::getGPUProfiles(const QString& gpuUUID) {
    if (!gpuProfiles.contains(gpuUUID)) {
        gpuProfiles[gpuUUID] = QMap<QString, GPUProfile>();
        gpuProfiles[gpuUUID].insert("Default", GPUProfile());
        writeSettings();
    }
    return gpuProfiles[gpuUUID];
}

void Settings::newProfile(const QString &gpuUUID, const QString& profileName) {
    // Check if name already exists
    auto& profiles = gpuProfiles[gpuUUID];
    if (profiles.contains(profileName))
            throw SettingsException("A profile with this name already exists");

    profiles.insert(profileName, GPUProfile());
    writeSettings();
}

void Settings::deleteProfile(const QString& gpuUUID, const QString& profileName) {
    gpuProfiles[gpuUUID].remove(profileName);

    // Check if "apply on start" must be removed
    if (applyOnStart[gpuUUID] == profileName)
        applyOnStart.remove(gpuUUID);

    writeSettings();
}

void Settings::editProfile(const QString& gpuUUID, GPUProfile edited, const QString& profileName) {
    gpuProfiles[gpuUUID][profileName] = edited;
    writeSettings();
}

void Settings::setApplyOnStart(const QString &gpuUUID, const QString &profileName) {
    applyOnStart[gpuUUID] = profileName;
    writeSettings();
}
