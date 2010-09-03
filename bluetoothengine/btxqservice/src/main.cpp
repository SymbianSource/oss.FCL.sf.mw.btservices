/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:  
 *
 */


#include <qcoreapplication.h>
#include "btxqaddrservice.h"
#include "btxqpowerservice.h"
#include "btxqdutservice.h"
#include "btxqsspdebugservice.h"


int main(int argc, char **argv)
{
    QCoreApplication a( argc, argv );
    
    BtxqAddrService* btAddrService = new BtxqAddrService();
    BtxqPowerService* btToggleService = new BtxqPowerService();
    BtxqDutService* btDutService = new BtxqDutService();
    BtxqSspDebugService* btSspDebugService = new BtxqSspDebugService();

    int retVal = a.exec();
    
    delete btSspDebugService;
    delete btDutService;
    delete btToggleService;
    delete btAddrService;
    
    return retVal;
}

