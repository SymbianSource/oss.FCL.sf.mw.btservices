/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "btconversationviewlauncher.h"
#include <xqaiwrequest.h>
#include <xqappmgr.h>

const qint64 KBluetoothMsgsConversationId = 0x01;


EXPORT_C CBtConversationViewLauncher* CBtConversationViewLauncher::NewL()
    {
    return (new(ELeave)CBtConversationViewLauncher);
    }

EXPORT_C CBtConversationViewLauncher::~CBtConversationViewLauncher()
    {
    
    }

/**
This API makes use of the QtHighway service provided by the Messaging Application
to open the Bluetooth conversation view.
*/

EXPORT_C void CBtConversationViewLauncher::LaunchConversationViewL()
    {
    QString service("com.nokia.services.hbserviceprovider");
    QString interface("conversationview");
    QString operation("open(qint64)");
    QList<QVariant> args;
    XQAiwRequest* request;
    XQApplicationManager appManager;
    request = appManager.create(service, interface, operation, false); // not embedded
    if ( request == NULL )
        {
        //Could not create request because of the arguments passed to the create() API.
        User::Leave(KErrArgument);       
        }
    args << QVariant(KBluetoothMsgsConversationId);
    request->setArguments(args);
    TInt error = KErrNone;
    if(!request->send())
        {
        // The only likely Symbian error code that can be associated is KErrNotSupported
        error = KErrNotSupported;
        }
    delete request;
    User::LeaveIfError(error);
    }

CBtConversationViewLauncher::CBtConversationViewLauncher()
    {
    
    }
