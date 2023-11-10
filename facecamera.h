#ifndef FACECAMERA_H
#define FACECAMERA_H

#include <QWidget>
#include <QThread>
#include "facemodule.h"

namespace Ui {
class FaceCamera;
}

class FaceCamera : public QWidget
{
    Q_OBJECT

public:
    explicit FaceCamera(QWidget *parent = nullptr);
    ~FaceCamera();

private:
    void InitFaceModule();
    void InitFaceDB();
    void OpenCamera();

private slots:
    void HandleFaceRect(int x, int y, int width, int height);
    void HandleRecognizeRet(bool ret, const QString &des);
    void HandleRecordRet(bool ret, const QString &des);
    void HandleImageCaptured(int id, const QImage &image);

    void on_btnVerify_clicked();
    void on_btnReset_clicked();
    void on_btnClear_clicked();
    void on_btnRecord_clicked();

signals:
    void callFaceTrack(const QImage & image);
    void callFaceRecognize(const QImage & image);
    void callFaceRecord(const QImage & image, const QString &userId, const QString &userName);
    void callClearDB();

private:
    Ui::FaceCamera *ui;

    QThread m_faceThread;
    FaceModule *m_faceMod;
    QRect m_faceRect; // 浜鸿劯妗嗕綅缃
    bool m_startVerify;
    bool m_startRecord;
};

#endif // FACECAMERA_H
