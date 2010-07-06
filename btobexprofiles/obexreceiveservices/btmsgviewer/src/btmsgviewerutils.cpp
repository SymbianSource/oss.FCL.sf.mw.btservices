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

#include <mmsvattachmentmanager.h>
#include "btmsgviewerutils.h"


CBtMsgViewerUtils* CBtMsgViewerUtils::NewL()
    {
    CBtMsgViewerUtils* me = new (ELeave) CBtMsgViewerUtils();
    CleanupStack::PushL(me);
    me->ConstructL();
    CleanupStack::Pop(me);
    return me;
    }

CBtMsgViewerUtils::CBtMsgViewerUtils()
    {
    
    }

void CBtMsgViewerUtils::ConstructL()
    {
    iMsvSession = CMsvSession::OpenSyncL(*this);
    }

CBtMsgViewerUtils::~CBtMsgViewerUtils()
    {
    if ( iMsvSession )
        {
		 delete iMsvSession;
        }
    }

HBufC* CBtMsgViewerUtils::GetMessagePath(TInt aMessageId, TInt aError)
    {
    HBufC* fileName = NULL;
    TRAP(aError, fileName = HBufC::NewL(KMaxPath));   
    if(aError < KErrNone)
        {
        return fileName;
        }
    
    TRAP(aError, GetMessagePathL(fileName->Des(), aMessageId));
    return fileName;
    }

void CBtMsgViewerUtils::GetMessagePathL(TPtr aMsgPath, const TInt aMessageId)
    {
    CMsvEntry* messageEntry = iMsvSession->GetEntryL(aMessageId);
    CleanupStack::PushL(messageEntry);
    
    CMsvEntry* attachmentEntry = iMsvSession->GetEntryL((*messageEntry)[0].Id());
    CleanupStack::PushL(attachmentEntry);
    
    CMsvStore* store = attachmentEntry->EditStoreL();
    CleanupStack::PushL(store); 
    
    //get file handle for the attachment & the complete path of the file
    RFile attachmentFile;
    attachmentFile = store->AttachmentManagerL().GetAttachmentFileL(0);
    attachmentFile.FullName(aMsgPath);
    attachmentFile.Close();
    
    //mark attachment as Read
    TMsvEntry attachEntry = attachmentEntry->Entry();
    attachEntry.SetUnread(EFalse);
    attachmentEntry->ChangeL(attachEntry);
    
    CleanupStack::PopAndDestroy(store);
    CleanupStack::PopAndDestroy(attachmentEntry);
    CleanupStack::PopAndDestroy(messageEntry);
    }

void CBtMsgViewerUtils::HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, 
                                                    TAny* aArg2, TAny* aArg3)
    {
    (void) aEvent;
    (void) aArg1;
    (void) aArg2;
    (void) aArg3;
    }



