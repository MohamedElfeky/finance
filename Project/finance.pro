#-------------------------------------------------
#
# Project created by QtCreator 2015-12-26T19:10:51
#
#-------------------------------------------------

QT += core gui
QT += printsupport
QT += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = finance
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h

FORMS    += mainwindow.ui
