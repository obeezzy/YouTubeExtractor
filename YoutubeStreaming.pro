#-------------------------------------------------
#
# Project created by QtCreator 2016-07-22T13:16:54
#
#-------------------------------------------------

QT       += core gui network av avwidgets

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YoutubeStreaming
TEMPLATE = app

SOURCES += examples/main.cpp\
        examples/mainwindow.cpp \
    youtubeextractor/youtubeextractor.cpp

HEADERS  += examples/mainwindow.h \
    youtubeextractor/youtubeextractor.h

FORMS    += examples/mainwindow.ui
