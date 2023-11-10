#ifndef ARCSOFTFACE_H
#define ARCSOFTFACE_H

#include <QMainWindow>
#include <QLabel>

#include "arcfaceengine.h"
#include "facecamera.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ArcSoftFace; }
QT_END_NAMESPACE

class ArcSoftFace : public QMainWindow
{
    Q_OBJECT

public:
    ArcSoftFace(QWidget *parent = nullptr);
    ~ArcSoftFace();

private:
    void InitUi();

    void InitFaceModule();

    void SdkActivate();

    void LoadPictrue(QString path, QLabel *viewLab);

    void FaceDetectByImage(QString imgPath, QLabel *imgLabel);


private slots:
    void on_btnLoad_clicked();

    void on_btnFaceDetect_clicked();

    void on_btnLoadPic1_clicked();

    void on_btnClearPic1_clicked();

    void on_btnLoadPic2_clicked();

    void on_btnClearPic2_clicked();

    void on_btnCompare_clicked();

private:
    Ui::ArcSoftFace *ui;

    QString m_fdPicPath;

    ArcFaceEngine *m_imageFaceEngine;
    ArcFaceEngine *m_videoFaceEngine;

    QString m_pic1Path;
    QString m_pic2Path;

    QSharedPointer<FaceCamera> m_camera;

};
#endif // ARCSOFTFACE_H
