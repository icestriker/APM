#-------------------------------------------------
#
# Project created by QtCreator 2016-11-08T09:36:31
#
#-------------------------------------------------

QT       += core gui
QT       +=webkit webkitwidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = APM
TEMPLATE = app


SOURCES += main.cpp\
        apm.cpp \
    qcustomplot.cpp

HEADERS  += apm.h \
    qcustomplot.h

FORMS    += apm.ui
RC_FILE=logo.rc

RESOURCES += \
    ui.qrc

OTHER_FILES += \
    logo.rc \
    logo.ico
