#pragma once

#include <QString>
#include <QVariant>

/*!
    \class Project Settings List Item
    \brief List Item for Project Settings List Model. See Project Settings List Model for more informations

*/

class ProjectSettingsListItemBool;
class ProjectSettingsListItemSlider;
class ProjectSettingsListItemFile;

class ProjectSettingsListItem
{

public:
    explicit ProjectSettingsListItem(QString name, bool isHeadline, QVariant value);
    ProjectSettingsListItem();
    ~ProjectSettingsListItem();
    QString m_name;
    bool m_isHeadline;
    QVariant m_value;
};
