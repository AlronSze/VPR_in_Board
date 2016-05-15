#-------------------------------------------------
#
# Project created by QtCreator 2016-02-21T11:21:27
#
#-------------------------------------------------

QT       += core gui
QT       += serialport # For PC debug
#CONFIG  += serialport # For board

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VPR_Board
TEMPLATE = app


SOURCES += main.cpp \
           loginmanager.cpp \
           recordvoice.cpp \
    mfcc.cpp \
    datapacket.cpp \
    gmm.cpp

HEADERS += loginmanager.h \
           recordvoice.h \
    mfcc.h \
    datapacket.h \
    gmm.h

FORMS   += loginmanager.ui

RESOURCES +=
