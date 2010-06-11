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
#include    "obexutilsdialog.h"
#include    "obexutilsdialogtimer.h"
#include    "obexutilsuilayer.h"
#include    <secondarydisplay/obexutilssecondarydisplayapi.h>
#include    <aknnotewrappers.h>
#include    <eikprogi.h>
#include    <Obexutils.rsg>
#include    <e32def.h>
#include    <bautils.h>
#include    <StringLoader.h>
#include    <featmgr.h>
#include    "obexutilsdebug.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CObexUtilsDialog::CObexUtilsDialog( MObexUtilsDialogObserver* aObserverPtr ) : 
    iDialogObserverPtr( aObserverPtr )
    {
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::NewL
// -----------------------------------------------------------------------------
EXPORT_C CObexUtilsDialog* CObexUtilsDialog::NewL( MObexUtilsDialogObserver* aObserverPtr )
    {
    CObexUtilsDialog* self = new ( ELeave ) CObexUtilsDialog( aObserverPtr );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return( self );
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::NewLC
// -----------------------------------------------------------------------------
EXPORT_C CObexUtilsDialog* CObexUtilsDialog::NewLC( MObexUtilsDialogObserver* aObserverPtr )
    {
    CObexUtilsDialog* self = new ( ELeave ) CObexUtilsDialog( aObserverPtr );
    CleanupStack::PushL( self );
    self->ConstructL();
    return( self );
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::ConstructL
// Symbian OS default constructor can leave.
// -----------------------------------------------------------------------------
void CObexUtilsDialog::ConstructL()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::ConstructL()"));

    if (!iDialogObserverPtr)
        {
        // The observer pointer was not given as an argument.
        //
        User::Leave(KErrArgument);
        }
  
    TFileName fileName;
    fileName += KObexUtilsFileDrive;
    fileName += KDC_RESOURCE_FILES_DIR;
    fileName += KObexUtilsResourceFileName;
    BaflUtils::NearestLanguageFile( CCoeEnv::Static()->FsSession(), fileName );
    iResourceFileId = CCoeEnv::Static()->AddResourceFileL( fileName );

    iCoverDisplayEnabled = IsCoverDisplayL();

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::ConstructL() completed"));
    } 

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
CObexUtilsDialog::~CObexUtilsDialog()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::~CObexUtilsDialog()"));

    CCoeEnv::Static()->DeleteResourceFile( iResourceFileId );
    delete iObexDialogTimer;

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::~CObexUtilsDialog() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::LaunchProgressDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::LaunchProgressDialogL( 
    MObexUtilsProgressObserver* aObserverPtr, TInt aFinalValue, 
    TInt aResId, TInt aTimeoutValue )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchProgressDialogL()"));

    if ( aObserverPtr )
        {
        // The observerPtr was given, so store it and start a timer
        //
        iProgressObserverPtr = aObserverPtr;

        if ( iObexDialogTimer )
            {
            iObexDialogTimer->Cancel();
            delete iObexDialogTimer;
            iObexDialogTimer = NULL;
            }

        iObexDialogTimer = CObexUtilsDialogTimer::NewL( this );
        iObexDialogTimer->SetTimeout( aTimeoutValue );
        }

    iProgressDialogResId = aResId;
   
    iProgressDialog = new( ELeave ) CAknProgressDialog( 
        ( reinterpret_cast<CEikDialog**>( &iProgressDialog ) ), ETrue );
    PrepareDialogExecuteL( aResId, iProgressDialog );
    iProgressDialog->ExecuteLD( R_SENDING_PROGRESS_NOTE );
    
    HBufC* buf = StringLoader::LoadLC( aResId );
    iProgressDialog->SetTextL( buf->Des() );
    CleanupStack::PopAndDestroy( buf );

    iProgressDialog->GetProgressInfoL()->SetFinalValue( aFinalValue );
    iProgressDialog->SetCallback( this );
    if ( iProgressObserverPtr )
        {
        iObexDialogTimer->Tickle();
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchProgressDialogL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::LaunchWaitDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::LaunchWaitDialogL( TInt aResId )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchWaitDialogL()"));

    if ( iWaitDialog || iProgressDialog )
        {
        // Allow only one dialog at a time
        //
        User::Leave( KErrInUse );
        }

    iWaitDialog = new( ELeave ) CAknWaitDialog(
            ( reinterpret_cast<CEikDialog**>( &iWaitDialog ) ), EFalse );
    
    iWaitDialog->SetCallback( this );
    PrepareDialogExecuteL( aResId, iWaitDialog );
    iWaitDialog->ExecuteLD( aResId );

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchWaitDialogL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::CancelWaitDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::CancelWaitDialogL()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::CancelWaitDialogL()"));

    if( iWaitDialog )
        {
        iWaitDialog->SetCallback(NULL);
        iWaitDialog->ProcessFinishedL();
        iWaitDialog = NULL;
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::CancelWaitDialogL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::CancelProgressDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::CancelProgressDialogL()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::CancelProgressDialogL()"));

    if( iProgressDialog )
        {        
        iProgressDialog->SetCallback(NULL);
        iProgressDialog->ProcessFinishedL();
        iProgressDialog = NULL;

        if ( iObexDialogTimer )
            {
            iObexDialogTimer->Cancel();
            delete iObexDialogTimer;
            iObexDialogTimer = NULL;
            }
        }  
        
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::CancelProgressDialogL() completed"));  
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::UpdateProgressDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::UpdateProgressDialogL( TInt aValue, TInt aResId  )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressDialogL()"));

    if ( iProgressDialog )
        {
        iProgressDialog->GetProgressInfoL()->SetAndDraw( aValue );
        
        HBufC* buf = StringLoader::LoadLC( aResId );
        iProgressDialog->SetTextL( buf->Des() );
        iProgressDialog->LayoutAndDraw();
        CleanupStack::PopAndDestroy( buf );
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressDialogL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::DialogDismissedL
// -----------------------------------------------------------------------------
void CObexUtilsDialog::DialogDismissedL( TInt aButtonId )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::DialogDismissedL()"));

    // The dialog has already been deleted by UI framework.
    //
    if( aButtonId == EAknSoftkeyCancel )
        {
        if ( iDialogObserverPtr )
            {
            iDialogObserverPtr->DialogDismissed( aButtonId );
            }

        if ( iObexDialogTimer )
            {
            iObexDialogTimer->Cancel();
            delete iObexDialogTimer;
            iObexDialogTimer = NULL;
            }
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::DialogDismissedL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::UpdateProgressDialog
// -----------------------------------------------------------------------------
void CObexUtilsDialog::UpdateProgressDialog()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressDialog()"));

    TRAPD( ignoredError, UpdateProgressDialogL( 
        iProgressObserverPtr->GetProgressStatus(), iProgressDialogResId ) );
        
    if (ignoredError != KErrNone)
        {
        FLOG(_L("Ignore this error"));
        }

    if ( iObexDialogTimer )
        {
        iObexDialogTimer->Tickle();
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressDialog() completed"));
    }
    
// -----------------------------------------------------------------------------
// CObexUtilsDialog::LaunchQueryDialogL
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CObexUtilsDialog::LaunchQueryDialogL( const TInt& aResourceID )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchQueryDialogL()"));
    
    CAknQueryDialog* dlg = CAknQueryDialog::NewL();
    CleanupStack::PushL( dlg );
    PrepareDialogExecuteL( aResourceID, dlg );
    CleanupStack::Pop( dlg );
    TInt keypress = dlg->ExecuteLD( aResourceID );
    
    return keypress;
    }
    
// -----------------------------------------------------------------------------
// CObexUtilsDialog::ShowNumberOfSendFileL
// -----------------------------------------------------------------------------
//  
    
EXPORT_C void CObexUtilsDialog::ShowNumberOfSendFileL( TInt aSentNum, TInt aTotalNum )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::ShowNumberOfSendFile()"));
    
    CAknInformationNote* myNote = new (ELeave) CAknInformationNote();

    CArrayFix<TInt>* nums = new( ELeave ) CArrayFixFlat<TInt>(3);
    CleanupStack::PushL(nums);
    nums->AppendL(aSentNum);  
    nums->AppendL(aTotalNum);
    CleanupStack::Pop(nums);

    HBufC* stringholder = StringLoader::LoadLC( R_BT_SENT_IMAGE_NUMBER, *nums); 
    PrepareDialogExecuteL( R_BT_SENT_IMAGE_NUMBER, myNote );
    myNote->ExecuteLD( *stringholder );
    CleanupStack::PopAndDestroy( stringholder );
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::PrepareDialogExecuteL
// -----------------------------------------------------------------------------
//
void CObexUtilsDialog::PrepareDialogExecuteL( const TInt& aResourceID, CEikDialog* aDialog )
    {
    if (iCoverDisplayEnabled)
        {
        TInt dialogIndex =
            ((aResourceID & KResourceNumberMask) - KFirstResourceOffset) + KEnumStart;
        aDialog->PublishDialogL( dialogIndex, KObexUtilsCategory );
        }
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::IsCoverDisplayL()
// -----------------------------------------------------------------------------
//
TBool CObexUtilsDialog::IsCoverDisplayL()
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

//  End of File  
