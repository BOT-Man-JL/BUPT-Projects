#-------------------------------------------------
#
# Project created by QtCreator 2017-06-14T16:07:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Air-Conditioner-Client
TEMPLATE = app
RC_ICONS = :/button/icon.png
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
    ../../src/client/client.cpp \
    ../../src/client/client-view-gui-qt.cpp \
    ../../src/client/client-view-manager-gui.cpp

HEADERS += \
    ../../src/client/client-controller.h \
    ../../src/client/client-model.h \
    ../../src/client/client-protocol.h \
    ../../src/client/client-view.h \
    ../../src/common/json/json.hpp \
    ../../src/common/bot-cs.h \
    ../../src/common/bot-socket.h \
    ../../src/common/common-model.h \
    ../../src/common/common-protocol.h \
    ../../src/common/common-view.h \
    ../../src/client/client-view-gui.h \
    ../../src/client/client-view-gui-qt.h

FORMS += \
    ../../src/client/ui/authwindow.ui \
    ../../src/client/ui/controlwindow.ui

DISTFILES += \
    ../../src/common/json/json.zip

RESOURCES += \
    ../../src/client/ui/resource.qrc
