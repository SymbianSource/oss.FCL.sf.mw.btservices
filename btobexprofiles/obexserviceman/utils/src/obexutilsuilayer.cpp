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
#include "obexutilsuilayer.h"
#include "obexutilslaunchwaiter.h"
#include "obexutilsdebug.h"

#include <secondarydisplay/obexutilssecondarydisplayapi.h>
#include <Obexutils.rsg>
#include <aknnotewrappers.h>
#include <AknGlobalConfirmationQuery.h>
#include <eikon.rsg>
#include <avkon.rsg>
#ifdef NO101APPDEPFIXES
#include <muiu.mbg>
#else   //NO101APPDEPFIXES
enum TMuiuConsts
    {
    EMbmMuiuQgn_prop_mce_ir_unread = 16402,
    EMbmMuiuQgn_prop_mce_ir_unread_mask = 16403,
    EMbmMuiuQgn_prop_mce_ir_read = 16404,
    EMbmMuiuQgn_prop_mce_ir_read_mask = 16405,
    EMbmMuiuQgn_prop_mce_bt_unread = 16406,
    EMbmMuiuQgn_prop_mce_bt_unread_mask = 16407,
    EMbmMuiuQgn_prop_mce_bt_read = 16408,
    EMbmMuiuQgn_prop_mce_bt_read_mask = 16409
    };
#endif  //NO101APPDEPFIXES
#include <bautils.h>
#include <featmgr.h>

#include <stringresourcereader.h>
#include <StringLoader.h>

// Launching file manager related header files 
#include <AiwServiceHandler.h> // The AIW service handler
#include <apgcli.h>
#include <apacmdln.h>
#include <AknLaunchAppService.h>  //  Used to launch file manager in embedded mode.
#include <e32property.h> //for checking backup status

//Constants
const TInt KFileManagerUID3 = 0x101F84EB; /// File Manager application UID3
const TInt KUiNumberOfZoomStates = 2;          // second for the mask
const TInt KSortNumMax = 2;
const TInt KNfcUnreadIconIndex = 10;
const TInt KNfcReadIconIndex = 8;


// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::LaunchEditorApplicationOperationL
// -----------------------------------------------------------------------------
//
EXPORT_C CMsvOperation* TObexUtilsUiLayer::LaunchEditorApplicationOperationL( 
	CMsvSession& aMsvSession,
	CMsvEntry* aMessage,
    TRequestStatus& aObserverRequestStatus )
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::LaunchEditorApplicationOperationL()"));
    CObexUtilsLaunchWaiter* waiterOperation = CObexUtilsLaunchWaiter::NewL(
        aMsvSession,
        aMessage, 
        aObserverRequestStatus );
    
    return waiterOperation;
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::LaunchEditorApplication
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TObexUtilsUiLayer::LaunchEditorApplicationL( CMsvEntry* /*aMessage*/,
                                                           CMsvSession& /*aSession*/ )
    {
    // Obsolete
    return KErrNotSupported;
    }


// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::LaunchFileManager
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsUiLayer::LaunchFileManagerL( 
     TDesC& aPath, 
     TInt aSortMethod, 
     TBool isEmbeddedMode )
    {  
    if ( isEmbeddedMode )
        {
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::LaunchFileManager() Embedded mode"));
        
        CAiwGenericParamList* inParams = CAiwGenericParamList::NewLC();
        inParams->AppendL(TAiwGenericParam( EGenericParamDir, TAiwVariant( aPath ) ) );
        inParams->AppendL(TAiwGenericParam( EGenericParamDir, TAiwVariant( aSortMethod ) ) );
       
        CAknLaunchAppService* launchService = CAknLaunchAppService::NewL(TUid::Uid( KFileManagerUID3 ), // Use File Manager app UID directly
                                                                         NULL, 
                                                                         inParams ); 
        CleanupStack::PopAndDestroy( inParams );
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::LaunchFileManager() Embedded mode completed "));
        }
    else
        {
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::LaunchFileManager() "));
        TApaAppInfo appInfo;
        RApaLsSession apaLsSession;
        User::LeaveIfError( apaLsSession.Connect() );
        CleanupClosePushL( apaLsSession );
        User::LeaveIfError( apaLsSession.GetAppInfo( appInfo, TUid::Uid( KFileManagerUID3 ) ) ); // Use File Manager app UID directly
        CApaCommandLine* apaCmdLine = CApaCommandLine::NewLC();
        apaCmdLine->SetExecutableNameL( appInfo.iFullName );
        apaCmdLine->SetCommandL( EApaCommandOpen );
        apaCmdLine->SetDocumentNameL( aPath );
        TBuf8<KSortNumMax> sortMode; 
        sortMode.AppendNum( aSortMethod );
        apaCmdLine->SetTailEndL( sortMode );
        TThreadId dummy;
        User::LeaveIfError( apaLsSession.StartApp( *apaCmdLine, dummy ) );
        CleanupStack::PopAndDestroy( apaCmdLine );
        CleanupStack::PopAndDestroy( &apaLsSession ); 
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::LaunchFileManager() standalone mode completed "));
        }
    }

// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::LaunchEditorApplication
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsUiLayer::LaunchEditorApplicationL (TMsvId& aMsvIdParent)
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::LaunchEditorApplication() "));
    CDummySessionObserver* sessionObs = new( ELeave )CDummySessionObserver;
    CleanupStack::PushL( sessionObs );  //1st push
    CMsvSession* msvSession = CMsvSession::OpenSyncL( *sessionObs ); 
    CleanupStack::PushL( msvSession );  //2nd push
    
    // 1st, 2nd push?
    CMsvEntry* parentEntry = msvSession->GetEntryL(aMsvIdParent);
    CleanupStack::PushL(parentEntry);  // 3th push    
    
    TRequestStatus status = KRequestPending;
    CObexUtilsLaunchWaiter* waiterOperation = CObexUtilsLaunchWaiter::NewL(
                                                                    *msvSession,
                                                                    parentEntry,
                                                                    status);
    CleanupStack::PopAndDestroy(3); // parentEntry, sessionObs, msvSession
    
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::LaunchEditorApplication() completed "));         
    }


// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::ShowErrorNoteL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsUiLayer::ShowErrorNoteL( const TInt& aResourceID )
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ShowErrorNoteL()"));

    TBuf<KObexUtilsMaxChar> textdata;
    ReadResourceL( textdata, aResourceID );
    
    CAknErrorNote* note = new( ELeave )CAknErrorNote( ETrue );
    CleanupStack::PushL( note );
    PrepareDialogExecuteL( aResourceID, note );
    CleanupStack::Pop( note );
    note->ExecuteLD( textdata );

    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ShowErrorNoteL() completed"));

    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::ShowInformationNoteL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsUiLayer::ShowInformationNoteL( const TInt& aResourceID )
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ShowInformationNoteL()"));

    TBuf<KObexUtilsMaxChar> textdata;
    ReadResourceL( textdata, aResourceID );

    CAknInformationNote* note = new( ELeave )CAknInformationNote;
    CleanupStack::PushL( note );
    PrepareDialogExecuteL( aResourceID, note );
    CleanupStack::Pop( note );
    note->ExecuteLD( textdata );

    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ShowInformationNoteL() completed"));
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::ShowGlobalInformationNoteL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsUiLayer::ShowGlobalConfirmationQueryL( const TInt& aResourceID )
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ShowGlobalInformationNoteL()"));

    TBuf<KObexUtilsMaxChar> textdata;
    TRequestStatus status;    
    
    ReadResourceL( textdata, aResourceID );    	    
    CAknGlobalConfirmationQuery* note = CAknGlobalConfirmationQuery::NewLC();        
    
    status=KRequestPending;
    note->ShowConfirmationQueryL(status,
                                textdata, 
                                R_AVKON_SOFTKEYS_OK_EMPTY, 
                                R_QGN_NOTE_ERROR_ANIM,KNullDesC,
                                0,
                                0,
                                CAknQueryDialog::EErrorTone
                                );    
    User::WaitForRequest(status);
    
    CleanupStack::PopAndDestroy(note );    

    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ShowGlobalInformationNoteL() completed"));
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::ShowGlobalConfirmationQueryPlainL
// -----------------------------------------------------------------------------
//
EXPORT_C TBool TObexUtilsUiLayer::ShowGlobalConfirmationQueryPlainL( const TInt& aResourceID)
    {
    CAknGlobalConfirmationQuery* pQ = CAknGlobalConfirmationQuery::NewL();
    CleanupStack::PushL(pQ);
    HBufC* stringholder = NULL;
    stringholder = StringLoader::LoadLC( aResourceID);
    TRequestStatus status = KRequestPending;
    pQ->ShowConfirmationQueryL(status,*stringholder,R_AVKON_SOFTKEYS_YES_NO);
    User::WaitForRequest(status);
    CleanupStack::PopAndDestroy(2);//PQ and Stringholder
    return (status.Int() == EAknSoftkeyYes);        
    }


// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::ShowGlobalFileOpenConfirmationQueryL
// -----------------------------------------------------------------------------
//
EXPORT_C TBool TObexUtilsUiLayer::ShowGlobalFileOpenConfirmationQueryL( const TInt& aResourceID, const TDesC& aFilePath)
    {
    // Read string from resource file
    TFileName fileName;
    fileName += KObexUtilsFileDrive;
    fileName += KDC_RESOURCE_FILES_DIR;
    fileName += KObexUtilsResourceFileName;
    
    CStringResourceReader* stringResourceReader = CStringResourceReader::NewL(fileName);
    CleanupStack::PushL(stringResourceReader);
    const TDesC& resString = stringResourceReader->ReadResourceString(aResourceID);
    RBuf manipString;
    manipString.Assign(resString.AllocL());
    CleanupStack::PopAndDestroy(stringResourceReader);
    manipString.CleanupClosePushL();
    
    // Remove bracket section
    _LIT(KPrefix, "[");
    _LIT(KSuffix, "]");
    TInt prefixPos = manipString.Find(KPrefix); 
    if (prefixPos != KErrNotFound)
        {
        TInt keyLength = 0;
        TInt suffixPos = manipString.Find(KSuffix); 
        keyLength = (manipString.Mid(prefixPos)).Length()-(manipString.Mid(suffixPos)).Length()+1;
        manipString.Delete(prefixPos, keyLength);  
        }
    
    // Replace "%U" string parameter with file path
    _LIT(KString, "%U");
    TInt replacePos = manipString.Find(KString); 
    if( replacePos == KErrNotFound )
        {
        User::Leave(KErrNotFound);
        }
    const TInt minRequiredSize = manipString.Length() - KString().Length() + aFilePath.Length();
    // ensure that the buffer is big enough (otherwise re-alloc)
    if(manipString.MaxLength() < minRequiredSize)
        {
        manipString.ReAllocL(minRequiredSize);
        }
    manipString.Replace(replacePos, KString().Length(), aFilePath);
    
    // Initiate query dialog
    TRequestStatus status = KRequestPending;
    
    CAknGlobalConfirmationQuery* pQ = CAknGlobalConfirmationQuery::NewL();
    CleanupStack::PushL(pQ);
    pQ->ShowConfirmationQueryL(status, manipString, R_AVKON_SOFTKEYS_YES_NO);
    User::WaitForRequest(status);
    CleanupStack::PopAndDestroy(2, &manipString); // pQ, manipString
    return (status.Int() == EAknSoftkeyYes);
    }


// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::ReadResourceL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsUiLayer::ReadResourceL( TDes& aBuf, const TInt& aResourceID )
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ReadResourceL()"));

    RFs fileSession;
    CleanupClosePushL<RFs>( fileSession );
    User::LeaveIfError( fileSession.Connect() );
    
    TFileName fileName;
    fileName += KObexUtilsFileDrive;
    fileName += KDC_RESOURCE_FILES_DIR;
    fileName += KObexUtilsResourceFileName;
    
    BaflUtils::NearestLanguageFile( fileSession, fileName );
    
    RResourceFile resourcefile;
    CleanupClosePushL<RResourceFile>( resourcefile );
    resourcefile.OpenL( fileSession, fileName );
    resourcefile.ConfirmSignatureL( 0 );
    HBufC8* readBuffer = resourcefile.AllocReadLC( aResourceID );
    
    const TPtrC16 ptrReadBuffer( (TText16*) readBuffer->Ptr(),( readBuffer->Length() + 1 ) >> 1 ); 
    HBufC16* textBuffer=HBufC16::NewLC( ptrReadBuffer.Length() );
    *textBuffer = ptrReadBuffer;
    aBuf.Copy( *textBuffer );

    CleanupStack::PopAndDestroy( 4 ); // textBuffer, fileSession, resourcefile, readBuffer
    
    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ReadResourceL() completed"));
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::OperationNotSupported
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TObexUtilsUiLayer::OperationNotSupported()
    {
    return R_EIK_TBUF_NOT_AVAILABLE;
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::ContextIcon
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TObexUtilsUiLayer::ContextIcon( const TMsvEntry& aContext, TContextMedia aMedia )
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ContextIcon()"));

    TInt icon = 0;
    if( aMedia == EBluetooth )
        {
        if( aContext.Unread() )
            {
            icon = EMbmMuiuQgn_prop_mce_bt_unread - EMbmMuiuQgn_prop_mce_ir_unread;
            }
        else
            {
            icon = EMbmMuiuQgn_prop_mce_bt_read - EMbmMuiuQgn_prop_mce_ir_unread;
            }
        }
    else if( aMedia == EInfrared )
        {
        if( aContext.Unread() )
            {
            icon = 0;
            }
        else
            {
            icon = EMbmMuiuQgn_prop_mce_ir_read - EMbmMuiuQgn_prop_mce_ir_unread;
            }
        }
    else if( aMedia == ENfc )
        {
        if( aContext.Unread() )
            {
            icon = KNfcUnreadIconIndex;
            }
        else
            {
            icon = KNfcReadIconIndex;
            }
        }

    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::ContextIcon() completed"));

    return icon;
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::UpdateBitmaps
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsUiLayer::UpdateBitmaps( TUid aMedia, 
    TInt& aNumberOfZoomStates, TFileName& aBitmapFile, TInt& aStartBitmap, 
    TInt& aEndBitmap )
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::UpdateBitmaps()"));

    aBitmapFile = KCommonUiBitmapFile;
    aNumberOfZoomStates = KUiNumberOfZoomStates;
    if( aMedia == KUidMsgTypeBt )
        {
        aStartBitmap = EMbmMuiuQgn_prop_mce_ir_unread;
        aEndBitmap = EMbmMuiuQgn_prop_mce_bt_read_mask;
        }
    else
        {
        aStartBitmap = EMbmMuiuQgn_prop_mce_ir_unread;
        aEndBitmap = EMbmMuiuQgn_prop_mce_bt_unread_mask;
        }

    FLOG(_L("[OBEXUTILS]\t TObexUtilsUiLayer::UpdateBitmaps() completed"));
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::DeleteCBitMapArray
// -----------------------------------------------------------------------------
//
inline void DeleteCBitMapArray(TAny* aPtr)
    {
    if (aPtr) 
        {
        TObexUtilsUiLayer::CBitmapArray* array =
            reinterpret_cast<TObexUtilsUiLayer::CBitmapArray*>(aPtr);    
        array->ResetAndDestroy();
        delete array;
        }
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::CreateAndAppendBitmapL
// -----------------------------------------------------------------------------
//
void CreateAndAppendBitmapL(const TAknsItemID&          aID,
                            const TInt                  aFileBitmapId,
                            CArrayPtr<TObexUtilsUiLayer::CBitmapArray>* aIconArrays     )
    {
    TFileName muiubmpFilename;
    muiubmpFilename += KObexUtilsFileDrive;
    muiubmpFilename += KDC_APP_BITMAP_DIR;
    muiubmpFilename += KCommonUiBitmapFile;

    TObexUtilsUiLayer::CBitmapArray* array=NULL;
    array=new(ELeave) CArrayPtrFlat<CFbsBitmap>(KUiNumberOfZoomStates);
    CleanupStack::PushL(TCleanupItem(DeleteCBitMapArray, array));

    CFbsBitmap* bitmap=0, *mask=0;
    
    //Can not use CreateIconLC since the order in which bitmap and mask are pushed into Cleanup Stack is undefined.
    AknsUtils::CreateIconL(
        AknsUtils::SkinInstance(),
        aID,
        bitmap,
        mask,
        muiubmpFilename,
        aFileBitmapId,
        aFileBitmapId+1);
    CleanupStack::PushL(mask);
    CleanupStack::PushL(bitmap);

    // warning: bmp is deleted by the array CleanupItem. Immediately Pop or risk double deletion upon a Leave.
    array->AppendL(bitmap);
    CleanupStack::Pop(bitmap);

    // warning: bmp is deleted by the array CleanupItem. Immediately Pop or risk double deletion upon a Leave.
    array->AppendL(mask);
    CleanupStack::Pop(mask);

    aIconArrays->AppendL(array);
    CleanupStack::Pop(array);
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::CreateIconsL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsUiLayer::CreateIconsL(
    TUid aMedia,
    CArrayPtr<TObexUtilsUiLayer::CBitmapArray>* aIconArrays )
    {
    if( aMedia == KUidMsgTypeBt ) //Bluetooth 
        {
        CreateAndAppendBitmapL(
            KAknsIIDQgnPropMceIrUnread,
            EMbmMuiuQgn_prop_mce_ir_unread,
            aIconArrays);
        CreateAndAppendBitmapL(
            KAknsIIDQgnPropMceIrRead,
            EMbmMuiuQgn_prop_mce_ir_read,
            aIconArrays);
        CreateAndAppendBitmapL(
            KAknsIIDQgnPropMceBtUnread,
            EMbmMuiuQgn_prop_mce_bt_unread,
            aIconArrays);
        CreateAndAppendBitmapL(
            KAknsIIDQgnPropMceBtRead,
            EMbmMuiuQgn_prop_mce_bt_read,
            aIconArrays);
        CreateAndAppendBitmapL(
            KAknsIIDQgnPropMceNfcRead,
            0,
            aIconArrays);
        CreateAndAppendBitmapL(
            KAknsIIDQgnPropMceNfcUnread,
            0,
            aIconArrays);        
        }
    else //Infrared
        {
        CreateAndAppendBitmapL(
            KAknsIIDQgnPropMceIrUnread,
            EMbmMuiuQgn_prop_mce_ir_unread,
            aIconArrays);
        CreateAndAppendBitmapL(
            KAknsIIDQgnPropMceIrRead,
            EMbmMuiuQgn_prop_mce_ir_read,
            aIconArrays);
        CreateAndAppendBitmapL(
            KAknsIIDQgnPropMceBtUnread,
            EMbmMuiuQgn_prop_mce_bt_unread,
            aIconArrays);
        }
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::IsBackupRunning
// -----------------------------------------------------------------------------
//
EXPORT_C TBool TObexUtilsUiLayer::IsBackupRunning()
    {
    const TUint32 KFileManagerBkupStatus = 0x00000001;
    
    TInt status = EFileManagerBkupStatusUnset;
    TBool retValue = EFalse;
    TInt err = RProperty::Get( TUid::Uid(KFileManagerUID3), KFileManagerBkupStatus,
                              status );
    if ( err == KErrNone )
        {
        if ( status == EFileManagerBkupStatusBackup || 
             status == EFileManagerBkupStatusRestore )
            {
            TSecureId fileManagerSecureId( KFileManagerUID3 );
            //only returning ETrue if backup process is still active
            retValue = ProcessExists( fileManagerSecureId );
            }
        }
   
    return retValue;
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::PrepareDialogExecuteL
// -----------------------------------------------------------------------------
//
void TObexUtilsUiLayer::PrepareDialogExecuteL( const TInt& aResourceID, CEikDialog* aDialog )
    {
    if (IsCoverDisplayL())
        {
        TInt dialogIndex =
            ((aResourceID & KResourceNumberMask) - KFirstResourceOffset) + KEnumStart;
        aDialog->PublishDialogL( dialogIndex, KObexUtilsCategory );
        }
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::IsCoverDisplayL()
// -----------------------------------------------------------------------------
//
TBool TObexUtilsUiLayer::IsCoverDisplayL()
    {
    TBool coverDisplay = EFalse;
	FeatureManager::InitializeLibL();
	if ( FeatureManager::FeatureSupported( KFeatureIdCoverDisplay ) )
		{
		coverDisplay = ETrue;
		}
	FeatureManager::UnInitializeLib();
    return coverDisplay;
    }

// -----------------------------------------------------------------------------
// TObexUtilsUiLayer::ProcessExists
// -----------------------------------------------------------------------------
//
TBool TObexUtilsUiLayer::ProcessExists( const TSecureId& aSecureId )
    {
    _LIT( KFindPattern, "*" );
    TFindProcess finder(KFindPattern);
    TFullName processName;
    while( finder.Next( processName ) == KErrNone )
        {
        RProcess process;
        if ( process.Open( processName ) == KErrNone )
            {
            TSecureId processId( process.SecureId() );
            process.Close();
            if( processId == aSecureId )
                {
                return ETrue;
                }
            }
        }
    return EFalse;
    }
//  End of File  
