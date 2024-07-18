#-------------------------------------------------
#
# Project created by QtCreator 2016-12-03T20:36:21
#
#-------------------------------------------------

CONFIG   += c++11
QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = ReferenceOrganiser
TEMPLATE = app


SOURCES += main.cpp \
    addpaperdialog.cpp \
    createdatabasedialog.cpp \
    databasefilereader.cpp \
    databasefilewriter.cpp \
    databasehandler.cpp \
    databasenamedialog.cpp \
    duplicatesviewer.cpp \
    history.cpp \
    metadialog.cpp \
    organisermain.cpp \
    settingsdialog.cpp \
    searchdialog.cpp \
    reviewparser.cpp \
    busyindicator.cpp \
    reviewscanner.cpp


HEADERS  += organisermain.h \
    addpaperdialog.h \
    createdatabasedialog.h \
    databasefilereader.h \
    databasefilewriter.h \
    databasehandler.h \
    databasenamedialog.h \
    duplicatesviewer.h \
    history.h \
    metadialog.h \
    settingsdialog.h \
    searchdialog.h \
    reviewparser.h \
    busyindicator.h \
    reviewscanner.h \
    recordlistitem.h \
    papermeta.h

FORMS    += organisermain.ui \
    addpaper.ui \
    createdatabasedialog.ui \
    databasenamedialog.ui \
    duplicatesdialog.ui \
    metadialog.ui \
    settings.ui \
    searchdialog.ui

RESOURCES += \
    reforg.qrc

win32 {
    RC_FILE = reforg.rc
}
