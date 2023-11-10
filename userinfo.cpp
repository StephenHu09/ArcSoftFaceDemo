#include "userinfo.h"

UserInfo::UserInfo(QObject *parent) : QObject(parent),
  m_name(QString()),
  m_id(QString()),
  m_photoPath(QString()),
  m_feature(QString())
{

}

UserInfo::~UserInfo()
{

}

void UserInfo::SetName(const QString &name)
{
    m_name = name;
}

void UserInfo::SetId(const QString &id)
{
    m_id = id;
}

void UserInfo::SetFeature(const QString &feature)
{
    m_feature = feature;
}

void UserInfo::SetPhotoPath(const QString &photoPath)
{
    m_photoPath = photoPath;
}

QString UserInfo::GetName()
{
    return m_name;
}

QString UserInfo::GetId()
{
    return m_id;
}

QString UserInfo::GetFeature()
{
    return m_feature;
}

QString UserInfo::GetPhotoPath()
{
    return m_photoPath;
}
