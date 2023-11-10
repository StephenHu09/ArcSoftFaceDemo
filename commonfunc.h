#ifndef COMMONFUNC_H
#define COMMONFUNC_H

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QRegExp>
#include <QSettings>
#include <QStringLiteral>
#include <QTime>
#include <QRandomGenerator>
#include <QDebug>
#include <QStandardPaths>

static const QString APP_DEF_DIR = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/ArcSoftFace";

static const QString APP_RUN_DIR = QApplication::applicationDirPath() + "/ArcSoftFace";

/**
 * @brief 公共接口类, 提供一些共用接口
 */
class CommonFunc: public QObject
{
public:
    CommonFunc() = delete;
    CommonFunc(const CommonFunc&) = delete;
    CommonFunc &operator=(const CommonFunc&) = delete;

    // 加载样式表
    static void LoadStyleSheet(QWidget *obj, const QString &qssFile)
    {
        if (obj == 0) {
            return;
        }

        QFile file(qssFile);
        if (file.exists()) {
            if (file.open(QFile::ReadOnly) == false) {
                qDebug() << "qss file: " << qssFile << " open failed";
            }
            QString styleSheet = QLatin1String(file.readAll());
            obj->setStyleSheet(styleSheet);
            file.close();
        } else {
            qDebug() << "qss file is not exist: " << qssFile;
        }
    }



    // 创建多级目录
    static QString CreateMultiDir(const QString &path)
    {
        QDir dir(path);
        if (dir.exists(path)) {
            return path;
        }

        // 根目录不存在 返回空
        if (!path.contains('/')) {
            // QLOG_DEBUG() << "Root dir is not exist: " << path;
            return QString();
        }

        QString parentDir = CreateMultiDir(path.left(path.lastIndexOf('/')));
        QString dirName = path.mid(path.lastIndexOf('/') + 1);
        QDir parentPath(parentDir);
        if (!dirName.isEmpty() ) {
            parentPath.mkpath(dirName);
        }

        return (parentDir + "/" + dirName);
    }

    // 判断目录是否为空
    static bool IsDirectoryEmpty(const QString &path) {
        QDir directory(path);
        QStringList entries = directory.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
        return entries.isEmpty();
    }


    // 目录拷贝
    static bool CopyDir(const QString &sourcePath, const QString &destPath) {
        QDir sourceDir(sourcePath);
        QDir destDir(destPath);

        if (sourcePath.compare(destPath)) {
            qInfo() << "Source directory is same with dest directory.";
            return true;
        }

        if (!sourceDir.exists()) {
            qWarning() << "Source directory does not exist.";
            return false;
        }

        if (!destDir.exists()) {
            if (!destDir.mkpath(".")) {
                qWarning() << "Failed to create destination directory.";
                return false;
            }
        }

        QFileInfoList files = sourceDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

        foreach (QFileInfo fileInfo, files) {
            QString sourceFilePath = fileInfo.absoluteFilePath();
            QString destinationFilePath = destPath + QDir::separator() + fileInfo.fileName();

            // 存在同名文件，跳过
            if (QFile::exists(destinationFilePath)) {
                qDebug() << "Skipping file (already exists):" << destinationFilePath;
                continue;
            }
#if 0
            // 存在同名文件，先删除
            if (QFile::exists(destinationFilePath)) {
                if (!QFile::remove(destinationFilePath)) {
                    qWarning() << "Failed to remove existing file:" << destinationFilePath;
                    return false;
                }
            }
#endif
            if (!QFile::copy(sourceFilePath, destinationFilePath)) {
                qWarning() << "Failed to copy file:" << sourceFilePath;
                return false;
            }
        }

        QFileInfoList subDirs = sourceDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach (QFileInfo subDirInfo, subDirs) {
            QString sourceSubDirPath = subDirInfo.absoluteFilePath();
            QString destinationSubDirPath = destPath + QDir::separator() + subDirInfo.fileName();

            if (!CopyDir(sourceSubDirPath, destinationSubDirPath)) {
                return false;
            }
        }

        return true;
    }


