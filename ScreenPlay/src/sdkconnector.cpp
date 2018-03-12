#include "sdkconnector.h"

#include <QJsonDocument>
#include <QJsonObject>

SDKConnector::SDKConnector(QObject* parent)
    : QObject(parent)
{
    m_server = QSharedPointer<QLocalServer>(new QLocalServer(this));
    connect(m_server.data(), &QLocalServer::newConnection, this, &SDKConnector::newConnection);

    if (!m_server.data()->listen("ScreenPlay")) {
        //TODO
    }
}

void SDKConnector::newConnection()
{
    m_clients.append(QSharedPointer<SDKConnection>(new SDKConnection(m_server.data()->nextPendingConnection())));
}

void SDKConnector::closeAllWallpapers()
{
    for (int i = 0; i < m_clients.size(); ++i) {
        m_clients.at(i)->close();
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

void SDKConnection::setSocket(QLocalSocket* socket)
{
    m_socket = socket;
}

QLocalSocket* SDKConnection::socket() const
{
    return m_socket;
}
