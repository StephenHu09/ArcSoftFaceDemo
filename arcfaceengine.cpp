#include "arcfaceengine.h"
#include <QImage>
#include <QDebug>

#define NSCALE 32
#define FACENUM 50

ArcFaceEngine::ArcFaceEngine(QObject *parent) : QObject(parent)
{

}

ArcFaceEngine::~ArcFaceEngine()
{
    qDebug() << "===== ArcFaceEngine free =====";
    UnInitEngine();
}

MRESULT ArcFaceEngine::ActiveSDK(const QString &appID, const QString &sdkKey, const QString &activeKey)
{
#ifdef PRO
    MRESULT res = ASFOnlineActivation(appID.toUtf8().data(), sdkKey.toUtf8().data(), activeKey.toUtf8().data());
#else
    Q_UNUSED(activeKey);
    MRESULT res = ASFOnlineActivation(appID.toUtf8().data(), sdkKey.toUtf8().data());
#endif

    if (res != MOK && res != MERR_ASF_ALREADY_ACTIVATED)
        return res;

    return MOK;
}

MRESULT ArcFaceEngine::GetActiveFileInfo(ASF_ActiveFileInfo &activeFileInfo)
{
    MRESULT res = ASFGetActiveFileInfo(&activeFileInfo);
    return res;
}

MRESULT ArcFaceEngine::InitEngine(ASF_DetectMode detectMode)
{
    m_hEngine = NULL;
    MInt32 mask = 0;

    if (ASF_DETECT_MODE_IMAGE == detectMode)
    {
        mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS | ASF_IR_LIVENESS;
    }
    else
    {
        mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_LIVENESS | ASF_IR_LIVENESS;
    }

    MRESULT res = ASFInitEngine(detectMode, ASF_OP_0_ONLY, NSCALE, FACENUM, mask, &m_hEngine);
    return res;
}

MRESULT ArcFaceEngine::UnInitEngine()
{
    // 销毁引擎
    return ASFUninitEngine(m_hEngine);
}

MRESULT ArcFaceEngine::PreDetectFace(const QImage &image, ASF_SingleFaceInfo &faceRect, bool isRGB)
{
    if (image.isNull()) {
        qDebug() << "PreDetectFace : image is NULL";
        return -1;
    }

    QImage cutImg = QImage();
    MRESULT res = MOK;
    ASF_MultiFaceInfo detectedFaces = { 0 }; // 人脸检测

    if (isRGB) {
        QRect subRect(0, 0, image.width() - (image.width() % 4), image.height()); // 宽度按4的倍数对齐
        cutImg = image.copy(subRect).convertToFormat(QImage::Format_RGB888).rgbSwapped();
        ASVLOFFSCREEN offscreen = { 0 };
        ColorSpaceConversion(cutImg, ASVL_PAF_RGB24_B8G8R8, offscreen);
        res = ASFDetectFacesEx(m_hEngine, &offscreen, &detectedFaces);
        // qDebug() << "ASFDetectFacesEx : res =" << res << ", faceNum ="<< detectedFaces.faceNum;
    } else  {  //IR图像
        QImage grayImg = image.convertToFormat(QImage::Format_Grayscale8);
        QRect subRect(0, 0, grayImg.width() - (grayImg.width() % 4), grayImg.height()); // 宽度按4的倍数对齐
        cutImg = grayImg.copy(subRect);

        ASVLOFFSCREEN offscreen = { 0 };
        ColorSpaceConversion(cutImg, ASVL_PAF_GRAY, offscreen);
        res = ASFDetectFacesEx(m_hEngine, &offscreen, &detectedFaces);
    }

    if (res != MOK || detectedFaces.faceNum < 1) {
        return -1;
    }

    int max = 0;
    int maxArea = 0;

    for (int i = 0; i < detectedFaces.faceNum; i++)
    {
        if (detectedFaces.faceRect[i].left < 0)
            detectedFaces.faceRect[i].left = 10;
        if (detectedFaces.faceRect[i].top < 0)
            detectedFaces.faceRect[i].top = 10;
        if (detectedFaces.faceRect[i].right < 0 || detectedFaces.faceRect[i].right > cutImg.width())
            detectedFaces.faceRect[i].right = cutImg.width() - 10;
        if (detectedFaces.faceRect[i].bottom < 0 || detectedFaces.faceRect[i].bottom > cutImg.height())
            detectedFaces.faceRect[i].bottom = cutImg.height() - 10;

        if ((detectedFaces.faceRect[i].right - detectedFaces.faceRect[i].left)*
            (detectedFaces.faceRect[i].bottom - detectedFaces.faceRect[i].top) > maxArea)
        {
            max = i;
            maxArea = (detectedFaces.faceRect[i].right - detectedFaces.faceRect[i].left)*
                (detectedFaces.faceRect[i].bottom - detectedFaces.faceRect[i].top);
        }
    }

    faceRect.faceRect.left = detectedFaces.faceRect[max].left;
    faceRect.faceRect.top = detectedFaces.faceRect[max].top;
    faceRect.faceRect.right = detectedFaces.faceRect[max].right;
    faceRect.faceRect.bottom = detectedFaces.faceRect[max].bottom;
    faceRect.faceOrient = detectedFaces.faceOrient[max];

    return res;
}

