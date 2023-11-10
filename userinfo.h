#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>

class UserInfo : public QObject
{
    Q_OBJECT
public:
    explicit UserInfo(QObject *parent = nullptr);
    ~UserInfo();

    void SetName(const QString &name);
    void SetId(const QString &id);
    void SetFeature(const QString &feature);
    void SetPhotoPath(const QString &photoPath);

    QString GetName();
    QString GetId();
    QString GetFeature();
    QString GetPhotoPath();

signals:


private:
    QString m_name;
    QString m_id;
    QString m_photoPath;
    QString m_feature;
};

#endif // USERINFO_H
