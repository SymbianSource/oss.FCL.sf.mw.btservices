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
* Description:  Definition of the global progress dialog class
*
*/




#ifndef __OBEXUTILS_GLOBAL_PROGRESS_DIALOG__
#define __OBEXUTILS_GLOBAL_PROGRESS_DIALOG__

//  INCLUDES
#include <e32base.h>
#include <stringresourcereader.h>
#include <Obexutils.rsg>
#include <avkon.rsg>                     // R_QGN_GRAF_WAIT_BAR_ANIM
#include <AknGlobalNote.h>

#include <coecntrl.h>

// FORWARD DECLARATIONS
class CAknGlobalProgressDialog;


// CLASS DECLARATION

// Call back for progress bar
NONSHARABLE_CLASS(  MGlobalProgressCallback )
    {
    public:
        virtual void HandleGlobalProgressDialogL( TInt aSoftkey ) = 0;
    };





NONSHARABLE_CLASS( CGlobalProgressDialog ) : public CActive
{
   public:  // Constructors and destructor
        

      /**
      * Two-phased constructor.
      */
      IMPORT_C static CGlobalProgressDialog* NewLC(MGlobalProgressCallback* aObserver);
      IMPORT_C static CGlobalProgressDialog* NewL(MGlobalProgressCallback* aObserver);


      /**
      * Destructor.
      */
      virtual ~CGlobalProgressDialog();

   public: // New functions
        
      /**
      * Sets the icon for this dialog
      * @param aIconText Text of the icon
      * @param aIconFile File containing icon
      * @param aIconId Icon ID
      * @param aIconMaskId  Icon mask ID
      * @return void
      */
      void SetIconL( const TDesC& aIconText, const TDesC& aIconFile,
                                TInt aIconId = 0, TInt aIconMaskId = -1 );
      /**
      * Sets the image for the dialog
      * @param aImageFile Image filename
      * @param aImageId Image ID
      * @param aImageMaskId Image mask ID
      * @return void
      */
      void SetImageL( const TDesC& aImageFile, TInt aImageId = 0,
                                 TInt aImageMaskId = -1 );

      /**
      * Shows the progress dialog
      * @return void
      */
      IMPORT_C void ShowProgressDialogL(TInt aStringId);
      IMPORT_C void ShowProgressDialogNameSizeL( TDesC& aFileName, 
                                                 TInt64 aFileSize);
      
      
      /**
      * Updates the progress dialog
      * @param aValue progress bar value
      * @param aFinalValue progress bar final value
      * @return void
      */
      IMPORT_C void UpdateProgressDialog(TInt aValue, TInt aFinalValue);

      /**
      * Called to complete the global progress dialog
      * @return void
      */
      IMPORT_C void ProcessFinished();
      

   protected:  // Functions from base classes
        
      /**
      * From CActive Active Object RunL()
      */
      virtual void RunL();

      /**
      * From CActive Active Object DoCancel()
      */
      virtual void DoCancel();


   private:

      /**
      * By default constructor is private.
      */
      void ConstructL(MGlobalProgressCallback* aObserver);

      /**
      * C++ default constructor.
      */
      CGlobalProgressDialog();

   private: //data
        CAknGlobalProgressDialog*  iProgressDialog;
         
        MGlobalProgressCallback*   iKeyCallback;
        CStringResourceReader*  iStringResourceReader;
                
};



// Call back for note with animation and without animation
NONSHARABLE_CLASS(  MGlobalNoteCallback )
    {
    public:
        virtual void HandleGlobalNoteDialogL( TInt aSoftkey ) = 0;
    };



NONSHARABLE_CLASS( CGlobalDialog ) : public CActive
{
   public:  // Constructors and destructor
        

      /**
      * Two-phased constructor.
      */
      IMPORT_C static CGlobalDialog* NewLC(MGlobalNoteCallback* aObserver);
      IMPORT_C static CGlobalDialog* NewL(MGlobalNoteCallback* aObserver);


      /*
       *  public functions
       */
      IMPORT_C void ShowNoteDialogL( TInt aResourceId, TBool anAnimation);
      IMPORT_C void ShowErrorDialogL(TInt aResourceId);            
      IMPORT_C  void ProcessFinished();

      /**
      * Destructor.
      */
      virtual ~CGlobalDialog();


   protected:  // Functions from base classes
        
      /**
      * From CActive Active Object RunL()
      */
      virtual void RunL();

      /**
      * From CActive Active Object DoCancel()
      */
      virtual void DoCancel();


   private:

      /**
      * By default constructor is private.
      */
      void ConstructL(MGlobalNoteCallback* aObserver);

      /**
      * C++ default constructor.
      */
      CGlobalDialog();

   private: //data
        
        MGlobalNoteCallback*   iKeyCallback;
        CStringResourceReader*  iStringResourceReader;
        CAknGlobalNote* iAknGlobalNote;
        TInt iWaitNoteID;

};











#endif  // __OBEXUTILS_GLOBAL_PROGRESS_DIALOG__
