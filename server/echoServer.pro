QT += core network sql
QT -= gui

CONFIG += console c++11
CONFIG -= app_bundle

TARGET = echoServer
TEMPLATE = app

SOURCES += \
    main.cpp \
    mytcpserver.cpp \
    database.cpp \
    calculator.cpp \
    functionsforserver.cpp

HEADERS += \
    mytcpserver.h \
    database.h \
    calculator.h \
    functionsforserver.h
