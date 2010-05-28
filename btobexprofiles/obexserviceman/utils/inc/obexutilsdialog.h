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


#ifndef COBEXUTILSDIALOG_H
#define COBEXUTILSDIALOG_H

//  INCLUDES
#include    <e32base.h>
#include    <AknWaitDialog.h>

// FORWARD DECLARATIONS
class CAknWaitDialog;
class CObexUtilsDialogTimer;

// CLASS DECLARATION

/**
*  An observer interface for asking progress status of an operation using 
*  a progress dialog.
*/
NONSHARABLE_CLASS(  MObexUtilsProgressObserver )
    {
    public:

        /**
        * Returns the progress status of the operation.
        * @since 2.6
        * @return A progress value relative to final value.
        */
        virtual TInt GetProgressStatus() = 0;
    };

// CLASS DECLARATION

/**
*  An observer interface for informing about dialog events.
*/
NONSHARABLE_CLASS(  MObexUtilsDialogObserver )
    {
    public:

        /**
        * Informs the observer that a dialog has been dismissed.
        * @since 2.6
        * @param aButtonId The button that was used to dismiss the dialog.
        * @return None.
        */
        virtual void DialogDismissed( TInt aButtonId ) = 0;
    };


// CLASS DECLARATION

/**
*  A class for launching and managing dialogs.
*/
NONSHARABLE_CLASS( CObexUtilsDialog ) : public CBase, public MProgressDialogCallback
    {
    public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CObexUtilsDialog* NewL( 
            MObexUtilsDialogObserver* aObserverPtr );        
        
        IMPORT_C static CObexUtilsDialog* NewLC( 
            MObexUtilsDialogObserver* aObserverPtr );

        /**
        * Destructor.
        */
        virtual ~CObexUtilsDialog();
   
    public: // New functions
        
        /**
        * Launches a progress dialog.
        * @since 2.6
        * @param aObserverPtr A pointer to progress observer. A NULL pointer if 
                              the progress dialog is updated manually.
        * @param aFinalValue The final value of the operation (progress=100%).
        * @param aResId A resource id for the string to be shown in the dialog.
        * @param aTimeoutValue A value telling how often should the dialog be
                               updated. Relevant only if observer given.
        * @return None.
        */
        IMPORT_C void LaunchProgressDialogL( 
            MObexUtilsProgressObserver* aObserverPtr, TInt aFinalValue, 
            TInt aResId, TInt aTimeoutValue );    
        
        /**
        * Launches a wait dialog.
        * @since 2.6
        * @param aResId A resource id for the string to be shown in the dialog.
        * @return None.
        */
        IMPORT_C void LaunchWaitDialogL( TInt aResId );
        
        /**
        * Cancels a wait dialog if one exists.
        * @since 2.6        
        * @return None.
        */
        IMPORT_C void CancelWaitDialogL();
        
        /**
        * Cancels a wait progress dialog if one exists.
        * @since 2.6       
        * @return None.
        */
        IMPORT_C void CancelProgressDialogL();
        
        /**
        * Updates a progress dialog. Should not be used if the 
        * MObexUtilsDialogObserver pointer was given.
        * @since 2.6
        * @param aValue A progress value relative to final value.
        * @param aResId A resource id for the string to be shown in the dialog.
        * @return None
        */
        IMPORT_C void UpdateProgressDialogL( TInt aValue, TInt aResId );
        
        /**
        * Show a query note
        * @param aResourceID A resource id for the note.
        * @return User's input - Yes/No
        */
        IMPORT_C TInt LaunchQueryDialogL( const TInt& aResourceID );
        
        /**
        * Show how many files are sent in case not all images are supported
        * @param aSentNum Number of sent files
        * @param aTotlNum Number of total files
        * return None.
        */
        
        IMPORT_C void ShowNumberOfSendFileL( TInt aSentNum, TInt aTotalNum );

        /**
        * Prepares dialog for execution
        * @param aResourceID Resource ID of the dialog
        * @param aDialog Dialog
        */
        void PrepareDialogExecuteL( const TInt& aResourceID, CEikDialog* aDialog );

        /**
        * Check if cover display is enabled
        * return True if enabled
        */
        TBool IsCoverDisplayL();

    public: // New functions (not exported)

        /**
        * Updates the progress dialog.
        * @return None.
        */
        void UpdateProgressDialog();

    private: // Functions from base classes

        /**
        * From MProgressDialogCallback A dialog has been dismissed.
        * @param aButtonId The button that was used to dismiss the dialog.
        * @return None.
        */
        void DialogDismissedL( TInt aButtonId );

    private:
        TInt ExecuteDialogL( const TInt& aResourceID, CEikDialog* aDialog );

    private:

        /**
        * C++ default constructor.
        */
        CObexUtilsDialog( MObexUtilsDialogObserver* aObserverPtr );
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

    private: // Data

        CAknProgressDialog*         iProgressDialog;
        CAknWaitDialog*             iWaitDialog;
        CObexUtilsDialogTimer*      iObexDialogTimer;
        TInt                        iResourceFileId;
        TInt                        iProgressDialogResId;
        TBool                       iCoverDisplayEnabled;

        // Not Owned
        //
        MObexUtilsProgressObserver* iProgressObserverPtr;
        MObexUtilsDialogObserver*   iDialogObserverPtr;
    };

#endif      // COBEXUTILSDIALOG_H
            
// End of File
