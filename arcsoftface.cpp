/*********************************************************************************
  * Description: 基于虹软(ArcSoft) SDK3.0 实现的人脸识别验证demo，基于 Qt5.12 编译调试
  * Author: hucj
  * Version: v1.0.3
  * Date: 2023-11-09
  * History:
    20231106: 基本接口验证ok，可以实现静态图片的 FaceDetect 和 FaceCompare
    20231109：实现camera 人脸识别功能，支持人脸录入和人脸验证。
**********************************************************************************/

#include "arcsoftface.h"
#include "ui_arcsoftface.h"

#include <QDebug>
#include <QFileDialog>
#include <QPainter>
#include <QPen>
#include <QDateTime>
#include "commonfunc.h"

/*
 *  https://ai.arcsoft.com.cn/ucenter/resource/build/index.html#/index
 * 通过虹软-开发者中心下载SDK，配置 APPID和 SDK_KEY，
 * 注意：Windows(X86) 和（X64）平台的 SDK_KEY 不一样，请分别配置使用。
 *
#define SDK_KEY
#define APP_ID ""
#define X86_SDK_KEY ""
#define X64_SDK_KEY ""
*/

ArcSoftFace::ArcSoftFace(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ArcSoftFace)
{
    ui->setupUi(this);

    InitFaceModule();

    InitUi();
}

ArcSoftFace::~ArcSoftFace()
{
    delete ui;

//    m_camera.reset();
}


void ArcSoftFace::InitUi()
{
    setWindowTitle(QString("ArcSoft 虹软人脸识别") + QString(" V%1").arg(APP_VERSION));
    setFixedSize(1200, 800);

    // 将 m_camera 添加到tab3页面
    m_camera = QSharedPointer<FaceCamera>(new FaceCamera(ui->tab_3));
    qDebug() << "m_camera layout size =" << m_camera->size();

}

void ArcSoftFace::InitFaceModule()
{
    m_imageFaceEngine = new ArcFaceEngine(this);

#if 1
    SdkActivate();
#endif

    //获取激活文件信息
    ASF_ActiveFileInfo activeFileInfo = { 0 };
    MRESULT res = m_imageFaceEngine->GetActiveFileInfo(activeFileInfo);
    if (res != MOK) {
        qDebug() << "ASFGetActiveFileInfo failed :" << res;
    } else {
        QDateTime startDateTime = QDateTime::fromSecsSinceEpoch(QString(activeFileInfo.startTime).toLongLong());
        qDebug() << "startDateTime =" << startDateTime.toString("yyyy-MM-dd hh:mm:ss");
        QDateTime endDateTime = QDateTime::fromSecsSinceEpoch(QString(activeFileInfo.endTime).toLongLong());
        qDebug() << "endDateTime =" << endDateTime.toString("yyyy-MM-dd hh:mm:ss");
    }

    res = m_imageFaceEngine->InitEngine(ASF_DETECT_MODE_IMAGE); // Image
    qDebug() << "IMAGE模式下初始化结果 :" << res;
}

// 首次使用需联网激活
void ArcSoftFace::SdkActivate()
{
    // APPID 和 KEY 默认从配置文件读取，或者定义宏使用: APP_ID, X86_SDK_KEY, X64_SDK_KEY
#ifdef SDK_KEY
    QString appId = APP_ID;
    QString keyX86 = X86_SDK_KEY;
    QString keyX64 = X64_SDK_KEY;
#else
    QString runDir = QApplication::applicationDirPath();
    QString configDir = runDir + "/ArcSoftFace";
    CommonFunc::CreateMultiDir(configDir);

    QString configFile = configDir + "/config.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    QString appId = settings.value("/ACTIVATE/APP_ID", "unknown").toString();
    QString keyX86 = settings.value("/ACTIVATE/X86_SDK_KEY", "unknown").toString();
    QString keyX64 = settings.value("/ACTIVATE/X64_SDK_KEY", "unknown").toString();
    // bool activateState = settings.value("/ACTIVATE/STATE", false).toBool();
#endif


    MRESULT faceRes = 0;
#ifdef WIN_X64
    faceRes = m_imageFaceEngine->ActiveSDK(appId, keyX64, "");
#else // X86
    faceRes = m_imageFaceEngine->ActiveSDK(appId, keyX86, "");
#endif
    qDebug() << "激活结果 :" << faceRes; // 0：成功， 非0：查询错误码表
}

