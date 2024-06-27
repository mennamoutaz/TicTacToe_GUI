QT += core gui widgets testlib

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_unittests.cpp \
    ../../Desktop/elhamdullahfinalfinal/elhamdullah4/aiplayer.cpp \
    ../../Desktop/elhamdullahfinalfinal/elhamdullah4/gameboard.cpp \
    shell.c \
    sqlite3.c

HEADERS += \
    ../../Desktop/elhamdullahfinalfinal/elhamdullah4/aiplayer.h \
    ../../Desktop/elhamdullahfinalfinal/elhamdullah4/gameboard.h \
    sqlite3.h \
    sqlite3ext.h

DISTFILES += \
    sqlite3.def \
    sqlite3.dll
