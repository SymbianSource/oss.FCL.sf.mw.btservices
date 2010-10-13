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
TARGET = btconversationviewlauncher 	

TARGET.UID3 = 0xA89FB97A

TARGET.CAPABILITY = CAP_GENERAL_DLL

INCLUDEPATH += ./inc

CONFIG += hb

# Input
SOURCES += ./src/btconversationviewlauncher.cpp

HEADERS += ./inc/btconversationviewlauncher.h

LIBS += -lhbcore \
		-lxqservice \
		-lxqserviceutil

symbian*: { 
	TARGET.EPOCALLOWDLLDATA = 1
	MMP_RULES -= "OPTION_REPLACE ARMCC --export_all_vtbl -D__QT_NOEFFECTMACRO_DONOTUSE"
	}

BLD_INF_RULES.prj_exports += "./rom/btconversationviewlauncher.iby CORE_APP_LAYER_IBY_EXPORT_PATH(btconversationviewlauncher.iby)"
