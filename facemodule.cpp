#include "facemodule.h"

#include <QDebug>
#include <QByteArray>
#include <QDateTime>

#include "commonfunc.h"

// --------------------- 人脸特征码转换处理 ---------------------
void FeatureToString(ASF_FaceFeature &feature, QString &str)
{
    if (feature.feature == nullptr || feature.featureSize <= 0) {
        str.clear();
        return;
    }

    QByteArray bytes(reinterpret_cast<char *>(feature.feature), feature.featureSize * sizeof(MByte));
    str = QString::fromLatin1(bytes.toBase64());
}

void StringToFeature(const QString &str, ASF_FaceFeature &feature)
{
    if (str.isEmpty()) {
        feature.feature = nullptr;
        feature.featureSize = 0;
        return;
    }

    QByteArray byte = QByteArray::fromBase64(str.toLatin1());
    feature.feature = new unsigned char[byte.size()];
    memcpy(feature.feature, byte.constData(), byte.size()); // TODO: 替换安全函数 *_s
    feature.featureSize = byte.size();
}
// --------------------- 人脸特征码转换处理 ---------------------

FaceModule::FaceModule(QObject *parent) : QObject(parent)
{
    InitFaceEngine();
}

FaceModule::~FaceModule()
{
    qDebug() << "free FaceModule";

    FaceDBClear();
}

void FaceModule::FaceTrack(const QImage &image)
{
    QMutexLocker locker(&m_lock);

    if (image.isNull() || !m_videoFaceEngine) {
        return;
    }

    ASF_SingleFaceInfo SingleDetectedFaces = { 0 };
    MRESULT res = m_videoFaceEngine->PreDetectFace(image, SingleDetectedFaces, true);
    if (res != MOK) {
        // qWarning() << "FaceTrack error, res =" << res;
    }

    int left = SingleDetectedFaces.faceRect.left;
    int top = SingleDetectedFaces.faceRect.top;
    int right = SingleDetectedFaces.faceRect.right;
    int bottom = SingleDetectedFaces.faceRect.bottom;
    // int faceOrient = SingleDetectedFaces.faceOrient;

    emit sigFaceRect(left, top, (right - left), (bottom - top));
}

void FaceModule::FaceRecognize(const QImage &image)
{
    QMutexLocker locker(&m_lock);

    // 1.提取人脸特征
    ASF_FaceFeature feature = { 0 };
    MRESULT res = ExtractFeature(image, feature);
    if (res != MOK) {
        emit sigRecordRet(false, QString("ArcSoft SDK引擎接口调用失败!"));
        return;
    }

    // 2.比较人脸库
    float maxSim = 0;
    QString matchName = QString();
    QString matchId = QString();
    bool ret = FaceVerify1N(feature, maxSim, matchName, matchId);
    if (ret) {
        QString log = QString("摄像头人脸识别成功 Success, name = %1, id = %2").arg(matchName).arg(matchId);
        qDebug() << log;
        emit sigRecognizeRet(true, log);
    } else {
        QString log = QString("摄像头人脸识别失败 failed");
        qDebug() << log;
        emit sigRecognizeRet(false, log);
    }
}

void FaceModule::FaceRecord(const QImage &image, const QString &userId, const QString &userName)
{
    QMutexLocker locker(&m_lock);

    UserInfo *tempUser = new UserInfo(this);
    tempUser->SetName(userName);
    tempUser->SetId(userId);

    // 1.提取人脸特征值
    ASF_FaceFeature feature = { 0 };
    MRESULT res = ExtractFeature(image, feature);
    if (res != MOK) {
        emit sigRecordRet(false, QString("ArcSoft SDK引擎接口调用失败!"));
        return;
    }

    // 2.与人脸库进行比对, 检测是否已录入过
    float maxSim = 0;
    QString matchName = QString();
    QString matchId = QString();
    bool ret = FaceVerify1N(feature, maxSim, matchName, matchId);
    if (ret) {
        QString log = QString("该用户人脸已录入 failed");
        qDebug() << log;
        emit sigRecordRet(false, log);
        return;
    }

    QString featrueStr = QString();
    FeatureToString(feature, featrueStr);
    tempUser->SetFeature(featrueStr);


    // 3.人脸照片保存
    QString saveDir = QApplication::applicationDirPath() + "/ArcSoftFace" + "/savephoto";
    CommonFunc::CreateMultiDir(saveDir);

    qint64 timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
    QString filename = QString("test_%1.png").arg(timestamp);
    QString photoPath = saveDir + "/" + filename;
    image.save(photoPath);
    tempUser->SetPhotoPath(photoPath);

    qDebug() << "photo path = " << photoPath;
    m_userDbList.push_back(tempUser);

    QString log = QString("用户 %1, ID %2, 人脸录入成功 Success").arg(userName).arg(userId);
    qDebug() << log;
    emit sigRecordRet(true, log);

    // TODO: 4.刷新本地数据库 sqlite
}

void FaceModule::FaceDBClear()
{
    for (UserInfo* userPtr : m_userDbList) {
        if (userPtr) {
            delete userPtr;
        }
    }
    m_userDbList.clear(); // 清空列表，以确保没有悬挂指针
}

void FaceModule::InitFaceEngine()
{
    m_videoFaceEngine = new ArcFaceEngine(this);

    auto faceRes = m_videoFaceEngine->InitEngine(ASF_DETECT_MODE_VIDEO); // Video
    qDebug() << "InitFaceEngine >>> VIDEO模式下初始化结果 :" << faceRes;
}

MRESULT FaceModule::ExtractFeature(QImage img, ASF_FaceFeature &feature)
{
    if (!m_videoFaceEngine) {
        return -1;
    }

    QImage faceImg = img.copy();
    ASF_SingleFaceInfo SingleDetectedFaces = { 0 };
    MRESULT res = m_videoFaceEngine->PreDetectFace(faceImg, SingleDetectedFaces, true);
    qInfo() << "PreDetectFace, res =" << res;
    if (res == MOK) {
        res = m_videoFaceEngine->PreExtractFeature(faceImg, feature, SingleDetectedFaces);
    }
    qInfo() << "PreExtractFeature, res =" << res;

    return res;
}

void FaceModule::AddFaceDB(const QString &path, const QString &name, const QString &id)
{

}

bool FaceModule::FaceVerify1N(ASF_FaceFeature feature, float &sim, QString &name, QString &id)
{
    float maxSim = 0;

    for (UserInfo *userPtr : m_userDbList) {
        if (!userPtr) {
            continue;
        }

        QString tempFeature = userPtr->GetFeature();
        ASF_FaceFeature featureDb;
        StringToFeature(tempFeature, featureDb);

        MFloat confidenceLevel = 0;
        MRESULT matchRes = m_videoFaceEngine->FacePairMatching(confidenceLevel, feature, featureDb);
        qDebug() << "FaceVerify1N, confidenceLevel =" << confidenceLevel << ", matchRes =" << matchRes;
        if (confidenceLevel > maxSim) {
            maxSim = confidenceLevel;
            name = userPtr->GetName();
            id = userPtr->GetId();
        }
    }

    sim = maxSim;

    const float threshold = 0.8f; // 判定阈值
    if (maxSim < threshold) {
        return false;
    }

    return true;
}
