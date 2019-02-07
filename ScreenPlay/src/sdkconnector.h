#pragma once

#include <QJsonValue>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>
#include <QVector>

/*!
    \class SDKConnector
    \brief Used for every Wallpaper, Scene or Widget communication via Windows pipes/QLocalSocket

*/
class SDKConnection;

class SDKConnector : public QObject {
    Q_OBJECT
public:
    explicit SDKConnector(QObject* parent = nullptr);

signals:

public slots:
    void readyRead();
    void newConnection();
    void closeAllWallpapers();
    void closeWallpapersAt(int at);
    void setWallpaperValue(QString appID, QString key, QString value);
    void setSceneValue(QString appID, QString key, QString value);

private:
    QSharedPointer<QLocalServer> m_server;
    QVector<QSharedPointer<SDKConnection>> m_clients;
};

class SDKConnection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString appID READ appID WRITE setAppID NOTIFY appIDChanged)
    Q_PROPERTY(QVector<int> monitor READ monitor WRITE setMonitor NOTIFY monitorChanged)

public:
    explicit SDKConnection(QLocalSocket* socket, QObject* parent = nullptr) : QObject(parent)
    {

        m_socket = socket;
        connect(m_socket, &QLocalSocket::readyRead, this, &SDKConnection::readyRead);
        connect(m_socket, &QLocalSocket::disconnected, this, &SDKConnection::close);
    }
    ~SDKConnection (){
        qDebug() << "Terminating connection object";
    }

    QString appID() const
    {
        return m_appID;
    }

    QLocalSocket* socket() const;

    QVector<int> monitor() const
    {
        return m_monitor;
    }

signals:
    void requestCloseAt(int at);
    void appIDChanged(QString appID);
    void monitorChanged(QVector<int> monitor);

public slots:
    void readyRead()
    {

        auto msg = QString(m_socket->readAll());

        // The first message allways contains the appID
        if (msg.startsWith("appID=")) {
            //Only use the first 32 chars for the appID
            m_appID = msg.remove("appID=").mid(0, 32);
            msg.remove(m_appID);
            qDebug() << "###### Wallpaper width APPID created:"  << "\n######" << m_appID;
        } else {
            qDebug() << "### Message from: " << m_appID << "\n###" << msg;
        }
        // TODO We now get all debug messages from apps here
        // show them inside ScreenPlay somewhere
    }

    void close()
    {
        if ((m_socket->state()) == QLocalSocket::UnconnectedState || (m_socket->state()) == QLocalSocket::ClosingState)
            return;

        m_socket->disconnectFromServer();
        m_socket->close();

        qDebug() << "### Destroy APPID:\t " << m_appID << " State: " << m_socket->state();
    }

    void setAppID(QString appID)
    {
        if (m_appID == appID)
            return;

        m_appID = appID;
        emit appIDChanged(m_appID);
    }

    void setMonitor(QVector<int> monitor)
    {
        if (m_monitor == monitor)
            return;

        m_monitor = monitor;
        emit monitorChanged(m_monitor);
    }

private:
    QLocalSocket* m_socket;
    QString m_appID;
    QVector<int> m_monitor;
};
