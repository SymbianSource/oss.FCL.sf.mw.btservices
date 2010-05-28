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
* Description: 
*
*/


// INCLUDE FILES
#include <eikenv.h>
#include <DocumentHandler.h>
#include <mmsvattachmentmanager.h>
#include <msvids.h>
#include <bautils.h>
#include <AknCommonDialogsDynMem.h>     // for memory and file selection dialogs
#include <CommonDialogs.rsg>
#include <pathinfo.h>                   // for getting drive root path
#include <Obexutils.rsg>
#include <AknGlobalNote.h>
#include <StringLoader.h>
#include <AiwGenericParam.h>
#include "obexutilslaunchwaiter.h"
#include "obexutilsdebug.h"
#include "obexutilsuilayer.h"            // For launching file manager
#include "obexutilsmessagehandler.h"            // For updating an entry

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::NewLC
// -----------------------------------------------------------------------------
CObexUtilsLaunchWaiter* CObexUtilsLaunchWaiter::NewLC( 
	CMsvSession& aMsvSession,
	CMsvEntry* aMessage,
    TRequestStatus& aObserverRequestStatus )
    {
    CObexUtilsLaunchWaiter* self = new( ELeave )CObexUtilsLaunchWaiter( 
        aMsvSession,
        aMessage, 
        aObserverRequestStatus );
    CleanupStack::PushL( self );
    self->ConstructL( aMessage );
    return self;
    }

// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::NewL
// -----------------------------------------------------------------------------
CObexUtilsLaunchWaiter* CObexUtilsLaunchWaiter::NewL(
	CMsvSession& aMsvSession,
	CMsvEntry* aMessage,
    TRequestStatus& aObserverRequestStatus )
    {
    CObexUtilsLaunchWaiter* self = CObexUtilsLaunchWaiter::NewLC(
        aMsvSession,
        aMessage, 
        aObserverRequestStatus );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::ConstructL
// -----------------------------------------------------------------------------
void CObexUtilsLaunchWaiter::ConstructL( CMsvEntry* aMessage )
    {
    if (aMessage->Count() < 1)
        {
        User::Leave(KErrOverflow);
        }
    
    CMsvEntry* attachEntry = iMsvSession.GetEntryL(((*aMessage)[0]).Id());
    CleanupStack::PushL(attachEntry);  // 1st push
    CMsvStore* store = attachEntry->ReadStoreL();
    CleanupStack::PushL(store);  // 2nd push
    
    CMsvAttachment* attachInfo = store->AttachmentManagerL().GetAttachmentInfoL(0);
    CleanupStack::PushL(attachInfo); // 3rd push
          
    TDataType dataType = attachInfo->MimeType();
    TFileName filePath;
    filePath = attachInfo->FilePath();
  
    TInt error = KErrNone;
    TBool isCompleteSelf = EFalse;      
    CEikonEnv* eikEnv = CEikonEnv::Static();

    if ( attachInfo->Type() == CMsvAttachment::EMsvFile )
        {
        RFile attachFile;        
        TRAP( error, attachFile = store->AttachmentManagerL().GetAttachmentFileL(0));
        if ( error == KErrNone )
            {
            CleanupClosePushL(attachFile);  // 4th push          
            CAiwGenericParamList* paramList = CAiwGenericParamList::NewLC();  // 5th push
            TAiwGenericParam paramSave(EGenericParamAllowSave, ETrue);
            paramList->AppendL( paramSave );          
            
            if ( eikEnv )
                {               
                iDocumentHandler = CDocumentHandler::NewL( eikEnv->Process() );
                iDocumentHandler->SetExitObserver( this );
                TRAP( error, iDocumentHandler->OpenFileEmbeddedL(attachFile, dataType, *paramList));               
                }// eikEnv        
            CleanupStack::PopAndDestroy(2); // paramList, attachFile
            }
        }// EMsvFile
    
    if ( attachInfo->Type() == CMsvAttachment::EMsvLinkedFile )
        {
        CAiwGenericParamList* paramList = CAiwGenericParamList::NewLC();  // 4th push
        TAiwGenericParam paramSave(EGenericParamFileSaved, ETrue);
        paramList->AppendL( paramSave );
        
        if ( eikEnv )
            {            
            iDocumentHandler = CDocumentHandler::NewL( eikEnv->Process() );
            iDocumentHandler->SetExitObserver( this );
            RFs rfs;
            User::LeaveIfError( rfs.Connect() );
            if ( BaflUtils::FileExists( rfs, filePath ) )                                 
                {
                RFile64 shareableFile;
                TRAP( error, iDocumentHandler->OpenTempFileL(filePath,shareableFile));
                if ( error == KErrNone)
                    {
                    TRAP( error, iDocumentHandler->OpenFileEmbeddedL( shareableFile, dataType, *paramList));
                    }
                shareableFile.Close();
                
                if ( error == KErrNotSupported )  
                    {                    
                    delete iDocumentHandler;
                    iDocumentHandler = NULL;
                    
                    const TInt sortMethod = 2;  // 0 = 'By name', 1 = 'By type', 
                                                // 2 = 'Most recent first' and 3 = 'Largest first'
                    TRAP (error, TObexUtilsUiLayer::LaunchFileManagerL( filePath, 
                                                                        sortMethod, 
                                                                        ETrue )); // ETrue -> launch file manager in embedded mode.
                    isCompleteSelf = ETrue;
                    }  // KErrNotSupported
                }            
            else 
                {
                error = KErrNone;
                TFileName fileName;
                if (LocateFileL(fileName, filePath))
                    {
                    // Update the entry
                    TRAP(error, TObexUtilsMessageHandler::UpdateEntryAttachmentL(fileName,aMessage));
                    if ( error == KErrNone )
                        {
                        // Show a confirmation note
                        CAknGlobalNote* note = CAknGlobalNote::NewLC();
                        HBufC* stringholder  = StringLoader::LoadLC( R_BT_SAVED_LINK_UPDATED );
                        note->ShowNoteL(EAknGlobalConfirmationNote, *stringholder);
                        CleanupStack::PopAndDestroy(2); //note and stringholder
                        }            
                    }    
                isCompleteSelf = ETrue;
                }  
           
            rfs.Close();
            } // eikEnv
        
        CleanupStack::PopAndDestroy(); // paramList                                     
        } // EMsvLinkedFile
     
    
    // Set message to READ     
    TMsvEntry entry = aMessage->Entry();
    entry.SetUnread( EFalse );
    aMessage->ChangeL( entry );
    
    User::LeaveIfError ( error );
    CleanupStack::PopAndDestroy(3); //  attachInfo, store, attachEntry        
    
    iObserverRequestStatus = KRequestPending;  // CMsvOperation (observer)
    iStatus = KRequestPending;  // CMsvOperation
    SetActive();

    if ( isCompleteSelf )
        {
        HandleServerAppExit( error );
        }
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
CObexUtilsLaunchWaiter::~CObexUtilsLaunchWaiter()
    {
    Cancel();
    if (iDocumentHandler)
        {
        delete iDocumentHandler;
        iDocumentHandler = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::RunL
// -----------------------------------------------------------------------------
void CObexUtilsLaunchWaiter::RunL()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::RunL()"));
    
    TRequestStatus* status = &iObserverRequestStatus;
    User::RequestComplete( status, KErrNone );

    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::RunL() completed"));
    }
    
// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::DoCancel
// -----------------------------------------------------------------------------
void CObexUtilsLaunchWaiter::DoCancel()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::DoCancel()"));
   
    if ( iStatus == KRequestPending )
        {
        TRequestStatus* pstat = &iStatus;
        User::RequestComplete( pstat, KErrCancel );
        }
      
    if( iObserverRequestStatus == KRequestPending )
        {
        TRequestStatus* observerStatus = &iObserverRequestStatus;
        User::RequestComplete( observerStatus, KErrCancel );
        }

    
    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::DoCancel() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::CObexUtilsLaunchWaiter
// -----------------------------------------------------------------------------
CObexUtilsLaunchWaiter::CObexUtilsLaunchWaiter( 
	CMsvSession& aMsvSession,
	CMsvEntry* /*aMessage*/,
    TRequestStatus& aObserverRequestStatus )
    :
    CMsvOperation(aMsvSession, EPriorityStandard, aObserverRequestStatus),
    iDocumentHandler(NULL)
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::CObexUtilsLaunchWaiter()"));

    CActiveScheduler::Add( this );

    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::CObexUtilsLaunchWaiter() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::HandleServerAppExit
// -----------------------------------------------------------------------------
void CObexUtilsLaunchWaiter::HandleServerAppExit(TInt aReason)
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::HandleServerAppExit()"));

    if( iStatus == KRequestPending )
        {
        // Complete self
        //
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, aReason );
        }

	MAknServerAppExitObserver::HandleServerAppExit( aReason );	
    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::HandleServerAppExit() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::ProgressL
// -----------------------------------------------------------------------------
const TDesC8& CObexUtilsLaunchWaiter::ProgressL()
    {
    return KNullDesC8;
    }

// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::LocateFile
// -----------------------------------------------------------------------------
TBool CObexUtilsLaunchWaiter::LocateFileL(TFileName& aFileName, const TFileName& anOldFileName)
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::LocateFile()"));
    
    TBuf<200> buf;
    TRequestStatus status = KRequestPending;
    
    TParse fileParseOld;
    fileParseOld.Set(anOldFileName, NULL, NULL);
    TFileName oldName = fileParseOld.NameAndExt();
            
    // check old link if the file saved in mmc. If so, check if mmc available. 
    // if unavailable, show "is File not found as memory card is not present"
    //
    TInt err = CheckIfSaveInMMC( anOldFileName );
    TBool showMMCOut = EFalse;
    if( err == EDriveF)
        {
        if( CheckDriveL(EDriveF) )
            showMMCOut = ETrue;
        }
    else if( err == EDriveE )
        {
        if( CheckDriveL(EDriveE) )
            showMMCOut = ETrue;
        }

    TBool answer = EFalse;
    if( showMMCOut )
        {
        answer = TObexUtilsUiLayer::ShowGlobalConfirmationQueryPlainL(R_BT_SAVED_NO_MEMORY_CARD);
        }
    else
        {
        answer = TObexUtilsUiLayer::ShowGlobalConfirmationQueryPlainL(R_BT_SAVED_SEARCH);
        }
   
    TBool updateLink = EFalse;
    if ( answer )
        {
        updateLink = LaunchFileSelectionDialogL(aFileName, oldName);               
        }  
    
    FLOG(_L("[OBEXUTILS]\t CObexUtilsLaunchWaiter::LocateFile() completed"));
    return updateLink;
    }
    
// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::LaunchFileSelectionDialogL
// -----------------------------------------------------------------------------
TBool CObexUtilsLaunchWaiter::LaunchFileSelectionDialogL(
    TFileName& aFileName, 
    const TFileName& anOldName)
    {
    TBuf<200> buf;
    TParse fileParseNew;
    TFileName nameNew;
    TBool updateLink = EFalse;
   
    while ( !updateLink ) 
        {
        TBool isSelected =  AknCommonDialogsDynMem::RunSelectDlgLD( AknCommonDialogsDynMem::EMemoryTypeMMCExternal|
                                                                    AknCommonDialogsDynMem::EMemoryTypeMMC|
                                                                    AknCommonDialogsDynMem::EMemoryTypeInternalMassStorage|
                                                                    AknCommonDialogsDynMem::EMemoryTypePhone,
                                                                    aFileName,
                                                                    R_CFD_DEFAULT_SELECT_MEMORY_SELECTION,
                                                                    R_CFD_DEFAULT_SELECT_FILE_SELECTION );
        
        if ( isSelected )
            {
            fileParseNew.Set(aFileName, NULL, NULL);
            nameNew = fileParseNew.NameAndExt();
            
            if ( nameNew.Compare(anOldName)) // names do not match
                {
                updateLink =  TObexUtilsUiLayer::ShowGlobalConfirmationQueryPlainL(R_BT_SAVED_SEARCH_UPDATE);                        
                }
            else
                {
                updateLink = ETrue;
                }       
            }
        else
            {
            break;  // Exit from while loop.
            }
        }
  
    return updateLink;
    }

// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::CheckDriveL
// -----------------------------------------------------------------------------
TInt CObexUtilsLaunchWaiter::CheckDriveL(TDriveNumber aDriveNumber)
    {
    RFs rfs;
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL( rfs ) ;
    TVolumeInfo volumeInfo;
    TInt err = rfs.Volume(volumeInfo, aDriveNumber);
    CleanupStack::PopAndDestroy();    // rfs
    
    return err;
    }
// -----------------------------------------------------------------------------
// CObexUtilsLaunchWaiter::CheckIfSaveInMMC
// -----------------------------------------------------------------------------
TInt CObexUtilsLaunchWaiter::CheckIfSaveInMMC(const TFileName& aFileName)
    {
    if(aFileName.Find(_L("F:")) != KErrNotFound)
        return EDriveF;
    if(aFileName.Find(_L("E:")) != KErrNotFound)
        return EDriveE;

    return KErrNotFound;
    }
//  End of File  