MRESULT ArcFaceEngine::PreDetectFace(const QImage &image, ASF_MultiFaceInfo &multiFaceList, bool isRGB)
{
    if (image.isNull()) {
        qWarning() << "PreDetectFace : image is NULL";
        return -1;
    }

    QImage cutImg = QImage();
    MRESULT res = MOK;
    // ASF_MultiFaceInfo detectedFaces = { 0 }; // 人脸检测

    if (isRGB) {
        QRect subRect(0, 0, image.width() - (image.width() % 4), image.height()); // 宽度按4的倍数对齐
        cutImg = image.copy(subRect).convertToFormat(QImage::Format_RGB888).rgbSwapped();
        ASVLOFFSCREEN offscreen = { 0 };
        ColorSpaceConversion(cutImg, ASVL_PAF_RGB24_B8G8R8, offscreen);
        res = ASFDetectFacesEx(m_hEngine, &offscreen, &multiFaceList);
        qDebug() << "ASFDetectFacesEx : res =" << res << ", faceNum ="<< multiFaceList.faceNum;
    } else  {  //IR图像
        QImage grayImg = image.convertToFormat(QImage::Format_Grayscale8);
        QRect subRect(0, 0, grayImg.width() - (grayImg.width() % 4), grayImg.height()); // 宽度按4的倍数对齐
        cutImg = grayImg.copy(subRect);

        ASVLOFFSCREEN offscreen = { 0 };
        ColorSpaceConversion(cutImg, ASVL_PAF_GRAY, offscreen);
        res = ASFDetectFacesEx(m_hEngine, &offscreen, &multiFaceList);
    }

    return res;
}

MRESULT ArcFaceEngine::PreExtractFeature(QImage &image, ASF_FaceFeature &feature, ASF_SingleFaceInfo &faceRect)
{
    if (image.isNull()) {
        return -1;
    }

    QRect subRect(0, 0, image.width() - (image.width() % 4), image.height()); // 宽度按4的倍数对齐
    QImage cutImg = image.copy(subRect).convertToFormat(QImage::Format_RGB888).rgbSwapped();
    if (cutImg.isNull()) {
        qDebug() << "cutImg is NULL";
        return -1;
    }

    MRESULT res = MOK;
    ASF_FaceFeature detectFaceFeature = { 0 }; // 特征值
    ASVLOFFSCREEN offscreen = { 0 };
    ColorSpaceConversion(cutImg, ASVL_PAF_RGB24_B8G8R8, offscreen);

    res = ASFFaceFeatureExtractEx(m_hEngine, &offscreen, &faceRect, &detectFaceFeature);
    if (MOK != res) {
        return res;
    }

    if (!feature.feature) {
        qDebug() << "feature is NULL, malloc space";
        feature.featureSize = detectFaceFeature.featureSize;
        feature.feature = (MByte *)malloc(feature.featureSize);
        // return -1;
    }

    memset(feature.feature, 0, detectFaceFeature.featureSize);
    memcpy(feature.feature, detectFaceFeature.feature, detectFaceFeature.featureSize);

    return res;
}

