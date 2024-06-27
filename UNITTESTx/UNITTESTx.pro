QT += core gui widgets testlib

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
     ../Project/aiplayer.cpp \
     ../Project/gameboard.cpp \
     ../Project/mainwindow.cpp \
       tst_unittests.cpp
    shell.c \
    sqlite3.c

HEADERS += \
    ../Project/aiplayer.h \
    ../Project/gameboard.h \
    ../Project/mainwindow.h \
    sqlite3.h \
    sqlite3ext.h
    
SOURCES += tst_unittests.moc


DISTFILES += \
    sqlite3.def \
    sqlite3.dll

FORMS += \
    ../Project/mainwindow.ui
    
    # Include path to Qt headers
INCLUDEPATH += $$PWD/../../Qt/5.15.2/mingw81_64/include

# Ensure linking with necessary Qt libraries
LIBS += -L$$PWD/../../Qt/5.15.2/mingw81_64/lib \
    -lQt5Test

QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_WARN_ON

# Define the target name
TARGET = UNITTESTx
