QT += core gui widgets network
CONFIG += c++17
TARGET = client
TEMPLATE = app
SOURCES += main.cpp mainwindow.cpp authwidget.cpp regwidget.cpp graphwidget.cpp clientsingleton.cpp
HEADERS += mainwindow.h authwidget.h regwidget.h graphwidget.h clientsingleton.h
