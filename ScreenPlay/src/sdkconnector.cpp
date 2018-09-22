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
        qDebug() << m_clients.data()->get()->appID();
        m_clients.at(i)->close();
        m_clients.clear();
        m_clients.squeeze();
    }
}

void SDKConnector::closeWallpapersAt(int at)
{
    for (int i = 0; i < m_clients.size(); ++i) {
        if (m_clients.at(i).data()->monitor().size() > 0) {
            if (m_clients.at(i).data()->monitor().at(0) == at) {
                m_clients.at(i).data()->close();
            }
        } else {
            qDebug() << "no wp window";
        }
    }
}

void SDKConnector::setWallpaperValue(QString appID, QString key, QString value)
{

    for (int i = 0; i < m_clients.count(); ++i) {
        qDebug() << appID << " " << m_clients.at(i).data()->appID() << " " << m_clients.count() << " " << key << " " << value;
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
