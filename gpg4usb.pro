######################################################################
# Automatically generated by qmake (2.01a) Mi Mai 21 02:28:39 2008
######################################################################

TEMPLATE = app
#unix:TARGET = start_linux
#win32:TARGET = start_windows
#mac:TARGET = start_mac
DESTDIR = release
DEPENDPATH += .
INCLUDEPATH += . ./include
CONFIG += release static

# Input
HEADERS += context.h gpgwin.h keylist.h keymgmt.h fileencryptiondialog.h keygenthread.h
SOURCES += context.cpp gpgwin.cpp main.cpp keylist.cpp keymgmt.cpp fileencryptiondialog.cpp keygenthread.cpp
RC_FILE = gpg4usb.rc
# comment out line below for static building
LIBS += -lgpgme -lgpg-error
DEFINES += _FILE_OFFSET_BITS=64

TRANSLATIONS = release/ts/gpg4usb_en.ts \
         release/ts/gpg4usb_de.ts
