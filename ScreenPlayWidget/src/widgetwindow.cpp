#include "widgetwindow.h"

#include <QCoreApplication>

WidgetWindow::WidgetWindow(QString projectPath, QString appid, QObject* parent)
    : QObject(parent)
    , m_appID { appid }
{

    Qt::WindowFlags flags = m_window.flags();
    m_window.setWidth(300);
    m_window.setHeight(300);
    m_window.setFlags(flags | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::BypassWindowManagerHint | Qt::SplashScreen);
    m_window.rootContext()->setContextProperty("window", this);
    m_window.setColor(Qt::transparent);

#ifdef Q_OS_WIN
    m_hwnd = reinterpret_cast<HWND>(m_window.winId());
    setWindowBlur();
#endif

    if (projectPath != "test") {
        QFile configTmp;
        QJsonDocument configJsonDocument;
        QJsonParseError parseError;

        configTmp.setFileName(projectPath + "/project.json");
        configTmp.open(QIODevice::ReadOnly | QIODevice::Text);
        m_projectConfig = configTmp.readAll();
        configJsonDocument = QJsonDocument::fromJson(m_projectConfig.toUtf8(), &parseError);

        if (!(parseError.error == QJsonParseError::NoError)) {
            qWarning("Settings Json Parse Error ");
        }

        m_project = configJsonDocument.object();
        QString fullPath = "file:///" + projectPath + "/" + m_project.value("file").toString();
        setSourcePath(fullPath);
    } else {
        setSourcePath("qrc:/test.qml");
    }
    // Instead of setting "renderType: Text.NativeRendering" every time
    // we can set it here once :)
    m_window.setTextRenderType(QQuickWindow::TextRenderType::NativeTextRendering);
    //    m_window.setResizeMode(QQuickView::ResizeMode::SizeViewToRootObject);
    m_window.setSource(QUrl("qrc:/mainWidget.qml"));
    m_window.show();
}

void WidgetWindow::setSize(QSize size)
{
    m_window.setWidth(size.width());
    m_window.setHeight(size.height());
}

void WidgetWindow::destroyThis()
{
    QCoreApplication::quit();
}

void WidgetWindow::messageReceived(QString key, QString value)
{
    emit qmlSceneValueReceived(key, value);
}

void WidgetWindow::setPos(int xPos, int yPos)
{

    QPoint delta((xPos - m_clickPos.x()), (yPos - m_clickPos.y()));
    int new_x = m_window.x() + delta.x();
    int new_y = m_window.y() + delta.y();

    m_window.setPosition(QPoint(new_x, new_y));
}

void WidgetWindow::setClickPos(const QPoint& clickPos)
{
    m_clickPos = clickPos;
}

void WidgetWindow::setWidgetSize(const int with, const int height)
{
    m_window.setWidth(with);
    m_window.setHeight(height);
}

#ifdef Q_OS_WIN
void WidgetWindow::setWindowBlur(unsigned int style)
{

    const HINSTANCE hModule = LoadLibrary(TEXT("user32.dll"));
    if (hModule) {
        struct ACCENTPOLICY {
            int nAccentState;
            int nFlags;
            int nColor;
            int nAnimationId;
        };
        struct WINCOMPATTRDATA {
            int nAttribute;
            PVOID pData;
            ULONG ulDataSize;
        };
        enum class Accent {
            DISABLED = 0,
            GRADIENT = 1,
            TRANSPARENTGRADIENT = 2,
            BLURBEHIND = 3,
            ACRYLIC = 4,
            INVALID = 5
        };
        typedef BOOL(WINAPI * pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);
        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hModule, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute) {
            ACCENTPOLICY policy = { static_cast<int>(style), 0, 0, 0 }; // ACCENT_ENABLE_BLURBEHIND=3...
            WINCOMPATTRDATA data = { 19, &policy, sizeof(ACCENTPOLICY) }; // WCA_ACCENT_POLICY=19
            SetWindowCompositionAttribute(m_hwnd, &data);
        }
        FreeLibrary(hModule);
    }
}
#endif