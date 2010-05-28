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





// INCLUDE FILES
#include <AknGlobalProgressDialog.h>
#include "obexutilsglobalprogressdialog.h"
#include "obexutilsuilayer.h"
#include <StringLoader.h>
#include <AknIconUtils.h>
#include <avkon.mbg>
#include <avkon.rsg>
#include <bautils.h>
//#include <BtuiViewResources.rsg>     // Compiled resource ids
#include <e32math.h>

#include <StringLoader.h>            // Localisation stringloader
#include <eikenv.h>

#include "obexutilsdebug.h"

const TInt KMaxDisplayFileName = 17;    // used while showing receiving filename. 
                                        // If the filename is longer than 20 bytes, we make it show only 20 bytes.

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CGlobalProgressDialog::CGlobalProgressDialog() : CActive(EPriorityNormal)
   {
   CActiveScheduler::Add( this );
   }


// Symbian default constructor can leave.
void CGlobalProgressDialog::ConstructL(MGlobalProgressCallback* aObserver)
   {
    iProgressDialog = CAknGlobalProgressDialog::NewL();
    iKeyCallback = aObserver;
   }


// Two-phased constructor.
EXPORT_C CGlobalProgressDialog* CGlobalProgressDialog::NewL(MGlobalProgressCallback* aObserver)
   {
   CGlobalProgressDialog* self = NewLC(aObserver);
   CleanupStack::Pop();
   return self;
   }

// Two-phased constructor.- stack version
EXPORT_C CGlobalProgressDialog* CGlobalProgressDialog::NewLC(MGlobalProgressCallback* aObserver)
   {
   CGlobalProgressDialog* self=new (ELeave) CGlobalProgressDialog();
   CleanupStack::PushL(self);
   self->ConstructL(aObserver);
   return self;
   }


// Destructor
CGlobalProgressDialog::~CGlobalProgressDialog()
   {
   Cancel();
   delete iProgressDialog;  
   delete iStringResourceReader;
   }



// ---------------------------------------------------------
// CGlobalProgressDialog::DoCancel
// Active object cancel
// ---------------------------------------------------------
//
void CGlobalProgressDialog::DoCancel()
   {
   if ( iProgressDialog )
      {
      iProgressDialog->CancelProgressDialog();
      }

    if(iStringResourceReader)
    {
        delete iStringResourceReader;
        iStringResourceReader = NULL;
    }
   }

// ---------------------------------------------------------
// CGlobalProgressDialog::RunL
// Active object RunL
// ---------------------------------------------------------
//
void CGlobalProgressDialog::RunL()
    {
    if ( iKeyCallback != NULL )
        {
        iKeyCallback->HandleGlobalProgressDialogL(iStatus.Int());
        }
    }

// ---------------------------------------------------------
// CGlobalProgressDialog::SetIconL
// Set icon on the dialog
// ---------------------------------------------------------
//
void CGlobalProgressDialog::SetIconL( const TDesC& aIconText, const TDesC& aIconFile,
                                TInt aIconId, TInt aIconMaskId )
   {
   iProgressDialog->SetIconL(aIconText, aIconFile, aIconId, aIconMaskId );
   }

// ---------------------------------------------------------
// CGlobalProgressDialog::SetImageL
// Set image on the dialog
// ---------------------------------------------------------
//
void CGlobalProgressDialog::SetImageL( const TDesC& aImageFile, TInt aImageId,
                                 TInt aImageMaskId )
   {
   iProgressDialog->SetImageL(aImageFile, aImageId, aImageMaskId);
   }


// ---------------------------------------------------------
// CGlobalProgressDialog::ShowProgressDialogL
// Shows progress dialog and sets active object active
// ---------------------------------------------------------
//
EXPORT_C void CGlobalProgressDialog::ShowProgressDialogL(TInt aStringId)
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
    //buf.Set(iStringResourceReader-> ReadResourceString(R_BT_RECEIVING_DATA));   
    buf.Set(iStringResourceReader-> ReadResourceString(aStringId));   
    iProgressDialog->ShowProgressDialogL( iStatus, buf, R_AVKON_SOFTKEYS_HIDE_CANCEL__HIDE );   
    //iProgressDialog->ShowProgressDialogL( iStatus, buf, R_BTUI_SOFTKEYS_OPTIONS_EXIT__CHANGE ); //R_OBEXUTILS_SOFTKEYS_HIDE_CANCEL );   
    SetActive();    
    }

