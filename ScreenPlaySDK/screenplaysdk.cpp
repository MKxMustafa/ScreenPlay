#include "screenplaysdk.h"

// USE THIS ONLY FOR redirectMessageOutputToMainWindow
static ScreenPlaySDK* global_sdkPtr = nullptr;

ScreenPlaySDK::ScreenPlaySDK(QQuickItem* parent)
    : QQuickItem(parent)
{
    init();
}

ScreenPlaySDK::ScreenPlaySDK(
    const QString& appID,
    const QString& type,
    QQuickItem* parent)
    : QQuickItem(parent)
    , m_type { type }
    , m_appID { appID }
{
    init();
}

void ScreenPlaySDK::init()
{
    // Redirect all messages from this to ScreenPlay
    global_sdkPtr = this;
    qInstallMessageHandler(ScreenPlaySDK::redirectMessageOutputToMainWindow);

    m_socket.setServerName("ScreenPlay");
    connect(&m_socket, &QLocalSocket::connected, this, &ScreenPlaySDK::connected);
    connect(&m_socket, &QLocalSocket::disconnected, this, &ScreenPlaySDK::disconnected);
    connect(&m_socket, &QLocalSocket::bytesWritten, this, &ScreenPlaySDK::bytesWritten);
    connect(&m_socket, &QLocalSocket::readyRead, this, &ScreenPlaySDK::readyRead);
    connect(&m_socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error), this, &ScreenPlaySDK::error);
    m_socket.connectToServer();
}

ScreenPlaySDK::~ScreenPlaySDK()
{
    m_socket.disconnectFromServer();
}

void ScreenPlaySDK::connected()
{
    QByteArray welcomeMessage = QString(m_appID + "," + m_type).toUtf8();
    m_socket.write(welcomeMessage);
    m_socket.waitForBytesWritten();

    setIsConnected(true);
    emit sdkConnected();
}

void ScreenPlaySDK::disconnected()
{
    setIsConnected(false);
    emit sdkDisconnected();
}

void ScreenPlaySDK::bytesWritten(qint64 bytes)
{
}

void ScreenPlaySDK::readyRead()
{
    QString tmp = m_socket.readAll();
    //qDebug() << "SDK MESSAGE RECEIVED: " << tmp;
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(QByteArray::fromStdString(tmp.toStdString()), &err);

    if (!(err.error == QJsonParseError::NoError)) {
        emit incommingMessageError(err.errorString());
        return;
    }
    QJsonObject ob = doc.object();
    QJsonObject::iterator iterator;
    for (iterator = ob.begin(); iterator != ob.end(); iterator++) {
        emit incommingMessage(iterator.key(), ob.value(iterator.key()).toString());
    }
}

void ScreenPlaySDK::error(QLocalSocket::LocalSocketError socketError)
{
    if (socketError == QLocalSocket::LocalSocketError::ConnectionRefusedError) {
        QCoreApplication::quit();
    }
}

void ScreenPlaySDK::redirectMessage(QByteArray& msg)
{
    if (isConnected()) {
        m_socket.write(msg);
        m_socket.waitForBytesWritten();
    }
}

void ScreenPlaySDK::ScreenPlaySDK::redirectMessageOutputToMainWindow(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if (global_sdkPtr == nullptr)
        return;

    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray file = "File: " + QByteArray(context.file) + ", ";
    QByteArray line = "in line " + QByteArray::number(context.line) + ", ";
    //QByteArray function = "function " + QByteArray(context.function) + ", Message: ";

    //localMsg = file + line +  localMsg;
    global_sdkPtr->redirectMessage(localMsg);

    switch (type) {
    case QtDebugMsg:
        //localMsg = " SDK START: " /*+  QByteArray::fromStdString(global_sdkPtr->contentType().toStdString()) + " "*/ + localMsg;
        break;
    case QtInfoMsg:
        //fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        //fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        //fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        //(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    }
}
