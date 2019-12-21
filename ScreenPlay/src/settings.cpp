#include "settings.h"


namespace ScreenPlay {

/*!
    \class ScreenPlay::Settings
    \inmodule ScreenPlay
    \brief Used for:
    \list
        \i User configuration
        \i Communication via the SDK Connector
    \endlist

*/

Settings::Settings(const shared_ptr<GlobalVariables>& globalVariables,
    QObject* parent)
    : QObject(parent)
    , m_version { QVersionNumber(1, 0, 0) }
    , m_globalVariables { globalVariables }
{

    setGitBuildHash(GIT_VERSION);

#ifdef QT_NO_DEBUG
    qInfo() << "ScreenPlay git hash: " << gitBuildHash();
#endif

    setupLanguage();

    if (m_qSettings.value("ScreenPlayExecutable").isNull()) {
        m_qSettings.setValue("ScreenPlayExecutable", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
        m_qSettings.sync();
    }

    QString appConfigLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    m_globalVariables->setLocalSettingsPath(QUrl::fromUserInput(appConfigLocation));

    if (!QDir(appConfigLocation).exists()) {
        if (!QDir().mkpath(appConfigLocation)) {
            qWarning("ERROR: Cloud not create appConfigLocation dir");
            return;
        }
    }

    // App settings
    QFile settingsFile(appConfigLocation + "/settings.json");
    if (!settingsFile.exists()) {
        qInfo("No Settings found, creating default settings");
        writeJsonFileFromResource("settings");
        setAutostart(true);
        setAnonymousTelemetry(true);
    }

    // Wallpaper and Widgets config
    QFile profilesFile(appConfigLocation + "/profiles.json");
    if (!profilesFile.exists()) {
        qInfo("No profiles.json found, creating default settings");
        writeJsonFileFromResource("profiles");
    }

    std::optional<QJsonObject> configObj = Util::openJsonFileToObject(appConfigLocation + "/settings.json");
    std::optional<QVersionNumber> version = Util::getVersionNumberFromString(configObj.value().value("version").toString());

    //Checks if the settings file has the same version as ScreenPlay
    if (version.has_value() && version.value() != m_version) {
        // TODO(Kelteseth): Display error message
        qWarning() << "Version missmatch fileVersion: " << version.value().toString() << "m_version: " << m_version.toString();
        return;
    }

    //If empty use steam workshop location
    if (QString(configObj.value().value("absoluteStoragePath").toString()).isEmpty()) {
        qDebug() << QCoreApplication::applicationDirPath();
        QDir steamWorkshopContentPath(QCoreApplication::applicationDirPath());
        steamWorkshopContentPath.cdUp();
        steamWorkshopContentPath.cdUp();

        /*
         * ! We must use this (ugly) method, because to stay FOSS we cannot call the steamAPI here !
         *
         * We start with the assumption that when we go up 2 folder.
         * So that there must be at least a common folder:
         * Windows example:
         * From -> C:\Program Files (x86)\Steam\steamapps\common\ScreenPlay
         * To   -> C:\Program Files (x86)\Steam\steamapps\
         * Dest.-> C:\Program Files (x86)\Steam\steamapps\workshop\content\672870
         *
         * When we reach the folder it _can_ contain a workshop folder when the user
         * previously installed any workshop content. If the folder does not exsist we
         * need to create it by hand. Normally Steam will create this folder but we need to
         * set it here at this point so that the QFileSystemWatcher in InstalledListModel does
         * not generate warnings.
         */

        bool hasCommonFolder = steamWorkshopContentPath.entryList().contains("common");

        if (!hasCommonFolder) {
            QString basePath = steamWorkshopContentPath.path();

            auto checkIfFolderExsistOrCreate = [](const QString& path, const QString& foldername) {
                QDir checkDir { path };

                if (!checkDir.cd(foldername)) {
                    return checkDir.mkdir(foldername);
                } else {
                    return true;
                }
            };

            if (checkIfFolderExsistOrCreate(basePath, "common")) {
                if (checkIfFolderExsistOrCreate(basePath + "/common", "content")) {
                    if (checkIfFolderExsistOrCreate(basePath + "/common/content", "672870")) {
                    }
                }
            }

        } else {
            steamWorkshopContentPath.cd("workshop");
            steamWorkshopContentPath.cd("content");
            steamWorkshopContentPath.cd("672870");

            m_globalVariables->setLocalStoragePath("file:///" + steamWorkshopContentPath.absolutePath());
            if (!writeSingleSettingConfig("absoluteStoragePath", m_globalVariables->localStoragePath())) {
                qWarning() << "Could not write settings file. Setup steam workshop folder at: " << m_globalVariables->localStoragePath();
            }
        }

    } else {
        m_globalVariables->setLocalStoragePath(QUrl::fromUserInput(configObj.value().value("absoluteStoragePath").toString()));
    }

    // We need these settings also in the steam version.
    // This way it is easier to access them. Maybe we should move everything into
    // the windows registry
    //Computer\HKEY_CURRENT_USER\Software\ScreenPlay\ScreenPlay
    if (m_qSettings.value("ScreenPlayContentPath").toUrl() != QUrl::fromLocalFile(m_globalVariables->localStoragePath().toString())) {
        m_qSettings.setValue("ScreenPlayContentPath", QDir::toNativeSeparators(m_globalVariables->localStoragePath().toString().remove("file:///")));
        m_qSettings.sync();
    }

    m_autostart = configObj.value().value("autostart").toBool();
    m_highPriorityStart = configObj.value().value("highPriorityStart").toBool();
    m_anonymousTelemetry = configObj.value().value("anonymousTelemetry").toBool();

    setupWidgetAndWindowPaths();
}

bool Settings::writeSingleSettingConfig(QString name, QVariant value)
{
    QString filename = m_globalVariables->localSettingsPath().toLocalFile() + "/settings.json";
    auto obj = Util::openJsonFileToObject(filename);

    if (!obj.has_value()) {
        qWarning("Settings Json Parse Error ");
        return false;
    }

    QJsonObject newConfig = obj.value();
    newConfig.insert(name, QJsonValue::fromVariant(value));

    return Util::writeJsonObjectToFile(filename, newConfig);
}

void Settings::writeJsonFileFromResource(const QString& filename)
{
    QFile file(m_globalVariables->localSettingsPath().toLocalFile() + "/" + filename + ".json");
    QFile defaultSettings(":/" + filename + ".json");

    file.open(QIODevice::WriteOnly | QIODevice::Text);
    defaultSettings.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream out(&file);
    QTextStream defaultOut(&defaultSettings);

    out << defaultOut.readAll();

    file.close();
    defaultSettings.close();
}

void Settings::setupWidgetAndWindowPaths()
{
    QDir workingDir(QDir::currentPath());
    QDir baseDir(QDir::currentPath());

#ifdef QT_DEBUG

    if (workingDir.cdUp()) {

#ifdef Q_OS_OSX
        m_globalVariables->setWidgetExecutablePath(QUrl::fromUserInput(workingDir.path() + "/../../../ScreenPlayWidget/ScreenPlayWidget.app/Contents/MacOS/ScreenPlayWidget").toLocalFile());
        m_globalVariables->setWallpaperExecutablePath(QUrl::fromUserInput(workingDir.path() + "/../../../ScreenPlayWallpaper/ScreenPlayWallpaper.app/Contents/MacOS/ScreenPlayWallpaper").toLocalFile());
#endif

#ifdef Q_OS_WIN
        m_globalVariables->setWidgetExecutablePath(QUrl(workingDir.path() + "/ScreenPlayWidget/debug/ScreenPlayWidget.exe"));
        m_globalVariables->setWallpaperExecutablePath(QUrl(workingDir.path() + "/ScreenPlayWallpaper/debug/ScreenPlayWallpaper.exe"));
#endif
    }

    // We need to detect the right base path so we can copy later the example projects
    baseDir.cdUp();
    baseDir.cdUp();
    baseDir.cd("ScreenPlay");
    baseDir.cd("ScreenPlay");
#endif
#ifdef QT_NO_DEBUG
    qDebug() << "Starting in Release mode!";

    // If we build in the release version we must be cautious!
    // The working dir in steam is the ScreenPlay.exe location
    // In QtCreator is the dir above ScreenPlay.exe (!)

    workingDir.cdUp();
    workingDir.cd("ScreenPlayWallpaper");

    if (QDir(workingDir.path() + "/release").exists()) {
        // If started by QtCreator
        workingDir.cd("release");
        m_globalVariables->setWallpaperExecutablePath(QUrl(workingDir.path() + "/ScreenPlayWallpaper.exe"));
        workingDir.cdUp();
        workingDir.cdUp();
        workingDir.cd("ScreenPlayWidget");
        workingDir.cd("release");
        m_globalVariables->setWidgetExecutablePath(QUrl(workingDir.path() + "/ScreenPlayWidget.exe"));
    } else {
        // If started by Steam
        m_globalVariables->setWallpaperExecutablePath(QUrl("ScreenPlayWallpaper.exe"));
        m_globalVariables->setWidgetExecutablePath(QUrl("ScreenPlayWidget.exe"));
    }
#endif
}

void Settings::setupLanguage()
{
    auto* app = static_cast<QGuiApplication*>(QGuiApplication::instance());
    if (m_qSettings.value("language").isNull()) {
        auto locale = QLocale::system().uiLanguages();
        auto localeSplits = locale.at(0).split("-");

        // Only install a translator if one exsists
        QFile tsFile;
        qDebug() << ":/translations/ScreenPlay_" + localeSplits.at(0) + ".qm";
        if (tsFile.exists(":/translations/ScreenPlay_" + localeSplits.at(0) + ".qm")) {
            m_translator.load(":/translations/ScreenPlay_" + localeSplits.at(0) + ".qm");
            m_qSettings.setValue("language", QVariant(localeSplits.at(0)));
            m_qSettings.sync();
            app->installTranslator(&m_translator);
        }
    } else {
        QFile tsFile;
        if (tsFile.exists(":/translations/ScreenPlay_" + m_qSettings.value("language").toString() + ".qm")) {
            m_translator.load(":/translations/ScreenPlay_" + m_qSettings.value("language").toString() + ".qm");
            app->installTranslator(&m_translator);
        }
    }
}

}
