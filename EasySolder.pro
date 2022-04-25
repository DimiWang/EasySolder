#-------------------------------------------------
#
# Project created by QtCreator 2015-03-03T14:45:48
#   Qt4
#-------------------------------------------------

QT       += core gui
RC_FILE  = app.rc
TARGET = EasySolder
TEMPLATE = app
DESTDIR = $$PWD/bin
UI_DIR = $$PWD/tmp
MOC_DIR = $$PWD/tmp


SOURCES += main.cpp\
        mainwindow.cpp \    
        workarea.cpp \
    importdlg.cpp \
    customdialogs.cpp

HEADERS  += mainwindow.h \    
        workarea.h \
        partitem.h \
        version/v.h \
    importdlg.h \
    customdialogs.h

unix:{
  message(linux compilation)
  DEFINES += __unix__
}

win32:{
  message(Windows compilation)
  CONFIG  += qaxcontainer
  DEFINES += __windows__
  SOURCES +=  exceldata.cpp
  HEADERS +=  exceldata.h
}

FORMS    += mainwindow.ui \
    importdlg.ui

RESOURCES += \
    resource.qrc

RC_FILE  = easysolder.rc #Rc with On semi Icon