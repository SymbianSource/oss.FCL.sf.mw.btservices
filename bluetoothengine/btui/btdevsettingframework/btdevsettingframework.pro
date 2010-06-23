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
TARGET = btdevsettingframework

MOC_DIR = moc
DEFINES += BUILD_BTDEVSETTINGFRAMEWORK

INCLUDEPATH += inc
    
CONFIG += qt \
    hb \
    dll
    
HEADERS += inc/btdevsettinginterface.h \
	inc/btabstractdevsetting.h \
	inc/btdevsettingpluginloader.h

SOURCES += src/btabstractdevsetting.cpp \
		   src/btdevsettingpluginloader.cpp
		   
symbian: { 
    SYMBIAN_PLATFORMS = WINSCW \
        ARMV5
        
        BLD_INF_RULES.prj_exports += \
          "$${LITERAL_HASH}include<platform_paths.hrh>" \
          "inc/btdevsettingglobal.h MW_LAYER_PLATFORM_EXPORT_PATH(btdevsettingglobal.h)" \
          "inc/btdevsettinginterface.h MW_LAYER_PLATFORM_EXPORT_PATH(btdevsettinginterface.h)" \
          "inc/btabstractdevsetting.h MW_LAYER_PLATFORM_EXPORT_PATH(btabstractdevsetting.h)" \
          "inc/btdevsettingpluginloader.h MW_LAYER_PLATFORM_EXPORT_PATH(btdevsettingpluginloader.h)" \
          "rom/btdevsettingframework.iby CORE_MW_LAYER_IBY_EXPORT_PATH(btdevsettingframework.iby)"
		  
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.UID3 = 0xEEEEEEEE
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    
    INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
}
