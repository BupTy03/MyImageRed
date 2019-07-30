#-------------------------------------------------
#
# Project created by QtCreator 2018-07-05T14:21:56
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

VERSION = 3.0
QMAKE_TARGET_COMPANY = Anton and Corp.
QMAKE_TARGET_PRODUCT = Image Correction App
QMAKE_TARGET_DESCRIPTION = Prost Proga
QMAKE_TARGET_COPYRIGHT = Anton Lag

TARGET = ImageRed
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++14
CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0




SOURCES += \
    imageholder.cpp \
    kerneldimensiondialog.cpp \
        main.cpp \
        mainwindow.cpp \
    histogram.cpp \
    imageprocessor.cpp \
    inputmatrixdialog.cpp \
    imageprocessing.cpp \
    fileiterator.cpp

HEADERS += \
    imageholder.hpp \
    kerneldimensiondialog.hpp \
    timer.hpp \
    mycoloriterator.hpp \
    matrix.hpp \
    mainwindow.hpp \
    inputmatrixdialog.hpp \
    imageprocessor.hpp \
    imageprocessing.hpp \
    histogram.hpp \
    fileiterator.hpp

FORMS += \
        mainwindow.ui

RC_ICONS = icon.ico

RESOURCES += \
    images.qrc \
    styles.qrc
