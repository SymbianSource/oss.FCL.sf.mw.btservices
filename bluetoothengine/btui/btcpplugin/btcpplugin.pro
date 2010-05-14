#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
TARGET = btcpplugin
DEPENDPATH += .
INCLUDEPATH += . ../inc/


CONFIG += qt hb plugin
LIBS += -lcpframework -lbtuimodel -lbtuidelegate

MOC_DIR = moc
OBJECTS_DIR = obj

#TRANSLATIONS = telephone_cp.ts
RESOURCES += btcpplugin.qrc

# Input
HEADERS += btcpplugin.h \
           btcpuibaseview.h \
           btcpuimainview.h \
           btcpuisearchview.h \
           btcpuisettingitem.h

SOURCES += btcpplugin.cpp \
           btcpuibaseview.cpp \
           btcpuimainview.cpp \
           btcpuisearchview.cpp \
           btcpuisettingitem.cpp
           
symbian: {
    DEFINES += PLUGINUID3=0x2002434E
    TARGET.UID3 = 0x2002434E
    TARGET.CAPABILITY = ALL -TCB
    
    TARGET.EPOCALLOWDLLDATA = 1  
	INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
    
	PLUGIN_STUB_PATH = /resource/qt/plugins/controlpanel
	
	deploy.path = C:
	pluginstub.sources = $${TARGET}.dll
	pluginstub.path = $$PLUGIN_STUB_PATH
	DEPLOYMENT += pluginstub

    qtplugins.path = $$PLUGIN_STUB_PATH
    qtplugins.sources += qmakepluginstubs/$${TARGET}.qtplugin
     
    for(qtplugin, qtplugins.sources):BLD_INF_RULES.prj_exports += "./$$qtplugin  $$deploy.path$$qtplugins.path/$$basename(qtplugin)" 
} 

