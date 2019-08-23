#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QStringList>
#include <QUrl>
#include <QtWebEngine>

#include <memory>

#include "src/create.h"
#include "src/installedlistfilter.h"
#include "src/installedlistmodel.h"
#include "src/monitorlistmodel.h"
#include "src/profilelistmodel.h"
#include "src/util.h"
#include "src/screenplaymanager.h"
#include "src/sdkconnector.h"
#include "src/settings.h"

using std::shared_ptr,
    std::make_shared,
    ScreenPlay::Util,
    ScreenPlay::InstalledListModel,
    ScreenPlay::ScreenPlayManager,
    ScreenPlay::InstalledListFilter,
    ScreenPlay::MonitorListModel,
    ScreenPlay::ProfileListModel,
    ScreenPlay::SDKConnector,
    ScreenPlay::Settings,
    ScreenPlay::Create;

int main(int argc, char* argv[])
{

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setOrganizationName("ScreenPlay");
    QGuiApplication::setOrganizationDomain("screen-play.app");
    QGuiApplication::setApplicationName("ScreenPlay");
    QGuiApplication::setApplicationVersion("0.3.0");

    QGuiApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon(":/assets/icons/favicon.ico"));

    // Qt < 6.0 needs this init QtWebEngine
    QtWebEngine::initialize();

    auto installedListModel = make_shared<InstalledListModel>();
    auto installedListFilter = make_shared<InstalledListFilter>(installedListModel);
    auto monitorListModel = make_shared<MonitorListModel>();
    auto profileListModel = make_shared<ProfileListModel>();
    auto sdkConnector = make_shared<SDKConnector>();

    // Create settings in the end because for now it depends on
    // such things as the profile list model to complete
    // It will also set the m_absoluteStoragePath in  profileListModel and installedListModel
    auto settings = make_shared<Settings>(installedListModel, profileListModel, monitorListModel, sdkConnector);

    ScreenPlayManager screenPlay(installedListModel, settings, monitorListModel, sdkConnector, profileListModel);
    Create create(settings);

    QQmlApplicationEngine mainWindowEngine;
    Util util { mainWindowEngine.networkAccessManager() };

    mainWindowEngine.rootContext()->setContextProperty("screenPlay", &screenPlay);
    mainWindowEngine.rootContext()->setContextProperty("screenPlayCreate", &create);
    mainWindowEngine.rootContext()->setContextProperty("utility", &util);
    mainWindowEngine.rootContext()->setContextProperty("installedListFilter", installedListFilter.get());
    mainWindowEngine.rootContext()->setContextProperty("monitorListModel", monitorListModel.get());
    mainWindowEngine.rootContext()->setContextProperty("installedListModel", installedListModel.get());
    mainWindowEngine.rootContext()->setContextProperty("profileListModel", profileListModel.get());
    mainWindowEngine.rootContext()->setContextProperty("screenPlaySettings", settings.get());
    mainWindowEngine.load(QUrl(QStringLiteral("qrc:/main.qml")));


    // Instead of setting "renderType: Text.NativeRendering" every time
    // we can set it here once :)
    auto* window = static_cast<QQuickWindow*>(mainWindowEngine.rootObjects().first());
    if (!window)
        qFatal("Could not receive window to set font rendering.");

    window->setTextRenderType(QQuickWindow::TextRenderType::NativeTextRendering);

    // Set visible if the -silent parameter was not set
    QStringList argumentList = app.arguments();
    if (!argumentList.contains("-silent")) {
        settings->setMainWindowVisible(true);
    }
    installedListModel->init();

    return app.exec();
}
