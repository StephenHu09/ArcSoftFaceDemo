#ifndef ARCFACEENGINE_H
#define ARCFACEENGINE_H

#include <QObject>

#include "merror.h"
#include "amcomdef.h"

//#define PRO

#ifdef PRO
#include "pro/arcsoft_face_sdk.h"
#else
#include "free/arcsoft_face_sdk.h"
#endif


class ArcFaceEngine : public QObject
{
    Q_OBJECT
public:
    explicit ArcFaceEngine(QObject *parent = nullptr);
    ~ArcFaceEngine();

    // 激活
    MRESULT ActiveSDK(const QString &appID, const QString &sdkKey, const QString &activeKey = QString());
    // 获取激活文件信息（可以获取到有效期）
    MRESULT GetActiveFileInfo(ASF_ActiveFileInfo& activeFileInfo);

    // 初始化引擎
    MRESULT InitEngine(ASF_DetectMode detectMode);
    // 释放引擎
    MRESULT UnInitEngine();
    // 人脸检测
    MRESULT PreDetectFace(const QImage &image, ASF_SingleFaceInfo &faceRect, bool isRGB);
    MRESULT PreDetectFace(const QImage &image, ASF_MultiFaceInfo &multiFaceList, bool isRGB);
    // 人脸特征提取
    MRESULT PreExtractFeature(QImage &image, ASF_FaceFeature &feature, ASF_SingleFaceInfo &faceRect);
    // 人脸比对
    MRESULT FacePairMatching(MFloat &confidenceLevel, ASF_FaceFeature feature1, ASF_FaceFeature feature2,
                             ASF_CompareModel compareModel = ASF_LIFE_PHOTO);
    // 设置活体阈值
    MRESULT SetLivenessThreshold(MFloat	rgbLiveThreshold, MFloat irLiveThreshold);
    // RGB图像人脸属性检测
    MRESULT FaceASFProcess(ASF_MultiFaceInfo detectedFaces, QImage &img, ASF_AgeInfo &ageInfo,
                           ASF_GenderInfo &genderInfo, ASF_Face3DAngle &angleInfo, ASF_LivenessInfo& liveNessInfo);
    // IR活体检测
    MRESULT FaceASFProcess_IR(ASF_MultiFaceInfo detectedFaces, QImage &img, ASF_LivenessInfo& irLiveNessInfo);

    // 获取版本信息
    const ASF_VERSION GetVersion();

signals:


private:
    MHandle m_hEngine;
};

// 图片裁剪
void PicCutOut(const QImage &srcImg, QImage &dstImg, QRect cutRect);
// 颜色空间转换
int ColorSpaceConversion(QImage &image, MInt32 format, ASVLOFFSCREEN &offscreen);


#endif // ARCFACEENGINE_H