MRESULT ArcFaceEngine::FacePairMatching(MFloat &confidenceLevel, ASF_FaceFeature feature1, ASF_FaceFeature feature2, ASF_CompareModel compareModel)
{
    MRESULT res = ASFFaceFeatureCompare(m_hEngine, &feature1, &feature2, &confidenceLevel, compareModel);
    return res;
}

MRESULT ArcFaceEngine::SetLivenessThreshold(MFloat rgbLiveThreshold, MFloat irLiveThreshold)
{
    ASF_LivenessThreshold threshold = { 0 };

    threshold.thresholdmodel_BGR = rgbLiveThreshold;
    threshold.thresholdmodel_IR = irLiveThreshold;

    MRESULT res = ASFSetLivenessParam(m_hEngine, &threshold);
    return res;
}

MRESULT ArcFaceEngine::FaceASFProcess(ASF_MultiFaceInfo detectedFaces, QImage &img, ASF_AgeInfo &ageInfo,
                                      ASF_GenderInfo &genderInfo, ASF_Face3DAngle &angleInfo, ASF_LivenessInfo &liveNessInfo)
{
    if (img.isNull()) {
        return -1;
    }

    MInt32 lastMask = ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS;
    QRect subRect(0, 0, img.width() - (img.width() % 4), img.height()); // 宽度按4的倍数对齐
    QImage cutImg = img.copy(subRect).convertToFormat(QImage::Format_RGB888).rgbSwapped();
    if (cutImg.isNull()) {
        return -1;
    }

    ASVLOFFSCREEN offscreen = { 0 };
    ColorSpaceConversion(cutImg, ASVL_PAF_RGB24_B8G8R8, offscreen);

    MRESULT res = ASFProcessEx(m_hEngine, &offscreen, &detectedFaces, lastMask);
    res |= ASFGetAge(m_hEngine, &ageInfo);
    res |= ASFGetGender(m_hEngine, &genderInfo);
    res |= ASFGetFace3DAngle(m_hEngine, &angleInfo);
    res |= ASFGetLivenessScore(m_hEngine, &liveNessInfo);

    return res;
}

MRESULT ArcFaceEngine::FaceASFProcess_IR(ASF_MultiFaceInfo detectedFaces, QImage &img, ASF_LivenessInfo &irLiveNessInfo)
{
    if (img.isNull()) {
        return -1;
    }

    QImage grayImg = img.convertToFormat(QImage::Format_Grayscale8);

    MInt32 lastMask = ASF_IR_LIVENESS;
    QRect subRect(0, 0, grayImg.width() - (grayImg.width() % 4), grayImg.height()); // 宽度按4的倍数对齐
    QImage cutGrayImg = grayImg.copy(subRect);
    if (cutGrayImg.isNull()) {
        return -1;
    }

    ASVLOFFSCREEN offscreen = { 0 };
    ColorSpaceConversion(cutGrayImg, ASVL_PAF_GRAY, offscreen);

    MRESULT res = ASFProcessEx_IR(m_hEngine, &offscreen, &detectedFaces, lastMask);
    res |= ASFGetLivenessScore_IR(m_hEngine, &irLiveNessInfo);

    return res;
}

const ASF_VERSION ArcFaceEngine::GetVersion()
{
    const ASF_VERSION version = ASFGetVersion();
    return version;
}

void PicCutOut(const QImage &srcImg, QImage &dstImg, QRect cutRect)
{
    if (cutRect.isNull() || cutRect.isEmpty()) {
        return; // 无效的裁剪区域，不执行任何操作
    }

    // 使用 QImage::copy() 复制裁剪区域
    dstImg = srcImg.copy(cutRect);
}

int ColorSpaceConversion(QImage &image, MInt32 format, ASVLOFFSCREEN &offscreen)
{
    switch (format)
    {
    case ASVL_PAF_RGB24_B8G8R8:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = image.width();
        offscreen.i32Height = image.height();
        offscreen.pi32Pitch[0] = image.bytesPerLine();
        offscreen.ppu8Plane[0] = (MUInt8*)image.bits();
        break;
    case ASVL_PAF_GRAY:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = image.width();
        offscreen.i32Height = image.height();
        offscreen.pi32Pitch[0] = image.bytesPerLine();
        offscreen.ppu8Plane[0] = (MUInt8*)image.bits();
        break;
    default:
        return 0;
    }
    return 1;
}
