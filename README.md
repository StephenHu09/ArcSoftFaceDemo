# ArcSoftFaceDemo
虹软（ArcSoft）人脸识别SDK3.0 + Qt实现，在win10环境下使用Qt5.12 编译调试，实现基本功能与接口调用。
1. 实现静态图片人脸识别(Detect)
2. 实现静态图片下的人脸相似度比对(Compare)
3. 实现Camera 进行人脸录入和人脸验证(Verify)


#### 软件架构
软件架构说明


#### 安装教程

1. 无需安装，不依赖openCV，使用Qt编译运行

#### 使用说明

1. 使用Qt Creator 打开 *.pro 文件编译运行即可。Qt5.12版本，编译工具选择 msvc2017 32bit或者64bit；
2. 本代码不包含库文件（自行下载 虹软SDK版本V3.0），需要手动将虹软的SDK库文件手动拷贝到 lib/X86 和 lib/X64 两个目录下；
3. APPID 和 SDK_KEY 默认从配置文件读取，配置文件参考 lib/config.ini，并放在程序运行目录下 ArcSoftFace/config.ini，再重新启动程序。
