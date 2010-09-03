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
#include <apgcli.h>
#include <txtrich.h>
#include <bioscmds.h>
#include <biocmtm.h>
#include <mtclreg.h>
#include <hbsymbianvariant.h>
#include "btmsgviewerutils.h"

const TInt32 KUidMsgTypeBtTInt32 = 0x10009ED5;
#define KBioUidValue  0x10001262
const TUid KBioUidValueUid =
{KBioUidValue};

    
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
    
    delete iMimeType;
    }

HBufC* CBtMsgViewerUtils::GetMessagePath(TInt aMessageId, TInt& aError)
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

void CBtMsgViewerUtils::GetMessagePathL(TPtr aMsgPath, const TInt aMessageId )
    {
    CMsvEntry* messageEntry = iMsvSession->GetEntryL(aMessageId);
    CleanupStack::PushL(messageEntry);
    
    TMsvEntry entry = messageEntry->Entry();
    if(entry.MtmData1() == KUidMsgTypeBtTInt32)
        {
        CMsvStore* store = messageEntry->ReadStoreL();
        CleanupStack::PushL(store);
        
        TInt attachmentCount = store->AttachmentManagerL().AttachmentCount();
        RFile attachmentFile;
        if(attachmentCount != 0)
            {
            //get file handle for the attachment & the complete path of the file
            attachmentFile = store->AttachmentManagerL().GetAttachmentFileL(0);
            CleanupClosePushL(attachmentFile);
            User::LeaveIfError(attachmentFile.FullName(aMsgPath));
            CleanupStack::PopAndDestroy(&attachmentFile);
            StoreMessageMimeTypeL(aMsgPath);
            
            //mark attachment as Read
            TMsvEntry attachEntry = messageEntry->Entry();
            attachEntry.SetUnread(EFalse);
            messageEntry->ChangeL(attachEntry);
            }
        else
            {
            CClientMtmRegistry* mtmReg = CClientMtmRegistry::NewL(*iMsvSession);
            CleanupStack::PushL(mtmReg); //1st Push
            
            CBIOClientMtm* bioClientMtm = (CBIOClientMtm*) mtmReg->NewMtmL(KBioUidValueUid);
            CleanupStack::PushL(bioClientMtm); //2nd Push
            
            bioClientMtm->SwitchCurrentEntryL(aMessageId);
            
            CMsvEntry* attachmentEntry = CMsvEntry::NewL(bioClientMtm->Session(),
                                        aMessageId,
                                        TMsvSelectionOrdering());
            CleanupStack::PushL(attachmentEntry); //3rd Push
            
            bioClientMtm->LoadMessageL();
    
            CMsvEntrySelection* selection = new (ELeave) CMsvEntrySelection;
            CleanupStack::PushL(selection); //4th Push
    
            selection->AppendL(aMessageId);
    
            TBuf8<1> aParameter;
            CMsvOperationActiveSchedulerWait* wait = CMsvOperationActiveSchedulerWait::NewLC();
            //5th Push
    
            CMsvOperation* operation = bioClientMtm->InvokeAsyncFunctionL(KBiosMtmParse, *selection,
                aParameter, wait->iStatus);
    
            wait->Start();
    
            CMsvStore* store = attachmentEntry->ReadStoreL();
            CleanupStack::PushL(store); //6th Push
            MMsvAttachmentManager& attachMan = store->AttachmentManagerL();
            
            attachmentFile = attachMan.GetAttachmentFileL(0);
            CleanupClosePushL(attachmentFile); //7th Push
            User::LeaveIfError(attachmentFile.FullName(aMsgPath));
            CleanupStack::PopAndDestroy(&attachmentFile);
            StoreMessageMimeTypeL(aMsgPath);
            
            //mark attachment as Read
            TMsvEntry attachEntry = attachmentEntry->Entry();
            attachEntry.SetUnread(EFalse);
            attachmentEntry->ChangeL(attachEntry);
            
            delete operation;
            CleanupStack::PopAndDestroy(6);
            }
        CleanupStack::PopAndDestroy(store);
        }
    else
        {
        CMsvEntry* attachmentEntry = iMsvSession->GetEntryL((*messageEntry)[0].Id());
        CleanupStack::PushL(attachmentEntry); //1st push
            
        CMsvStore* store = attachmentEntry->ReadStoreL();
        CleanupStack::PushL(store);  //2nd push
        
        TBuf<KMaxPath> attachmentName(store->AttachmentManagerL().GetAttachmentInfoL(0)->AttachmentName());
        RFs fs;
        User::LeaveIfError(fs.Connect());
        CleanupClosePushL(fs);
        TEntry entry;
        User::LeaveIfError(fs.Entry(attachmentName, entry));
        CleanupStack::PopAndDestroy(&fs);
        
        //get file handle for the attachment & the complete path of the file
        RFile attachmentFile;
        attachmentFile = store->AttachmentManagerL().GetAttachmentFileL(0);
        CleanupClosePushL(attachmentFile); //3rd Push
        User::LeaveIfError(attachmentFile.FullName(aMsgPath));
        CleanupStack::PopAndDestroy(&attachmentFile);
        StoreMessageMimeTypeL(aMsgPath);
        
        //mark attachment as Read
        TMsvEntry attachEntry = attachmentEntry->Entry();
        attachEntry.SetUnread(EFalse);
        attachmentEntry->ChangeL(attachEntry);
        
        CleanupStack::PopAndDestroy(store);
        CleanupStack::PopAndDestroy(attachmentEntry);
        }
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

void CBtMsgViewerUtils::StoreMessageMimeTypeL(TPtr aMsgPath)
    {
    RFs rfs;
    RFile file;
    
    User::LeaveIfError(rfs.Connect());
    
    User::LeaveIfError(rfs.ShareProtected());
    
    User::LeaveIfError(file.Open(rfs, aMsgPath, EFileShareReadersOrWriters | EFileRead));
    
    TDataRecognitionResult dataType;
    RApaLsSession apaSession;
    
    if(apaSession.Connect() == KErrNone)
        {
        if (apaSession.RecognizeData(file, dataType) == KErrNone)
            {
            if(iMimeType)
                {
                delete iMimeType;
                iMimeType = NULL;
                }
        
            iMimeType = dataType.iDataType.Des8().AllocL();
            
            rfs.Close();
            apaSession.Close();
            }
        }

    rfs.Close();
    }

HBufC8* CBtMsgViewerUtils::GetMimeType()
    {
    return iMimeType;
    }
