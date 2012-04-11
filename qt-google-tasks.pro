#-------------------------------------------------
#
# Project created by QtCreator 2012-01-08T21:31:53
#
#-------------------------------------------------

QT       += core gui webkit network


TARGET = qttasks
TEMPLATE = app

INCLUDEPATH += ../qjson/include

linux* {
LIBS += ../qjson/build/lib/libqjson.so
}

win* {
LIBS += ../qjson/build/lib/qjson0.dll
}

macx* {
LIBS += -F../qjson/build/lib -framework qjson
}


SOURCES += main.cpp\
        mainwindow.cpp \
    form.cpp \
    oauth2.cpp \
    logindialog.cpp \
    tasks_data_manager.cpp \
    tree_widget.cpp

HEADERS  += mainwindow.h \
    form.h \
    oauth2.h \
    logindialog.h \
    tasks_data_manager.h \
    tree_widget.h

FORMS    += mainwindow.ui \
    form.ui \
    logindialog.ui \
    item_widget.ui

RESOURCES += \
    resource.qrc


















