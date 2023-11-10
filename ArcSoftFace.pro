QT       += core gui
QT       += multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win* {
    contains(QT_ARCH, x86_64){
        message("64-bit")
        # LIBS += -L$$PWD/lib/X64 -l"libarcsoft_face" -l"libarcsoft_face_engine"
        LIBS += -L$$PWD/lib/X64/ -llibarcsoft_face_engine
        PLATFORM_X64 = WIN_X64
        DEFINES += $${PLATFORM_X64}
    }else{
        message("32-bit")
        LIBS += -L$$PWD/lib/X86/ -llibarcsoft_face_engine
    }
}

INCLUDEPATH += $$PWD/lib/X86
DEPENDPATH += $$PWD/lib/X86
INCLUDEPATH += $$PWD/lib/X64
DEPENDPATH += $$PWD/lib/X64
INCLUDEPATH += $$PWD/lib/inc


SOURCES += \
    arcfaceengine.cpp \
    facecamera.cpp \
    facemodule.cpp \
    main.cpp \
    arcsoftface.cpp \
    userinfo.cpp

HEADERS += \
    commonfunc.h \
    arcfaceengine.h \
    arcsoftface.h \
    facecamera.h \
    facemodule.h \
    userinfo.h


FORMS += \
    arcsoftface.ui \
    facecamera.ui

VERSION = 1.0.3
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/X64/ -llibarcsoft_face_engine
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/X64/ -llibarcsoft_face_engined


