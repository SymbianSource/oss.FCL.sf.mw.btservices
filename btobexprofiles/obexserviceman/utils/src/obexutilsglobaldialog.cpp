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
* Description:  Handles the global progress dialog for voice recognition
*
*/





// INCLUDE FILES

#include "obexutilsglobalprogressdialog.h"
#include "obexutilsuilayer.h"
#include <AknIconUtils.h>
#include <avkon.mbg>
#include <avkon.rsg>
#include <bautils.h>
#include    "obexutilsdebug.h"



// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CGlobalDialog::CGlobalDialog() : CActive(EPriorityNormal)
   {
   CActiveScheduler::Add( this );
   }

// Symbian default constructor can leave.
void CGlobalDialog::ConstructL(MGlobalNoteCallback* aObserver)
   {    
    iKeyCallback = aObserver;
    iAknGlobalNote = CAknGlobalNote::NewL();
   }


// Two-phased constructor.
EXPORT_C CGlobalDialog* CGlobalDialog::NewL(MGlobalNoteCallback* aObserver)
   {
   CGlobalDialog* self = NewLC(aObserver);
   CleanupStack::Pop();
   return self;
   }

// Two-phased constructor.- stack version
EXPORT_C CGlobalDialog* CGlobalDialog::NewLC(MGlobalNoteCallback* aObserver)
   {
   CGlobalDialog* self=new (ELeave) CGlobalDialog();
   CleanupStack::PushL(self);
   self->ConstructL(aObserver);
   return self;
   }

EXPORT_C void CGlobalDialog::ShowErrorDialogL(TInt aResourceId)
{
    TFileName fileName;
    fileName += KObexUtilsFileDrive;
    fileName += KDC_RESOURCE_FILES_DIR;
    fileName += KObexUtilsResourceFileName;
    
    if(!iStringResourceReader)
    {
        iStringResourceReader= CStringResourceReader::NewL( fileName );
    }
    TPtrC buf;
    buf.Set(iStringResourceReader-> ReadResourceString(aResourceId)); 
    iAknGlobalNote->SetSoftkeys(R_AVKON_SOFTKEYS_CLOSE);
    iAknGlobalNote->ShowNoteL(iStatus,EAknGlobalInformationNote, buf);    
    FTRACE( FPrint(_L( "[ObexUtils] CGlobalDialog: ShowNoteDialogL buf: \t %S" ), &buf) );
    SetActive();    
    
}
EXPORT_C void CGlobalDialog::ShowNoteDialogL( TInt aResourceId, TBool anAnimation)
{
    TFileName fileName;
    fileName += KObexUtilsFileDrive;
    fileName += KDC_RESOURCE_FILES_DIR;
    fileName += KObexUtilsResourceFileName;
    
    if(!iStringResourceReader)
    {
        iStringResourceReader= CStringResourceReader::NewL( fileName );
    }
    TPtrC buf;
    buf.Set(iStringResourceReader-> ReadResourceString(aResourceId)); 
    iAknGlobalNote->SetSoftkeys(R_AVKON_SOFTKEYS_CANCEL);
    if(anAnimation)
    {
        iAknGlobalNote->SetAnimation(R_QGN_GRAF_WAIT_BAR_ANIM);
    }
    iAknGlobalNote->ShowNoteL(iStatus,EAknGlobalWaitNote, buf);    
    FTRACE( FPrint(_L( "[ObexUtils] CGlobalDialog: ShowNoteDialogL buf: \t %S" ), &buf) );
    SetActive();

}

// Destructor
CGlobalDialog::~CGlobalDialog()
   {
   Cancel();
   if(iAknGlobalNote)
   {
       delete iAknGlobalNote;   
       iAknGlobalNote = NULL;
   }
   
   delete iStringResourceReader;
   
   }



// ---------------------------------------------------------
// CGlobalDialog::DoCancel
// Active object cancel
// ---------------------------------------------------------
//
void CGlobalDialog::DoCancel()
   {
      ProcessFinished();
    if(iStringResourceReader)
    {
        delete iStringResourceReader;
        iStringResourceReader = NULL;
    }
   }

// ---------------------------------------------------------
// CGlobalDialog::RunL
// Active object RunL
// ---------------------------------------------------------
//
void CGlobalDialog::RunL()
    {

    FTRACE( FPrint(_L( "[ObexUtils] CGlobalDialog: RunL iStatus.Int():\t %d" ), iStatus.Int() ) );
    if ( iKeyCallback != NULL )
        {
        iKeyCallback->HandleGlobalNoteDialogL(iStatus.Int());
        }
    }

// ---------------------------------------------------------
// CGlobalDialog::ProcessFinished
// Stops the progress dialog
// ---------------------------------------------------------
//
EXPORT_C void CGlobalDialog::ProcessFinished()
   { 
    FLOG( _L( "[ObexUtils] CGlobalDialog::ProcessFinished\t" ) );   
    delete iAknGlobalNote;
    iAknGlobalNote = NULL;   
   }

