######################################################################
# Automatically generated by qmake (2.01a) Sa. Dez 10 12:52:21 2011
######################################################################

CONFIG += qtestlib
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
#DESTDIR = ../release

# Input
SOURCES += testgpgcontext.cpp \
           ../gpgcontext.cpp \
           ../gpgconstants.cpp
HEADERS += ../gpgcontext.h \
           ../gpgconstants.h

LIBS += -lgpgme \
     -lgpg-error \

