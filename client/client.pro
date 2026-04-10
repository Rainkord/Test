QT += core gui widgets network

CONFIG += c++17

TARGET = client
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    authwidget.cpp \
    regwidget.cpp \
    verifywidget.cpp \
    graphwidget.cpp \
    clientsingleton.cpp \
    taskdialog.cpp \
    schemadialog.cpp

HEADERS += \
    mainwindow.h \
    authwidget.h \
    regwidget.h \
    verifywidget.h \
    graphwidget.h \
    clientsingleton.h \
    taskdialog.h \
    schemadialog.h

# Suppress deprecation warnings for Qt5 compatibility
DEFINES += QT_DEPRECATED_WARNINGS

# Output binary to build folder
DESTDIR = $$PWD/../build

# Default rules for deployment (mobile platforms, etc.)
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target