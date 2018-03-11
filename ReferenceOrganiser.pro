#-------------------------------------------------
#
# Project created by QtCreator 2016-12-03T20:36:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ReferenceOrganiser
TEMPLATE = app


SOURCES += main.cpp\
        organisermain.cpp \
    settingsdialog.cpp \
    searchdialog.cpp \
    reviewparser.cpp \
    busyindicator.cpp \
    reviewscanner.cpp \
    reviewedit.cpp

HEADERS  += organisermain.h \
    settingsdialog.h \
    searchdialog.h \
    paperrecord.h \
    reviewparser.h \
    busyindicator.h \
    reviewscanner.h \
    reviewedit.h \
    recordlistitem.h

FORMS    += organisermain.ui \
    settings.ui \
    newreview.ui \
    searchdialog.ui

RESOURCES += \
    reforg.qrc

win32 {
    RC_FILE = reforg.rc
}
