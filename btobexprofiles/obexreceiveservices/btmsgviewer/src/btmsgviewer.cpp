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


#include "btmsgviewer.h"
#include "apmstd.h"
#include <xqaiwrequest.h>
#include <f32file.h>
//#include <documenthandler.h>


BTMsgViewer::BTMsgViewer(QObject* parent)
: XQServiceProvider("com.nokia.services.btmsgdispservices.displaymsg",parent)
    {    
    publishAll();
    }

BTMsgViewer::~BTMsgViewer ()
    {
    
    }

int BTMsgViewer::displaymsg( int messageId )
    {    
    CBtMsgViewerUtils* btViewerUtils = 0;
    
    TRAPD(error, btViewerUtils = CBtMsgViewerUtils::NewL());  
    if(isError(error))
        {
        if(btViewerUtils)
            delete btViewerUtils;
        
        return error;   
        }
        
    HBufC* fileName = 0;
    fileName = btViewerUtils->GetMessagePath(messageId, error);
    if(isError(error))
        {
        if(fileName)
            delete fileName;
        
        delete btViewerUtils;
        return error;
        }
    
    QString attachmentFName = QString::fromUtf16(fileName->Ptr(),fileName->Length());    
    
    delete fileName;
    delete btViewerUtils;

    XQSharableFile sf;
    XQAiwRequest* request = 0;

    if (!sf.open(attachmentFName)) {
        return KErrNotFound;
    }

    // Get handlers
    XQApplicationManager appManager;
    QList<XQAiwInterfaceDescriptor> fileHandlers = appManager.list(sf);
    if (fileHandlers.count() > 0) {
        XQAiwInterfaceDescriptor d = fileHandlers.first();
        request = appManager.create(sf, d);

        if (!request) {
            sf.close();
            return KErrGeneral;
        }
    }
    else {
        sf.close();
        return KErrGeneral;
    }

    request->setEmbedded(true);
    request->setSynchronous(true);

    // Fill args
    QList<QVariant> args;
    args << qVariantFromValue(sf);
    request->setArguments(args);

    bool res = request->send();
    if  (!res) 
        {
        QString errMsg = request->lastErrorMessage();
        }

    // Cleanup
    sf.close();
    delete request;
    
    if(!res)
        return request->lastError();
    else
        return KErrNone;
    }

bool BTMsgViewer::isError(int aError)
    {
    return ((aError < KErrNone)?true:false);
    }
