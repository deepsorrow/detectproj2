#-------------------------------------------------
#
# Project created by QtCreator 2018-11-09T10:08:00
#
#-------------------------------------------------

QT       += core gui multimediawidgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = detectproj_2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

INCLUDEPATH += C:\opencv\build\include

LIBS += C:\opencv-build\bin\libopencv_core343.dll
LIBS += C:\opencv-build\bin\libopencv_highgui343.dll
LIBS += C:\opencv-build\bin\libopencv_imgcodecs343.dll
LIBS += C:\opencv-build\bin\libopencv_imgproc343.dll
LIBS += C:\opencv-build\bin\libopencv_features2d343.dll
LIBS += C:\opencv-build\bin\libopencv_calib3d343.dll
LIBS += C:\opencv-build\bin\libopencv_objdetect343.dll
LIBS += C:\opencv-build\bin\libopencv_videoio343.dll
LIBS += C:\Qt\5.11.2\mingw53_32\plugins\sqldrivers\qsqlmysql.dll


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    player.cpp \
    detection.cpp \
    masking.cpp \
    functions.cpp

HEADERS += \
        mainwindow.h \
    player.h \
    detection.h \
    masking.h \
    functions.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
