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

TEMPLATE = app
TARGET = btxqservice
CONFIG += hb \
    service
MOC_DIR = moc
OBJECTS_DIR = obj
symbian:
 { 
    TARGET.UID3 = 0xA89FB238
    TARGET.CAPABILITY = ALL -TCB
}
LIBS += -lhbcore \
    -lxqservice \
    -lxqserviceutil \
    -lbluetooth \
    -lbtengsettings \
    -lxqutils
SERVICE.FILE = xml/btxqservice_conf.xml

SERVICE.OPTIONS = embeddable \
    hidden
    
libFiles.path = "!:/sys/bin"

DEPLOYMENT += libFiles

HEADERS += inc/btxqaddrservice.h \
           inc/btxqpowerservice.h \
           inc/btxqdutservice.h \
           inc/btxqsspdebugservice.h
           
SOURCES += src/btxqaddrservice.cpp \
           src/main.cpp \
           src/btxqpowerservice.cpp \
           src/btxqdutservice.cpp \
           src/btxqsspdebugservice.cpp

INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE \
               ../inc \
               /epoc32/include/platform/mw/btservices

BLD_INF_RULES.prj_exports += \
  "$${LITERAL_HASH}include <platform_paths.hrh>" \
  "rom/btxqservice.iby   CORE_MW_LAYER_IBY_EXPORT_PATH(btxqservice.iby)"
  
  