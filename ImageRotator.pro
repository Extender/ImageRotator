#-------------------------------------------------
#
# Project created by QtCreator 2016-04-24T12:53:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageRotator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    graphicssceneex.cpp \
    graphicsviewex.cpp \
    io.cpp \
    text.cpp

HEADERS  += mainwindow.h \
    graphicssceneex.h \
    graphicsviewex.h \
    io.h \
    text.h \
    extcolordefs.h

FORMS    += mainwindow.ui