void ArcSoftFace::LoadPictrue(QString path, QLabel *viewLab)
{
    if (viewLab == nullptr) {
        qDebug() << "view pictrue label is null";
        return;
    }

    viewLab->clear();

    if (path.isNull() || path.isEmpty()) {
        qDebug() << "load pictrue path not set";
        return;
    }

    QImage image;
    image.load(path);
    QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

    qDebug() << " rgbImage.bytesPerLine() = " << rgbImage.bytesPerLine();

    QPixmap pixmap = QPixmap::fromImage(rgbImage.scaled(viewLab->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    viewLab->setPixmap(pixmap);
    viewLab->show();
}

void ArcSoftFace::FaceDetectByImage(QString imgPath, QLabel *imgLabel)
{
    if (imgPath.isNull() || imgPath.isEmpty()) {
        return;
    }

    QImage image(imgPath);

    ASF_MultiFaceInfo multiFaces = { 0 };

    MRESULT res = m_imageFaceEngine->PreDetectFace(image, multiFaces, true);
    if (res != MOK || multiFaces.faceNum < 1) {
        qWarning() << "PreDetectFace : face detect error. res =" << res << ", faceNum =" << multiFaces.faceNum;
        return;
    }

    for (int i = 0; i < multiFaces.faceNum; ++i) {

        int left = multiFaces.faceRect[i].left;
        int top = multiFaces.faceRect[i].top;
        int right = multiFaces.faceRect[i].right;
        int bottom = multiFaces.faceRect[i].bottom;
        int faceOrient = multiFaces.faceOrient[i];

        QRect faceRect(left, top, (right - left), (bottom - top));
        qDebug() << "faceRect [" << faceRect.x() << ", " << faceRect.y() << ", "
                 << faceRect.width() << ", " << faceRect.height() << "]" << ", faceOrient =" << faceOrient;

        QPainter painter;
        painter.begin(&image);
        painter.setPen(QPen(Qt::green, 2));
        // 人脸框扩大显示
        QRect newRect = QRect(faceRect.x() - 5, faceRect.y() - 5, faceRect.width() + 10, faceRect.height() + 10);
        painter.drawRect(newRect);
        painter.end();
    }

    QPixmap pixmap = QPixmap::fromImage(image.scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imgLabel->clear();
    imgLabel->setPixmap(pixmap);
    imgLabel->show();
}


void ArcSoftFace::on_btnLoadPic1_clicked()
{
    QString filePath= QFileDialog::getOpenFileName(this, tr("open image file"),
                                                         "./" ,
                                                         "JPEG Files(*.jpg *.jpeg);;PNG Files(*.png);;BMP Files(*.bmp)");

    LoadPictrue(filePath, ui->labelPic1);
    m_pic1Path = filePath;

    ui->labelCompRet->clear();
}

void ArcSoftFace::on_btnClearPic1_clicked()
{
    ui->labelPic1->clear();
    m_pic1Path.clear();
    ui->labelCompRet->clear();
}

void ArcSoftFace::on_btnLoadPic2_clicked()
{
    QString filePath= QFileDialog::getOpenFileName(this, tr("open image file"),
                                                         "./" ,
                                                         "JPEG Files(*.jpg *.jpeg);;PNG Files(*.png);;BMP Files(*.bmp)");

    LoadPictrue(filePath, ui->labelPic2);
    m_pic2Path = filePath;

    ui->labelCompRet->clear();
}

void ArcSoftFace::on_btnClearPic2_clicked()
{
    ui->labelPic2->clear();
    m_pic2Path.clear();
    ui->labelCompRet->clear();
}

void ArcSoftFace::on_btnCompare_clicked()
{
    if (m_pic1Path.isEmpty() || m_pic2Path.isEmpty()) {
        qDebug() << "pictrue not load >>>>>";
        return;
    }

    // FD1 处理 pic1
    ASF_SingleFaceInfo SingleDetectedFaces1 = { 0 };
    ASF_FaceFeature feature1 = { 0 };
    QImage img1(m_pic1Path);

    MRESULT res1 = m_imageFaceEngine->PreDetectFace(img1, SingleDetectedFaces1, true);
    qInfo() << "PreDetectFace, res1 =" << res1;
    if (res1 == MOK) {
        res1 = m_imageFaceEngine->PreExtractFeature(img1, feature1, SingleDetectedFaces1);
    }

    // FD2 处理 pic2
    ASF_SingleFaceInfo SingleDetectedFaces2 = { 0 };
    ASF_FaceFeature feature2 = { 0 };
    QImage img2(m_pic2Path);

    MRESULT res2 = m_imageFaceEngine->PreDetectFace(img2, SingleDetectedFaces2, true);
    qInfo() << "PreDetectFace, res2 =" << res2;
    if (res2 == MOK) {
        res2 = m_imageFaceEngine->PreExtractFeature(img2, feature2, SingleDetectedFaces2);
    }

    if (res1 != MOK || res2 != MOK) {
        qWarning() << "res1 =" << res1 << ", res2 =" << res2;
        return;
    }

    // FR 人脸比对
    MFloat confidenceLevel = 0;
    MRESULT matchRes = m_imageFaceEngine->FacePairMatching(confidenceLevel, feature1, feature2);
    qDebug() << "FacePairMatch, confidenceLevel =" << confidenceLevel << ", matchRes =" << matchRes;
    ui->labelCompRet->setText(QString("人脸对比相似度: %1").arg(confidenceLevel));

    QString checkStr = QString("人脸比对相似度: %1， ").arg(confidenceLevel);
    const MFloat threshold = 0.8f;
    if (confidenceLevel >= threshold) {
        ui->labelCompRet->setText(checkStr + "判断是同一张人脸");
        ui->labelCompRet->setStyleSheet("color: green; font-size: 22px;");
    } else {
        qDebug() << "图片比对完成，不是同一张人脸";
        ui->labelCompRet->setText(checkStr + "判断不是同一张人脸");
        ui->labelCompRet->setStyleSheet("color: red;  font-size: 22px;");
    }
}

void ArcSoftFace::on_btnLoad_clicked()
{
    QString filePath= QFileDialog::getOpenFileName(this, tr("open image file"),
                                                         "./" ,
                                                         "JPEG Files(*.jpg *.jpeg);;PNG Files(*.png);;BMP Files(*.bmp)");

    LoadPictrue(filePath, ui->labelFdView);
    m_fdPicPath = filePath;

    ui->labelPicPath->setWordWrap(true);
    ui->labelPicPath->setText(filePath);
}


void ArcSoftFace::on_btnFaceDetect_clicked()
{
    FaceDetectByImage(m_fdPicPath, ui->labelFdView);
}

