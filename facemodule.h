#ifndef FACERECMODULE_H
#define FACERECMODULE_H

#include <QObject>
#include <QMutex>
#include <QRect>
#include <QImage>

#include "arcfaceengine.h"
#include "userinfo.h"

class FaceModule : public QObject
{
    Q_OBJECT
public:
    explicit FaceModule(QObject *parent = nullptr);
    ~FaceModule();

public slots:
    void FaceTrack(const QImage & image);
    void FaceRecognize(const QImage & image);
    void FaceRecord(const QImage & image, const QString &userId = QString(), const QString &userName = QString());
    void FaceDBClear();

private:
    void InitFaceEngine();
    MRESULT ExtractFeature(QImage img, ASF_FaceFeature &feature);
    void AddFaceDB(const QString &path, const QString &name, const QString &id);
    bool FaceVerify1N(ASF_FaceFeature feature, float &sim, QString &name, QString &id);

signals:
    void sigFaceRect(int x, int y, int width, int height);
    void sigRecognizeRet(bool ret, const QString &des);
    void sigRecordRet(bool ret, const QString &des);

private:
    ArcFaceEngine *m_videoFaceEngine = nullptr;

    /* 互斥锁 */
    QMutex m_lock;
    QRect m_faceRect; // 人脸框位置

    QList<UserInfo *> m_userDbList; // 保存人脸库信息
};

#endif // FACERECMODULE_H
