#-------------------------------------------------
#
# Project created by QtCreator 2016-04-29T12:51:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = glWidget
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp \
    logo.cpp \
    window.cpp \
    dobject.cpp \
    oLoader.cpp \
    material.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    logo.h \
    window.h \
    dClock.h \
    dFace.h \
    dobject.h \
    oLoader.h \
    material.h

FORMS    += mainwindow.ui

DISTFILES += \
    StandardShading.fragmentshader \
    StandardShading.vertexshader \
    olento_testi.obj

CONFIG += console
