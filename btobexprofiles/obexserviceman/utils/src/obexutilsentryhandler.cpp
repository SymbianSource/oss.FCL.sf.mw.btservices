/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Handles the global progress dialog 
*
*/




#include <msvstore.h>
#include <mmsvattachmentmanager.h>

#include "obexutilsentryhandler.h"
#include "obexutilsdebug.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CObexutilsEntryhandler()
// ---------------------------------------------------------------------------
//
CObexutilsEntryhandler::CObexutilsEntryhandler(): CActive ( EPriorityNormal )
    {
    CActiveScheduler::Add(this);
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CObexutilsEntryhandler::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// NewL()
// ---------------------------------------------------------------------------
//
CObexutilsEntryhandler* CObexutilsEntryhandler::NewL()
    {
    CObexutilsEntryhandler* self = CObexutilsEntryhandler::NewLC();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// NewLC()
// ---------------------------------------------------------------------------
//
CObexutilsEntryhandler* CObexutilsEntryhandler::NewLC()
    {
    CObexutilsEntryhandler* self = new( ELeave ) CObexutilsEntryhandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// AddLinkAttachment()
// ---------------------------------------------------------------------------
//
TInt CObexutilsEntryhandler::AddEntryAttachment(
    const TDesC &aFilePath, 
    CMsvAttachment* anAttachInfo, 
    CMsvStore* aStore)
    {
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::AddEntryAttachment()"));       
    
    iStatus = KRequestPending;
  
    TRAPD(error, DoAddEntryAttachmentL(aFilePath, anAttachInfo, aStore));
    if (error != KErrNone )
        {        
        //Complete request
        TRequestStatus* status = &iStatus;
        User::RequestComplete(status, error);
        }
    
    SetActive();
    iSyncWaiter.Start();
    
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::AddEntryAttachment() Done"));
    return iStatus.Int();
    }


// ---------------------------------------------------------------------------
// DoAddLinkAttachmentL()
// ---------------------------------------------------------------------------
//
void CObexutilsEntryhandler::DoAddEntryAttachmentL(
    const TDesC &aFilePath, 
    CMsvAttachment* anAttachInfo, 
    CMsvStore* aStore)
    {
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::DoAddEntryAttachmentL()"));   
        
    aStore->AttachmentManagerL().AddLinkedAttachmentL(aFilePath,anAttachInfo, iStatus);
    
    //Complete request
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, KErrNone);
        
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::DoAddEntryAttachmentL() completed"));  
    }

// ---------------------------------------------------------------------------
// UpdateLinkAttachment()
// ---------------------------------------------------------------------------
//
TInt CObexutilsEntryhandler::UpdateEntryAttachment(
    TFileName& aFileName,
    CMsvAttachment* anOldAttachInfo,
    CMsvAttachment* aNewAttachInfo,
    CMsvStore* aStore)
    {
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::UpdateEntryAttachment()"));       
    
    iStatus = KRequestPending;
  
    TRAPD(error, DoUpdateEntryAttachmentL(aFileName,anOldAttachInfo, aNewAttachInfo, aStore));
    if (error != KErrNone )
        {        
        //Complete request
        TRequestStatus* status = &iStatus;
        User::RequestComplete(status, error);
        }
       
    SetActive();
    iSyncWaiter.Start();
          
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::UpdateEntryAttachment() Done"));
    return iStatus.Int();
    }

// ---------------------------------------------------------------------------
// DoUpdateEntryAttachmentL()
// ---------------------------------------------------------------------------
//
void CObexutilsEntryhandler::DoUpdateEntryAttachmentL(
    TFileName& aFileName,
    CMsvAttachment* anOldAttachInfo,
    CMsvAttachment* aNewAttachInfo,
    CMsvStore* aStore)
    {
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::DoUpdateEntryAttachmentL()"));   
    aStore->AttachmentManagerL().RemoveAttachmentL(anOldAttachInfo->Id(), iStatus);
    aStore->AttachmentManagerL().AddLinkedAttachmentL(aFileName,aNewAttachInfo, iStatus);
   
    //Complete request
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, KErrNone);
        
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::DoUpdateEntryAttachmentL() completed"));  
    }

// ---------------------------------------------------------------------------
// From class CActive.
// RunL()
// ---------------------------------------------------------------------------
//
void CObexutilsEntryhandler::RunL()
    {
    if ( iSyncWaiter.IsStarted() )
        {
        iSyncWaiter.AsyncStop();
        }
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::RunL() Done"));           
    }

// ---------------------------------------------------------------------------
// ~CObexutilslinkhandler()
// ---------------------------------------------------------------------------
//
CObexutilsEntryhandler::~CObexutilsEntryhandler()
    {
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::Destructor"));     
    Cancel();    
    }

// ---------------------------------------------------------------------------
// From class CActive.
// DoCancel()
// ---------------------------------------------------------------------------
//
void CObexutilsEntryhandler::DoCancel()
    {
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::DoCancel()"));           
    if ( iSyncWaiter.IsStarted() )
        {
        iSyncWaiter.AsyncStop();
        }
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::DoCancel() done"));               
    }
