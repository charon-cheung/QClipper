#-------------------------------------------------
#
# Project created by QtCreator 2017-06-22T19:09:40
#
#-------------------------------------------------
include( qxt.pri )
QT  += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QClipper
TEMPLATE = app

RC_ICONS = qclipper.ico

DESTDIR = bin
RC_FILE = app.rc
CONFIG(debug, debug | release){
    compiled = debug
}
CONFIG(release, debug | release){
    compiled = release
}

UI_DIR = $$compiled/ui
MOC_DIR = $$compiled/moc
OBJECTS_DIR = $$compiled/obj
RCC_DIR = $$compiled/res

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
        main.cpp \
        qclipper.cpp \
    template.cpp \
    setting.cpp \
    savecmd.cpp

HEADERS += \
        qclipper.h \
    template.h \
    setting.h \
    savecmd.h

FORMS += \
        qclipper.ui \
    template.ui \
    setting.ui

RESOURCES += \
    Res.qrc\

DISTFILES += \
    app.rc

#RC_FILE += app.rc
