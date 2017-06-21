﻿#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QLibrary>
#include <QModelIndex>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QQuickView>
#include <QScreen>
#include <QUrl>
#include <QVariant>
#include <QWindow>
#include <QtQuick/QQuickItem>
#include <qt_windows.h>

#include "backend.h"
#include "installedlistmodel.h"
#include "monitorlistmodel.h"
#include "packagefilehandler.h"
#include "screenplay.h"
#include "settings.h"
#include "profilelistmodel.h"

int main(int argc, char* argv[])
{

    Q_INIT_RESOURCE(qml);

    QGuiApplication app(argc, argv);

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("Aimber");
    QCoreApplication::setOrganizationDomain("aimber.net");
    QCoreApplication::setApplicationName("ScreenPlay");
    app.setWindowIcon(QIcon(":/assets/icons/favicon.ico"));
    QQuickStyle::setStyle("Material");

    InstalledListModel installedListModel;
    MonitorListModel monitorListModel;
    PackageFileHandler packageFileHandler;
    ProfileListModel profileListModel;
    // Create settings at the end because for now it depends on
    // such things as the profile list model to complete
    Settings settings;
    monitorListModel.loadMonitors();

    QQmlApplicationEngine mainWindow;
    mainWindow.rootContext()->setContextProperty("monitorListModel", &monitorListModel);
    mainWindow.rootContext()->setContextProperty("installedListModel", &installedListModel);
    mainWindow.rootContext()->setContextProperty("settings", &settings);
    mainWindow.rootContext()->setContextProperty("packageFileHandler", &packageFileHandler);
    mainWindow.rootContext()->setContextProperty("profileListModel", &profileListModel);

    mainWindow.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    // FIXME: Needed workaround to close the app because
    // apparently some thread still runs in the background
    QObject::connect(&app, &QGuiApplication::lastWindowClosed,
                     [&](){exit(app.exec()); });
    ScreenPlay sp(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    sp.context()->setContextProperty("installedListModel", &installedListModel);
    sp.context()->setContextProperty("settings", &settings);

    sp.loadQQuickView(QUrl(QStringLiteral("qrc:/qml/Components/ScreenPlay.qml")));
    sp.showQQuickView(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

    installedListModel.loadScreens();
    QObject::connect(&installedListModel, &InstalledListModel::setScreenVisible,
        &sp, &ScreenPlay::setVisible);

    int status = app.exec();

    //Shutdown
    return app.exec();
}
