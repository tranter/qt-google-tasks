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

win32-g++ {
    #MinGW compiler for Windows
    message(Configured for Windows (Qt Version: $$QT_VERSION; $$QMAKE_CC))

    LIBS += ../qjson/build/lib/qjson0.dll
}

win32-msvc* {
    #MSVC compiler for Windows
    message(Configured for Windows (Qt Version: $$QT_VERSION; $$QMAKE_CC))

    LIBS += ../qjson/build/lib/qjson0.lib
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


















