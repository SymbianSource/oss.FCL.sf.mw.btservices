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


#ifndef COBEXUTILSLAUNCHWAITER_H
#define COBEXUTILSLAUNCHWAITER_H

//  INCLUDES
#include <e32base.h>
#include <apparc.h>

#include <AknServerApp.h>
#include <msvapi.h>
#include <obexutilsdialog.h>

class CDocumentHandler;

// CLASS DECLARATION

/**
*  A class waiting for editing of an embedded document to complete.
*/

NONSHARABLE_CLASS( CObexUtilsLaunchWaiter ) : public CMsvOperation, public MAknServerAppExitObserver
	{
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
	    static CObexUtilsLaunchWaiter* NewLC( 
        	CMsvSession& aMsvSession,
        	CMsvEntry* aMessage,
            TRequestStatus& aObserverRequestStatus );

        /**
        * Two-phased constructor.
        */
    	static CObexUtilsLaunchWaiter* NewL( 
        	CMsvSession& aMsvSession,
        	CMsvEntry* aMessage,
            TRequestStatus& aObserverRequestStatus );

        /**
        * Destructor.
        */
        ~CObexUtilsLaunchWaiter();

    public: // Functions from base classes		
       
        /**
        * From MAknServerAppExitObserve Editing has completed.
        * @param TInt The exit mode including document state.
        * @return None.
        */
        void HandleServerAppExit(TInt aReason);

        /**
        * From CActive A request has been completed.
        * @return None.
        */
	    void RunL();

        /**
        * From CActive A request has been cancelled.
        * @return None.
        */
	    void DoCancel();
	    
        /**
        * ProgressL
        * @return TDesC8&, progress
        */
        virtual const TDesC8& ProgressL();

    private:

        /**
        * C++ default constructor.
        */
	    CObexUtilsLaunchWaiter(
        	CMsvSession& aMsvSession,
        	CMsvEntry* aMessage,
            TRequestStatus& aObserverRequestStatus );
            
        void ConstructL( CMsvEntry* aMessage );
        
        /**
         * Locate the file used to fix the broken link in Inbox
         * @param aFileName the file including full path user selects  ( on return )
         * @param anOldFileName old file name including full path saved in attachment.
         * @return TBool 
         */
        TBool LocateFileL(TFileName& aFileName, const TFileName& anOldFileName);
        
        /**
         * Launch Selection dialog for user to locate the file
         * @param aFileName the file including full path user selects  ( on return )
         * @param anOldFileName old file name excluding full path saved in attachment.
         * @return TBool
         */
        TBool LaunchFileSelectionDialogL(TFileName& aFileName, const TFileName& anOldFileName);
        
        /**
         * Check the drive if available.
         * @param aDriveNumber enum TDriveNumber defined in f32file.h
         * @return Symbian error code
         */
        TInt CheckDriveL(TDriveNumber aDriveNumber); 
        
        /**
         * Check if the file is saved in memory card.
         * @param aFileName full path and name of the file 
         * @return  TDriveNumber if saved in E or F drive; 
         *          otherwise KErrNotFound. 
         */
        TInt CheckIfSaveInMMC(const TFileName& aFileName);

    private:
        CDocumentHandler* iDocumentHandler;
	};

#endif      // COBEXUTILSLAUNCHWAITER_H

// End of File
