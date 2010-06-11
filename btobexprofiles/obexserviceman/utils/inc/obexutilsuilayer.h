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


#ifndef COBEXUTILSUILAYER_H
#define COBEXUTILSUILAYER_H

//  INCLUDES
#include <e32base.h>
#include <msvapi.h>
#include <data_caging_path_literals.hrh> 
#include <AknsUtils.h> 
#include <eikdialg.h>
#include <Obexutils.rsg>

// CONSTANTS

const TUid KUidMsgTypeBt                 = {0x10009ED5};
const TInt KObexUtilsMaxChar             = 80;
const TInt KObexUtilsMaxCharToFromField  = 256;

// Literals for resource location (drive, directory, file)
_LIT( KObexUtilsFileDrive, "z:");
_LIT( KObexUtilsResourceFileName, "obexutils.rsc" );

// Icon file
_LIT( KCommonUiBitmapFile, "MUIU.MBM" );

// Cover display
const TInt KEnumStart = 1;  // start of enumerations; start after ECmdNone
const TInt KResourceNumberMask = 0x00000FFF;
const TInt KFirstResourceOffset = (R_IR_CONNECTING & KResourceNumberMask);

class CMsvOperation;

// DATA TYPES

enum TContextMedia
    {
    EBluetooth,
    EInfrared,
	ENfc
    };

/**
* Backup status.
* The value is controlled by FileManager
*/
enum TFileManagerBkupStatusType
    {
    EFileManagerBkupStatusUnset   = 0x00000000,
    EFileManagerBkupStatusBackup  = 0x00000001,
    EFileManagerBkupStatusRestore = 0x00000002
    };

// CLASS DECLARATION

/**
*  Utility methods for UI related functionality.
*  
*/
NONSHARABLE_CLASS(  TObexUtilsUiLayer )
    {
    public: // New functions
    
        typedef CArrayPtr<CFbsBitmap> CBitmapArray;
        
        /**
        * Launches an editor application for the given message.
        * @param aMessage The message to be launched in an application.
        * @param aSession A message server session.
        * @param aObserverRequestStatus Request status of the observer.
        * @return MSV operation
        */
        IMPORT_C static CMsvOperation* LaunchEditorApplicationOperationL( 
        	CMsvSession& aMsvSession,
        	CMsvEntry* aMessage,
            TRequestStatus& aObserverRequestStatus );

        /**
        * Launches an editor application for the given message.
        * @param aMessage The message to be launched in an application.
        * @param aSession A message server session.
        * @return Symbian OS errorcode.
        */
        IMPORT_C static TInt LaunchEditorApplicationL( CMsvEntry* aMessage,
                                                       CMsvSession& aSession );

        /**
         * LaunchFileManager by specific path and sort method
         * @Since S60 v5.0
         * @param aPath The directory where file manager should open
         * @param aSortMethod sort method to sort the files in aPath
         * @param isEmbeddedMode indicates start mode for file manager or standalone mode
         * @return None 
         */ 
        IMPORT_C static void LaunchFileManagerL( TDesC& aPath, TInt aSortMethod, TBool isEmbeddedMode);
        
             
        /**
         * Open the file by Launching the suitable S60 application  
         * @Since S60 v5.0
         * @return None
         */
        IMPORT_C static void LaunchEditorApplicationL (TMsvId& aMsvIdParent);
        
        /**
        * Shows an error note.
        * @param aResourceID A resource id for the note.
        * @return None.
        */
        IMPORT_C static void ShowErrorNoteL( const TInt& aResourceID );

        /**
        * Shows an information note.
        * @param aResourceID A resource id for the note.
        * @return None.
        */
        IMPORT_C static void ShowInformationNoteL( const TInt& aResourceID );
        
        /**
        * Reads contents of a resource into a buffer.
        * @parma aBuf The buffer.
        * @param aResourceID The id of the resource
        * @return None.
        */
        IMPORT_C static void ReadResourceL( TDes& aBuf,
                                            const TInt& aResourceID );
         
        /**
        * Shows an global information note.
        * @param aResourceID A resource id for the note.
        * @return None.
        */                                    
        IMPORT_C static void ShowGlobalConfirmationQueryL( const TInt& aResourceID );

        /**
         * Show global conformation query without animations or tones.
         * @Since S60 5.0
         * @aResourceID aREsourceID for loc string
         * @return TBool
         */
        IMPORT_C static TBool ShowGlobalConfirmationQueryPlainL( const TInt& aResourceID);

        /**
         * Show global conformation query
         * @Since S60 5.0
         * @aResourceID aREsourceID for loc string
         * @aFilePath location for those files received.
         * @return TBool
         */
        IMPORT_C static TBool ShowGlobalFileOpenConfirmationQueryL( const TInt& aResourceID, const TDesC& aFilePath);
        

        
        /**
        * Returns a resource id for a not supported operation.
        * @return The resource id.
        */
        IMPORT_C static TInt OperationNotSupported();
        
        /**
        * Returns an icon for the given context.
        * @param aContext The context.
        * @param aMedia The used media.
        * @return The resource id of the icon.
        */
        IMPORT_C static TInt ContextIcon( const TMsvEntry& aContext,
                                          TContextMedia aMedia );

        /**
        * Updates bitmaps accoding to given media.
        * @param aMedia The used media.
        * @param aNumberOfZoomStates The media.
        * @param aBitmapFile The bitmap file.
        * @param aStartBitmap The resource id of the start bitmap.
        * @param aEndBitmap The resource id of the start bitmap.
        * @return None.
        */
        IMPORT_C static void UpdateBitmaps( TUid aMedia, 
                                            TInt& aNumberOfZoomStates, 
                                            TFileName& aBitmapFile,
                                            TInt& aStartBitmap,
                                            TInt& aEndBitmap );

        /**
        * Create icons according to given media
        * @param aMedia The used media.
        * @param aIconArray The IconArray used by the caller
        */
        IMPORT_C static void CreateIconsL( TUid aMedia, CArrayPtr<CBitmapArray>* aIconArrays);  

        /**
         * Checks if backup process is running 
         */
        IMPORT_C TBool static IsBackupRunning();
        
        /**
        * Prepares dialog for execution
        * @param aResourceID Resource ID of the dialog
        * @param aDialog Dialog
        */
        void static PrepareDialogExecuteL( const TInt& aResourceID, CEikDialog* aDialog );
        
        /**
        * Check if cover display is enabled
        * return True if enabled
        */
        TBool static IsCoverDisplayL();
        
        /**
         * Check if process with given id is active now
         * return True if is active
         */
        TBool static ProcessExists( const TSecureId& aSecureId );
        
        /**
         * A dummy class for opening CMsvSession.
         */
         class CDummySessionObserver : public CBase , public MMsvSessionObserver
             {
             public:
                 void HandleSessionEventL( TMsvSessionEvent/*aEvent*/,
                     TAny* /*aArg1*/,
                     TAny* /*aArg2*/,
                     TAny* /*aArg3*/ ) {};
             };
    };

#endif      // COBEXUTILSUILAYER_H
            
// End of File
