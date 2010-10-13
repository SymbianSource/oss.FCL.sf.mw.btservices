#
# Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#

TEMPLATE = lib
TARGET = btuidelegate
MOC_DIR = moc
DEFINES += BUILD_BTUIDELEGATE
INCLUDEPATH += . \
    ../inc

TRANSLATIONS = btdialogs.ts
# Only one .ts file allowed for a .pro file. btviews.ts is in btcpplugin.pro
RESOURCES += btuidelegate.qrc

CONFIG += qt \
    hb \
    dll
    
HEADERS += btdelegatepower.h \
    ../inc/btdelegatefactory.h \
    ../inc/btabstractdelegate.h \
    btdelegatevisibility.h \
    btdelegatedevname.h \
    btdelegateinquiry.h \
    btdelegateconnect.h \
    btdelegatepair.h \
    btdelegatedisconnect.h \
    btdelegatedevsecurity.h \
    btdelegateremotedevname.h \
    btdelegatedevremove.h
    
SOURCES += btdelegatepower.cpp \
    btdelegatefactory.cpp \
    btabstractdelegate.cpp \
    btdelegatevisibility.cpp \
    btdelegatedevname.cpp \
    btdelegateinquiry.cpp \
    btdelegateconnect.cpp \
    btdelegatepair.cpp \
    btdelegatedisconnect.cpp \
    btdelegatedevsecurity.cpp \
    btdelegateremotedevname.cpp \
    btdelegatedevremove.cpp

defFilePath = .
    
symbian: { 
    SYMBIAN_PLATFORMS = WINSCW \
        ARMV5
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.UID3 = 0xEE02434F
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
    LIBS += -lbtengsettings \
        -lbtserviceutil \
        -lbtdevice \
        -lbtmanclient \
        -lesock \
        -lbluetooth \
        -lbtengdevman \
        -lbtengconnman \
        -lcentralrepository \
        -lflogger \
        -lbtuimodel
    //MMP_RULES -= EXPORTUNFROZEN
}
