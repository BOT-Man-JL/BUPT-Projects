#-------------------------------------------------
#
# Project created by QtCreator 2017-06-10T23:57:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Air-Conditioner-Server
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    ../../src/server/server.cpp \
    ../../src/server/ormlite/sqlite3.c \
    ../../src/server/server-view-manager-gui.cpp \
    ../../src/server/server-view-gui-qt.cpp

HEADERS += \
    ../../src/server/ormlite/ormlite.h \
    ../../src/server/ormlite/sqlite3.h \
    ../../src/server/server-model.h \
    ../../src/server/server-protocol-controller.h \
    ../../src/server/server-protocol.h \
    ../../src/server/server-service.h \
    ../../src/server/server-view-controller.h \
    ../../src/server/server-view.h \
    ../../src/common/json/json.hpp \
    ../../src/common/bot-cs.h \
    ../../src/common/bot-socket.h \
    ../../src/common/common-model.h \
    ../../src/common/common-protocol.h \
    ../../src/common/common-view.h \
    ../../src/common/json/json.hpp \
    ../../src/server/server-view-gui.h \
    ../../src/server/server-view-gui-qt.h \
    ../../src/server/log-helper.h \
    ../../src/server/time-helper.h

FORMS += \
    ../../src/server/ui/configwindow.ui \
    ../../src/server/ui/guestwindow.ui \
    ../../src/server/ui/statisticwindow.ui \
    ../../src/server/ui/welcomewindow.ui \
    ../../src/server/ui/clientwindow.ui

DISTFILES += \
    ../../src/server/ormlite/ormlite.h.zip \
    ../../src/server/ormlite/sqlite3.c.zip \
    ../../src/server/ormlite/sqlite3.h.zip \
    ../../src/common/json/json.zip \

RESOURCES += \
    ../../src/server/ui/resource.qrc
