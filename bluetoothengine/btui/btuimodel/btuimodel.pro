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
TARGET = btuimodel

MOC_DIR = moc
DEFINES += BUILD_BTUIMODEL

INCLUDEPATH += . \
    ../inc
    
CONFIG += qt \
    hb \
    dll
    
HEADERS += btdevicedata.h \
    btlocalsetting.h \
    btuimodel.h \
    activewrapper.h
SOURCES += btdevicedata.cpp \
    btlocalsetting.cpp \
    btuimodel.cpp \
    activewrapper.cpp
    
symbian: { 
    SYMBIAN_PLATFORMS = WINSCW \
        ARMV5
	BLD_INF_RULES.prj_exports += "btuimodel.h |../inc/btuimodel.h"
	
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.UID3 = 0x2002434F
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
        -lflogger
    //MMP_RULES -= EXPORTUNFROZEN
}