// ---------------------------------------------------------
// CGlobalProgressDialog::ShowProgressDialogNameSizeL
// Shows progress dialog and sets active object active
// ---------------------------------------------------------
//
EXPORT_C void CGlobalProgressDialog::ShowProgressDialogNameSizeL(
    TDesC& aFileName, 
    TInt64 aFileSize)
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
   
    
    TBuf<20> sizeInString;
    sizeInString.Zero();
    
    if ( aFileSize >> 20 )    // size in MB
        {       
        TReal sizeInMB = 0;
        sizeInMB = ((TReal)aFileSize ) / (1024*1024);
        _LIT16(KFormatTwoDecimal,"%4.2f");  // keep 2 decimals
        sizeInString.Format(KFormatTwoDecimal,sizeInMB); 
        buf.Set(iStringResourceReader-> ReadResourceString(R_BT_IR_RECEIVING_DATA_SIZE_MB));
        }
    else if( aFileSize >> 10 )        // size in KB
        {
        TInt64 sizeInKB = 0;
        sizeInKB = aFileSize >> 10;
        sizeInString.AppendNum(sizeInKB); 
        buf.Set(iStringResourceReader-> ReadResourceString(R_BT_IR_RECEIVING_DATA_SIZE_KB));
        }
   else                              // size is unknown or less than 1K
        {
        buf.Set(iStringResourceReader-> ReadResourceString(R_BT_IR_RECEIVING_DATA_NO_SIZE));
        }
    
    
    TBuf<100> tbuf;
    tbuf.Zero();
    tbuf.Append(buf);
    _LIT(KPrefix,"[");
    _LIT(Ksuffix,"]");
    TInt prefixPos = tbuf.Find(KPrefix); 
    if (prefixPos!= KErrNotFound)
        {
        TInt keyLength = 0;
        TInt suffixPos = tbuf.Find(Ksuffix); 
        keyLength =(tbuf.Mid(prefixPos)).Length()-(tbuf.Mid(suffixPos)).Length()+1;
        tbuf.Delete(prefixPos, keyLength);  
        }
    _LIT(KString,"%U");
    

        
    if ( aFileName.Length() > KMaxDisplayFileName )   
        {
        // Filename is too long, 
        // We make it shorter. Hiding the chars in the middle part of filename.       
        //
        TFileName shortname;
        shortname = aFileName.Mid(0,KMaxDisplayFileName/2);
        shortname.Append(_L("..."));
        shortname.Append(aFileName.Mid(aFileName.Length() - KMaxDisplayFileName/2, KMaxDisplayFileName/2));
        tbuf.Replace(tbuf.Find(KString) , 2, shortname);
        }
    else
        {
        tbuf.Replace(tbuf.Find(KString) , 2, aFileName);
        }
    _LIT(KInt, "%N");
    if ( sizeInString.Length() > 0 )
        {
        tbuf.Replace(tbuf.Find(KInt) , 2, sizeInString);
        }
    
    iProgressDialog->ShowProgressDialogL( iStatus, tbuf, R_AVKON_SOFTKEYS_HIDE_CANCEL__HIDE );
    SetActive();
    }
// ---------------------------------------------------------
// CGlobalProgressDialog::UpdateProgressDialog
// Updates the progress dialog
// ---------------------------------------------------------
//
EXPORT_C void CGlobalProgressDialog::UpdateProgressDialog(TInt aValue, TInt aFinalValue)
   {
   iProgressDialog->UpdateProgressDialog(aValue, aFinalValue);   
   }

// ---------------------------------------------------------
// CGlobalProgressDialog::ProcessFinished
// Stops the progress dialog
// ---------------------------------------------------------
//
EXPORT_C void CGlobalProgressDialog::ProcessFinished()
   {
   iProgressDialog->ProcessFinished();
   }
