#-------------------------------------------------
#
# Project created by QtCreator 2016-02-21T11:21:27
#
#-------------------------------------------------

QT       += core gui
CONFIG   += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VPR_Board
TEMPLATE = app


SOURCES += main.cpp \
           loginmanager.cpp \
           recordvoice.cpp \
    mfcc.cpp \
    datapacket.cpp

HEADERS += loginmanager.h \
           recordvoice.h \
    mfcc.h \
    datapacket.h

FORMS   += loginmanager.ui

RESOURCES +=
