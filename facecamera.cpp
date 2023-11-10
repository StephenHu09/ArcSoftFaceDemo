#include "facecamera.h"
#include "ui_facecamera.h"

#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QTimer>
#include <QPainter>
#include <QDebug>
#include <QMessageBox>

FaceCamera::FaceCamera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FaceCamera)
{
    ui->setupUi(this);

    InitFaceModule();
    OpenCamera();
}

FaceCamera::~FaceCamera()
{
    qDebug() << "free FaceCamera";

    delete ui;

    m_faceThread.quit();
    m_faceThread.wait();
}

void FaceCamera::InitFaceModule()
{
    m_faceMod = new FaceModule;
    m_faceMod->moveToThread(&m_faceThread); // sdk算法集中在线程处理

    connect(&m_faceThread, &QThread::finished, m_faceMod, &FaceModule::deleteLater);
    connect(&m_faceThread, &QThread::finished, &m_faceThread, &QThread::deleteLater);

    connect(this, &FaceCamera::callFaceTrack, m_faceMod, &FaceModule::FaceTrack);
    connect(this, &FaceCamera::callFaceRecognize, m_faceMod, &FaceModule::FaceRecognize);
    connect(this, &FaceCamera::callFaceRecord, m_faceMod, &FaceModule::FaceRecord);
    connect(this, &FaceCamera::callClearDB, m_faceMod, &FaceModule::FaceDBClear);

    connect(m_faceMod, &FaceModule::sigFaceRect, this, &FaceCamera::HandleFaceRect);
    connect(m_faceMod, &FaceModule::sigRecognizeRet, this, &FaceCamera::HandleRecognizeRet);
    connect(m_faceMod, &FaceModule::sigRecordRet, this, &FaceCamera::HandleRecordRet);

    m_faceRect = QRect(0, 0, 0, 0);
    m_startVerify = false;
    m_startRecord = false;

    m_faceThread.start();
}

void FaceCamera::InitFaceDB()
{
    // TODO: 使用*.db数据库文件保存人脸注册信息
}

void FaceCamera::OpenCamera()
{
    const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : availableCameras) {
        qDebug() << "camera description :" << cameraInfo.description();
    }

    // 创建QCamera对象
    QCameraInfo cameraInfo = QCameraInfo::defaultCamera();
    QCamera *camera = new QCamera(cameraInfo, this);

    QCameraViewfinderSettings set;
    set.setResolution(640, 480);
    camera->setViewfinderSettings(set);

    QCameraImageCapture *imageCapture = new QCameraImageCapture(camera);
    // 必须设置, 否则每一帧图片都会存盘保存, 默认为系统‘图片’目录下
    imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    camera->start();

    // 查看摄像头支持的分辨率尺寸
    // qDebug() << "camera support size :" << camera->supportedViewfinderResolutions();

    // 连接信号 captureImageReady，当图像被捕获时进行处理
    connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &FaceCamera::HandleImageCaptured);

    // 捕获图像
    QTimer *camTimer = new QTimer;
    connect(camTimer, &QTimer::timeout, [=]() {
        imageCapture->capture();
    });

    camTimer->setInterval(20); // 预览基本流畅
    camTimer->start();
}

void FaceCamera::HandleFaceRect(int x, int y, int width, int height)
{
    m_faceRect = QRect(x, y, width, height);
}

void FaceCamera::HandleRecognizeRet(bool ret, const QString &des)
{
    ui->logInfo->setWordWrap(true);

    if (ret) {
        ui->logInfo->setText(des);
        ui->logInfo->setStyleSheet("color: green; font-size: 22px;");
    } else {
        ui->logInfo->setText(des);
        ui->logInfo->setStyleSheet("color: red; font-size: 22px;");
    }
}

void FaceCamera::HandleRecordRet(bool ret, const QString &des)
{
    ui->logInfo->setWordWrap(true);

    if (ret) {
        ui->logInfo->setText(des);
        ui->logInfo->setStyleSheet("color: blue; font-size: 22px;");
    } else {
        ui->logInfo->setText(des);
        ui->logInfo->setStyleSheet("color: red; font-size: 22px;");
    }
}

void FaceCamera::HandleImageCaptured(int id, const QImage &image)
{
    // 镜像翻转 (笔记本前置camera需要做镜像翻转)
    QImage flippedImage = image.mirrored(true, false);

    if(!m_faceThread.isRunning()) {
        m_faceThread.start();
    }

    if (m_startVerify) {
        m_startVerify = false;
        emit callFaceRecognize(flippedImage);
    }

    if (m_startRecord){
        m_startRecord = false;
        emit callFaceRecord(flippedImage, ui->userId->text(), ui->userName->text());
    }

    emit callFaceTrack(flippedImage);

    if (m_faceRect.width() > 0 && m_faceRect.height() > 0) {
        // 绘制人脸框
        QPainter painter;
        painter.begin(&flippedImage);
        painter.setPen(QPen(Qt::green, 2)); // 线宽
        painter.drawRect(m_faceRect);
        painter.end();
    }

    // 在 UI 上显示图像
    QImage showImage = flippedImage.scaled(ui->labelPreview->size(),
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
    ui->labelPreview->setPixmap(QPixmap::fromImage(showImage));
}

void FaceCamera::on_btnVerify_clicked()
{
    qDebug() << "Verify 开始人脸验证 >>>>>";
    m_startVerify = true;
}

void FaceCamera::on_btnReset_clicked()
{
    ui->logInfo->clear();
}

void FaceCamera::on_btnClear_clicked()
{
    ui->logInfo->clear();
    ui->userName->clear();
    ui->userId->clear();

    // emit callClearDB();
}

void FaceCamera::on_btnRecord_clicked()
{
    qDebug() << "Record 开始人脸录入 >>>>>";

    if (ui->userId->text().isEmpty() || ui->userName->text().isEmpty()) {
        int result = QMessageBox::information(nullptr, "提示", "人员信息缺失！\n请输入'用户名'和'用户ID'。", QMessageBox::Ok);
        if (result == QMessageBox::Ok) {}
        return;
    }

    m_startRecord = true;
}

