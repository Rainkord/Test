QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

# Подключаем исходники сервера напрямую (без DB и SMTP чтобы не нужны зависимости)
INCLUDEPATH += ../server

SOURCES += \
    tst_calculator.cpp \
    ../server/calculator.cpp

HEADERS += \
    ../server/calculator.h
