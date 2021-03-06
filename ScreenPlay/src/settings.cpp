#include "settings.h"

namespace ScreenPlay {

/*!
    \class ScreenPlay::Settings
    \inmodule ScreenPlay
    \brief Global settings class for reading and writing settings.

    Used for:
    \list
        \li User configuration via AppData\Local\ScreenPlay\ScreenPlay
        \list
            \li profiles.json - saved wallpaper and widgets config
            \li Computer\\HKEY_CURRENT_USER\\Software\\ScreenPlay\\ScreenPlay - ScreenPlayContentPath for steam plugin
        \endlist
        \li Communication via the SDK Connector
    \endlist
    Currently we save the regular settings (not the setup of the wallpaper and widgets) in two different locations.
    This can change in the future!
*/

/*!
    Constructor and sets up:
    \list 1
        \li Sets the git build hash via ScreenPlay.pro c++ define
        \li Checks the langauge via settings or system and available ones and installes a translator.
        \li Checks the paths for config folders in appdata
        \li Checks the AbsoluteStoragePath.
        \li Checks regisitry for steam plugin settings
        \li Parses autostart, anonymousTelemetry, highPriorityStart
        \li Checks ScreenPlayWallpaper and ScreenPlayWidgets executable paths.
    \endlist
    More errorchecking is needed here. For example when the proile or settings json cannot to parsed, use the default settings.
 */
Settings::Settings(const shared_ptr<GlobalVariables>& globalVariables,
    QObject* parent)
    : QObject(parent)
    , m_globalVariables { globalVariables }
{
    qRegisterMetaType<Settings::FillMode>("Settings::FillMode");
    qRegisterMetaType<Settings::Language>("Settings::Language");
    qmlRegisterUncreatableType<Settings>("Settings", 1, 0, "Settings", "Error only for enums");

    {
        if (!m_qSettings.contains("Autostart")) {
#ifdef Q_OS_WIN
            QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
            if (!m_qSettings.value("Autostart").toBool()) {
                if (!settings.contains("ScreenPlay")) {
                }
            }
            settings.setValue("ScreenPlay", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + " -silent");
            settings.sync();
#endif
            m_qSettings.setValue("Autostart", true);
            m_qSettings.sync();
        } else {
            setAutostart(m_qSettings.value("Autostart", true).toBool());
        }
    }

    setCheckWallpaperVisible(m_qSettings.value("CheckWallpaperVisible", false).toBool());
    setHighPriorityStart(m_qSettings.value("ScreenPlayExecutable", false).toBool());
    if (m_qSettings.contains("VideoFillMode")) {
        auto value = m_qSettings.value("VideoFillMode").toString();
        setVideoFillMode(QStringToEnum<FillMode>(value, FillMode::Fill));
    }
    setAnonymousTelemetry(m_qSettings.value("AnonymousTelemetry", true).toBool());

    // Wallpaper and Widgets config
    QFile profilesFile(m_globalVariables->localSettingsPath().toString() + "/profiles.json");
    if (!profilesFile.exists()) {
        qInfo("No profiles.json found, creating default profiles.json");
        qDebug() << profilesFile;
        writeJsonFileFromResource("profiles");
    }

    //If empty use steam workshop location
    if (QString(m_qSettings.value("ScreenPlayContentPath").toString()).isEmpty()) {

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
        QDir dir;
        QString path = QGuiApplication::instance()->applicationDirPath() + "/../../workshop/content/672870";
        if (!dir.mkpath(path)) {
            qWarning() << "Could not create steam workshop path for path: " << path;
        } else {
            m_globalVariables->setLocalStoragePath(QUrl::fromUserInput(path));
            m_qSettings.setValue("ScreenPlayContentPath", dir.cleanPath(path));
            m_qSettings.sync();
        }

    } else {
        m_globalVariables->setLocalStoragePath(QUrl::fromUserInput(m_qSettings.value("ScreenPlayContentPath").toString()));
    }

    setupWidgetAndWindowPaths();
    setGitBuildHash(GIT_VERSION);
}

/*!
  Writes the default JsonFile from the resources and the given \a filename. Currently we have two default json files:
   \list
    \li profiles.json
    \li settings.json
   \endlist
*/
void Settings::writeJsonFileFromResource(const QString& filename)
{
    QFile file(m_globalVariables->localSettingsPath().toString() + "/" + filename + ".json");
    QFile defaultSettings(":/" + filename + ".json");

    file.open(QIODevice::WriteOnly | QIODevice::Text);
    defaultSettings.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream out(&file);
    QTextStream defaultOut(&defaultSettings);

    out << defaultOut.readAll();

    file.close();
    defaultSettings.close();
}

/*!
  To have a better developer experience we check if we use a debug version. Then we assume
  That the paths are the default QtCreator paths and set the widgets and wallpaper executable
  paths accordingly.
*/
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

/*!
  When no default language is set in the registry we check the system set language. If there is no
  matching translation is available we set it to english. This function gets called from the UI when
  the user manually changes the language.
*/
void Settings::restoreDefault(const QString& appConfigLocation, const QString& settingsFileType)
{
    QString fullSettingsPath { appConfigLocation + "/" + settingsFileType + ".json" };
    qWarning() << "Unable to load config from " << fullSettingsPath
               << ". Restoring default!";
    QFile file { fullSettingsPath };
    file.remove();
    writeJsonFileFromResource(settingsFileType);
}

void Settings::setupLanguage()
{
    QString langCode;
    if (m_qSettings.value("Language").isNull()) {
        auto localeList = QLocale::system().uiLanguages();

        // Like En-us, De-de
        QStringList localeSplits = localeList.at(0).split("-");
        langCode = localeSplits.at(0);

        // Ether De, En, Ru, Fr...
        if (langCode.length() != 2) {
            qWarning() << "Could not parse locale of value " << langCode;
            return;
        }
    } else {
        langCode = m_qSettings.value("Language").toString();
    }

    setLanguage(QStringToEnum<Language>(langCode, Language::En));
    retranslateUI();
}

bool Settings::retranslateUI()
{
    auto* app = static_cast<QGuiApplication*>(QGuiApplication::instance());
    QString langCode = QVariant::fromValue(language()).toString();
    langCode = langCode.toLower();
    QFile tsFile;

    if (tsFile.exists(":/translations/ScreenPlay_" + langCode + ".qm")) {
        m_translator.load(":/translations/ScreenPlay_" + langCode + ".qm");
        app->installTranslator(&m_translator);
        emit requestRetranslation();

        if (language() == Settings::Language::Ko) {
            setFont("Noto Sans CJK KR Regular");
        } else {
            setFont("Roboto");
        }
        return true;
    }
    return false;
}
}
