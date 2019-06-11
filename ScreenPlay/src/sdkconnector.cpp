#include "sdkconnector.h"

#include <QJsonDocument>
#include <QJsonObject>
namespace ScreenPlay {
SDKConnector::SDKConnector(QObject* parent)
    : QObject(parent)
{

    m_server = QSharedPointer<QLocalServer>(new QLocalServer(this));
    connect(m_server.data(), &QLocalServer::newConnection, this, &SDKConnector::newConnection);
    if (!m_server.data()->listen("ScreenPlay")) {
        //TODO what to do if no ScreenPlay local socket can be open
    }
}

void SDKConnector::readyRead()
{
}

void SDKConnector::newConnection()
{
    m_clients.append(QSharedPointer<SDKConnection>(new SDKConnection(m_server.data()->nextPendingConnection())));
}

void SDKConnector::closeAllWallpapers()
{
    for (int i = 0; i < m_clients.size(); ++i) {
        m_clients.at(i)->close();
        m_clients.clear();
        m_clients.squeeze();
    }
}

void SDKConnector::closeWallpapersAt(int at)
{
    for (const QSharedPointer<SDKConnection>& refSDKConnection : m_clients) {
        refSDKConnection->close();
        if (!refSDKConnection->monitor().empty()) {
            // problem here !!
            if (refSDKConnection->monitor().at(0) == at) {
                refSDKConnection->close();
                qDebug() << "Wall Closed...!";
            } else {
                qDebug() << "COULD NOT CLOSE!";
            }
        } else {
            qDebug() << "no wp window ";
        }
    }
}

void SDKConnector::setWallpaperValue(QString appID, QString key, QString value)
{

    for (int i = 0; i < m_clients.count(); ++i) {
        if (m_clients.at(i).data()->appID() == appID) {
            QJsonObject obj;
            obj.insert(key, QJsonValue(value));

            QByteArray send = QJsonDocument(obj).toJson();
            m_clients.at(i).data()->socket()->write(send);
            m_clients.at(i).data()->socket()->waitForBytesWritten();
        }
    }
}

void SDKConnector::setSceneValue(QString appID, QString key, QString value)
{
    for (int i = 0; i < m_clients.count(); ++i) {
        if (m_clients.at(i).data()->appID() == appID) {
            QJsonObject obj;
            obj.insert("type", QJsonValue("qmlScene"));
            obj.insert(key, QJsonValue(value));

            QByteArray send = QJsonDocument(obj).toJson();
            m_clients.at(i).data()->socket()->write(send);
            m_clients.at(i).data()->socket()->waitForBytesWritten();
        }
    }
}

QLocalSocket* SDKConnection::socket() const
{
    return m_socket;
}
}
