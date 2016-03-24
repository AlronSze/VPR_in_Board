#-------------------------------------------------
#
# Project created by QtCreator 2016-02-21T11:21:27
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VPR_PC
TEMPLATE = app


SOURCES += main.cpp \
           loginmanager.cpp \
    datapacket.cpp \
    filemanager.cpp

HEADERS += loginmanager.h \
    datapacket.h \
    filemanager.h

FORMS   += loginmanager.ui \
    filemanager.ui