    // 目录搬移(剪切)
    static bool MoveDir(const QString &sourcePath, const QString &destPath) {
        QDir sourceDir(sourcePath);
        QDir destDir(destPath);

        if (!sourceDir.exists()) {
            qWarning() << "Source directory does not exist.";
            return false;
        }

        if (!destDir.exists()) {
            if (!destDir.mkpath(".")) {
                qWarning() << "Failed to create destination directory.";
                return false;
            }
        }

        QFileInfoList files = sourceDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

        foreach (QFileInfo fileInfo, files) {
            QString sourceFilePath = fileInfo.absoluteFilePath();
            QString destinationFilePath = destPath + QDir::separator() + fileInfo.fileName();

            if (!QFile::rename(sourceFilePath, destinationFilePath)) {
                qWarning() << "Failed to move file:" << sourceFilePath;
                return false;
            }
        }

        return true;
    }

    // 生成随机数字(测试用)
    static int GenerateRandomInt(int max, int min = 0)
    {
        if (max <= min) {
            return 0;
        }

        qsrand(QTime::currentTime().msec());
        int random = min + qrand() % (max - min) + 1;

        return random;
    }

    // 生成随机字符串(测试用)
    static QString GenerateRandomString(int length, int mode = 0)
    {
        static const QString defLetter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

        static const QString orgNumLetter = "0123456789";
        static const QString orgUpLetter  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        static const QString orgLowLetter = "abcdefghijklmnopqrstuvwxyz";
        static const QString orgAllLetter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789=-/+";

        QString charset = defLetter;
        if (mode == 1) {
            charset = orgNumLetter;
        } else if (mode == 2) {
            charset = orgUpLetter;
        } else if (mode == 3) {
            charset = orgLowLetter;
        } else if (mode == 4) {
            charset = orgAllLetter;
        }

        QString randomString;
        randomString.reserve(length);

        for (int i = 0; i < length; ++i) {
            int index = QRandomGenerator::global()->bounded(charset.length());
            randomString.append(charset.at(index));
        }

        return randomString;
    }

    // 判断是否是IP地址
    static bool CheckIP(const QString &ip)
    {
        QRegExp regExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
        return regExp.exactMatch(ip);
    }

    // 模糊延时
    static void msleep(unsigned long msec)
    {
        QTime deiTime = QTime::currentTime().addMSecs(msec);
        while (QTime::currentTime() < deiTime)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
    }

    // 窗口居中桌面
    static void WidgetInCenter(QWidget &widget)
    {
        QDesktopWidget w;
        int deskWidth = w.width();
        int deskHeight = w.height();

        QPoint centerPoint((deskWidth - widget.width()) / 2, (deskHeight - widget.height()) / 2);
        widget.move(centerPoint);
    }

    // 设置为开机启动
    static void AutoRunWithSystem(bool isAutoRun, const QString &appName, const QString &appPath)
    {
        QSettings reg(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), QSettings::NativeFormat);

        if (isAutoRun)
            reg.setValue(appName, appPath);
        else
            reg.setValue(appName, "");
    }

    // 应用重启
    static void Reboot()
    {
        QString program = QApplication::applicationFilePath();
        QStringList argument = QApplication::arguments();
        QString workDirectory = QDir::currentPath();
        QProcess::startDetached(program, argument, workDirectory);
        QApplication::exit();
    }

    // 按目标size裁剪图片
    static QImage ImageZoomAndCrop(const QImage &inputImage, const QSize &targetSize)
    {
        // 缩放图像，保持原始宽高比例
        QImage scaledImage = inputImage.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // 如果缩放后的图像尺寸超过了目标尺寸，进行居中裁剪
        if (scaledImage.width() > targetSize.width() || scaledImage.height() > targetSize.height()) {
            // 计算裁剪区域的坐标
            int xOffset = (scaledImage.width() - targetSize.width()) / 2;
            int yOffset = (scaledImage.height() - targetSize.height()) / 2;

            // 裁剪图像
            QRect cropRect(xOffset, yOffset, targetSize.width(), targetSize.height());
            scaledImage = scaledImage.copy(cropRect);
        }

        return scaledImage;
    }

};

#endif // COMMONFUNC_H
