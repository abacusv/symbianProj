# Copyright (c) 2011 Nokia Corporation.

TARGET = qwhowhere

TEMPLATE = app

QT += network svg

VERSION = 1.2.0	

CONFIG += mobility
MOBILITY = location \
    contacts \
    messaging

SOURCES += src/Message.cpp \
    src/ContactsDialog.cpp \
    src/FetchMap.cpp \
    src/PictureItem.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/Button.cpp \
    src/QueryDialog.cpp
	
HEADERS += src/Message.h \
    src/ContactsDialog.h \
    src/FetchMap.h \
    src/PictureItem.h \
    src/mainwindow.h \
    src/Button.h \
    src/QueryDialog.h

RESOURCES = src/resources.qrc

symbian { 
    ICON = src/icons/whowhere_icon.svg
    TARGET = QWhoWhere
    TARGET.UID3 = 0xEEF9CA30
    TARGET.EPOCHEAPSIZE = 0x100000 0x2000000
    TARGET.EPOCSTACKSIZE = 0x14000


    # Self-signing capabilities
    TARGET.CAPABILITY += NetworkServices \
        ReadUserData \
        WriteUserData \
        LocalServices \
        UserEnvironment \
        Location

    # Enable QtMobility Messaging module
    #DEFINES += MESSAGING_ENABLED
    # Additional capabilities for QtMobility Messaging module
    # that needs Open Signed Online signing
    #TARGET.CAPABILITY += ReadDeviceData WriteDeviceData

    # For locked orientation and image conversion
    LIBS += -lcone -leikcore -lavkon
}

maemo5 { 
    BINDIR = /opt/usr/bin
    DATADIR = /usr/share
    DEFINES += DATADIR=\\\"$$DATADIR\\\" \
        PKGDATADIR=\\\"$$PKGDATADIR\\\"
    INSTALLS += target \
        desktop \
        icon64
		
    target.path = $$BINDIR
    desktop.path = $$DATADIR/applications/hildon
    desktop.files += debian/qwhowhere.desktop

    icon64.path = $$DATADIR/icons/hicolor/64x64/apps
    icon64.files += src/icons/64x64/qwhowhere.png

    # Enable QtMobility Messaging module
    #DEFINES += MESSAGING_ENABLED
}

!contains(DEFINES,MESSAGING_ENABLED) {
    warning(Qt Mobility Messaging disabled!)
}
