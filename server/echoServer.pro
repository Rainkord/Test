QT += core network sql
CONFIG += console c++17
CONFIG -= app_bundle
TARGET = echoServer
SOURCES += main.cpp mytcpserver.cpp database.cpp calculator.cpp functionsforserver.cpp smtpclient.cpp
HEADERS += mytcpserver.h database.h calculator.h functionsforserver.h smtpclient.h
