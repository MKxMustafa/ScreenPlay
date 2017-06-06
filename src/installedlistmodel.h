#ifndef INSTALLEDLISTMODEL_H
#define INSTALLEDLISTMODEL_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QtConcurrent/QtConcurrent>

class ScreenPlayFile;

class InstalledListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit InstalledListModel(QObject* parent = 0);

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void append(const QJsonObject, const QString);
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadScreens();
    Q_INVOKABLE QVariantMap get(QString folderId);
    Q_INVOKABLE void setScreenVisibleFromQml(bool visible);
    Q_INVOKABLE void setScreenToVideoFromQml(QString absolutePath);
    Q_PROPERTY(QString _screensPath READ name CONSTANT)

    enum InstalledRole {
        TitleRole,
        PreviewRole,
        FolderIdRole,
        FileIdRole,
    };
    Q_ENUM(InstalledRole)

    QString name() const
    {
        return _screensPath;
    }

signals:
    void setScreenVisible(bool visible);
    void setScreenToVideo(QString absolutePath);

private:
    QList<ScreenPlayFile> _screenPlayFiles;
    QString _screensPath;
};

class ScreenPlayFile {

public:
    ScreenPlayFile();
    ScreenPlayFile(QJsonObject obj, QString folderName);

    QVariant _description;
    QVariant _file;
    QVariant _preview;
    QVariant _title;
    QString _folderId;
    QUrl _absolutePath;

    QVariantList _tags; //TODO: Implement me!
};

#endif // INSTALLEDLISTMODEL_H