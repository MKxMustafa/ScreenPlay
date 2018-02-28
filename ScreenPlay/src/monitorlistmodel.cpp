#include "monitorlistmodel.h"

MonitorListModel::MonitorListModel(QGuiApplication* guiapp, QObject* parent)
    : QAbstractListModel(parent)
{
    loadMonitors();
    m_qGuiApplication = guiapp;
    connect(m_qGuiApplication, &QGuiApplication::screenAdded, this, &MonitorListModel::screenAdded);
    connect(m_qGuiApplication, &QGuiApplication::screenRemoved, this, &MonitorListModel::screenRemoved);
}

QHash<int, QByteArray> MonitorListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { IDRole, "monitorID" },
        { NameRole, "monitorName" },
        { SizeRole, "monitorSize" },
        { AvailableGeometryRole, "monitorAvailableGeometry" },
        { AvailableVirtualGeometryRole, "monitorAvailableVirtualGeometry" },
        { NumberRole, "monitorNumber" },
        { GeometryRole, "monitorGeometry" },
        { ModelRole, "monitorModel" },
        { ManufacturerRole, "monitorManufacturer" },
    };
    return roles;
}

QRect MonitorListModel::getAbsoluteDesktopSize()
{
    return m_monitorList.at(0).m_availableVirtualGeometry;
}

QVariant MonitorListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
}

bool MonitorListModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

int MonitorListModel::rowCount(const QModelIndex& parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_monitorList.count();
}

QVariant MonitorListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    if (row < 0 || row >= m_monitorList.count()) {
        return QVariant();
    }

    if (index.row() < rowCount())
        switch (role) {
        case IDRole:
            return m_monitorList.at(index.row()).m_id;
        case NameRole:
            return m_monitorList.at(index.row()).m_name;
        case SizeRole:
            return m_monitorList.at(index.row()).m_size;
        case AvailableGeometryRole:
            return m_monitorList.at(index.row()).m_availableGeometry;
        case AvailableVirtualGeometryRole:
            return m_monitorList.at(index.row()).m_availableVirtualGeometry;
        case NumberRole:
            return m_monitorList.at(index.row()).m_number;
        case GeometryRole:
            return m_monitorList.at(index.row()).m_geometry;
        case ModelRole:
            return m_monitorList.at(index.row()).m_model;
        case ManufacturerRole:
            return m_monitorList.at(index.row()).m_manufacturer;
        default:
            return QVariant();
        }

    return QVariant();
}

bool MonitorListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags MonitorListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable; // FIXME: Implement me!
}

bool MonitorListModel::insertRows(int row, int count, const QModelIndex& parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
}

bool MonitorListModel::removeRows(int row, int count, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
}

void MonitorListModel::loadMonitors()
{
    int offsetX = 0;
    int offsetY = 0;

    for (int i = 0; i < QApplication::screens().count(); i++) {
        QScreen* screen = QApplication::screens().at(i);
        qDebug() << screen->availableGeometry().x();
        if (screen->availableGeometry().x() < 0) {
            offsetX += (screen->availableGeometry().x() * -1);
        }
        if (screen->availableGeometry().y() < 0) {
            offsetY += (screen->availableGeometry().y() * -1);
        }
    }

    qDebug() << offsetX << offsetY;

    beginInsertRows(QModelIndex(), 0, QApplication::screens().count());
    for (int i = 0; i < QApplication::screens().count(); i++) {
        QScreen* screen = QApplication::screens().at(i);
        qDebug() << screen->availableGeometry();

        m_monitorList.append(Monitor(screen->manufacturer(),
            screen->model(),
            screen->name(),
            screen->size(),
            QRect(screen->availableGeometry().x() + offsetX, screen->availableGeometry().y() + offsetY, screen->geometry().width(), screen->geometry().height()),
            // More convenient for the user if the first monitor == 1
            i + 1,
            QRect(screen->availableVirtualGeometry().x(), screen->availableVirtualGeometry().y(), screen->availableVirtualGeometry().width(), screen->availableVirtualGeometry().height()),
            QRect(screen->geometry().x() + offsetX, screen->geometry().y() + offsetY, screen->geometry().width(), screen->geometry().height()),
            screen));
    }
    endInsertRows();

    emit monitorReloadCompleted();
}

int MonitorListModel::size()
{
    return m_monitorList.size();
}

bool MonitorListModel::getMonitorListItemAt(int position, Monitor* monitor)
{
    if (position < 0 && position > m_monitorList.size()) {
        return false;
    } else {
        *monitor = m_monitorList.at(position);
        return true;
    }
}

void MonitorListModel::screenAdded(QScreen* screen)
{
    reset();
    loadMonitors();
}

void MonitorListModel::screenRemoved(QScreen* screen)
{
    reset();
    loadMonitors();
}

void MonitorListModel::reset()
{
    beginResetModel();
    m_monitorList.clear();
    m_monitorList.squeeze();
    endResetModel();
}

Monitor::Monitor()
{
}

Monitor::Monitor(QString manufacturer, QString model, QString name, QSize size, QRect availableGeometry, int number, QRect availableVirtualGeometry, QRect geometry, QScreen* screen)
{
    m_screen = screen;

    m_name = name;
    m_size = size;
    m_geometry = geometry;
    m_availableGeometry = availableGeometry;
    m_manufacturer = manufacturer;
    m_model = model;
    m_availableVirtualGeometry = availableVirtualGeometry;
    m_number = number;
    // FIXME: Use a better way to create an id
    // because name and manufacturer are allways empty
    m_id = name + "_" + QString::number(size.width()) + "x" + QString::number(size.height()) + "_" + QString::number(availableGeometry.x()) + "x" + QString::number(availableGeometry.y());
}

void MonitorListModel::reloadMonitors()
{
    beginResetModel();
    m_monitorList.clear();
    m_monitorList.squeeze();
    endResetModel();

    loadMonitors();
}
