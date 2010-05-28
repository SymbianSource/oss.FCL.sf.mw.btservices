/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Obex Server image receiver module implementation
*
*/


// INCLUDE FILES
#include    <avkon.hrh>                    // AVKON components
#include    "BIPController.h"
#include    "BIPCapabilityHandler.h"
#include    "BIPImageHandler.h"


#include    <e32base.h>
#include    <StringLoader.h>               // Series 60 localisation stringloader
#include    <obexutilsmessagehandler.h>
#include    <obexutilsuilayer.h>
#include    <obexutilsdialog.h>
#include    <UiklafInternalCRKeys.h>
#include    <Obexutils.rsg>
#include    <sysutil.h>
#include    <bautils.h>
#include    <driveinfo.h>                   
#include    <AknWaitDialog.h>
#include    <btengdomaincrkeys.h> 
#include    <e32math.h> 
#include    <es_sock.h>
#include    <bt_sock.h>
#include    <msvids.h>
#include    "debug.h"
        
// CONSTANTS
_LIT8(KBipCapabilityType, "x-bt/img-capabilities\0");


const TInt    KBufferSize = 0x10000;  // 64 kB

// ================= MEMBER FUNCTIONS =======================

CBIPController* CBIPController::NewL()
    {
    CBIPController* self = new ( ELeave ) CBIPController();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop(self); 
	return self;
    }
    
// ---------------------------------------------------------
// CBIPController()
// ---------------------------------------------------------
//
CBIPController::CBIPController()
    {
    }

// ---------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------
//
void CBIPController::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iBIPImageHandler = CBIPImageHandler::NewL();
    iLowMemoryActiveCDrive = CObexUtilsPropertyNotifier::NewL(this, ECheckPhoneMemory);
    iLowMemoryActiveMMC = CObexUtilsPropertyNotifier::NewL(this, ECheckMMCMemory);
    iDevMan = CBTEngDevMan::NewL(this);
    iResultArray = new(ELeave) CBTDeviceArray(1);
    // Get default folder from CenRep 
    TObexUtilsMessageHandler::GetCenRepKeyStringValueL(KCRUidBluetoothEngine, KLCReceiveFolder, iCenRepFolder);
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// ~CBIPController()
// ---------------------------------------------------------
//
CBIPController::~CBIPController()
    {   
    TRACE_FUNC_ENTRY    
    delete iGetObject;    
    delete iBIPCapabilityHandler;    
    delete iBIPImageHandler;    
    delete iLowMemoryActiveCDrive;    
    delete iLowMemoryActiveMMC;    
    delete iBuf;
    delete iProgressDialog;
    delete iWaitDialog;
    delete iBTObject;
    delete iDevMan;
    if (iResultArray)
        {
        iResultArray->ResetAndDestroy();
        delete iResultArray;
        }
    iFs.Close();
    TRACE_FUNC_EXIT    
    }

// ---------------------------------------------------------
// ErrorIndication()
// ---------------------------------------------------------
//
void CBIPController::ErrorIndication(TInt TRACE_ONLY(aError))
    {
    TRACE_FUNC_ENTRY
    TRACE_ERROR((_L("[obexreceiveservicebip] CBIPController: ErrorIndication error:\t %d"), aError));
    HandleError(EFalse); // false because this is not an explicit abort
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// AbortIndication()
// ---------------------------------------------------------
//
void CBIPController::AbortIndication()
    {
    TRACE_FUNC_ENTRY
    HandleError(ETrue); // true because this is an explicit abort
    TRACE_FUNC_EXIT
    }

void CBIPController::HandleError(TBool aAbort)
    {
    TRACE_FUNC_ENTRY
    
    if( iBTTransferState == ETransferPut || (!aAbort && iBTTransferState == ETransferPutDiskError) )
        {
        if(iBTObject)
            {
            iBTObject->Reset();
            }
        CancelTransfer();
        TRAP_IGNORE(TObexUtilsUiLayer::ShowGlobalConfirmationQueryL( R_BT_FAILED_TO_RECEIVE));
        }
    delete iBuf;
    iBuf = NULL;
    
    iBTTransferState = ETransferIdle;
    TRAP_IGNORE(TObexUtilsMessageHandler::RemoveInboxEntriesL(iBTObject,iMsvIdParent));
    TRAP_IGNORE(TObexUtilsMessageHandler::RemoveTemporaryRFileL(iFullPathFilename));
    
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CancelTransfer()
// ---------------------------------------------------------
//
void CBIPController::CancelTransfer()
    {
    TRACE_FUNC_ENTRY
    CloseReceivingIndicator();
    if(iBTTransferState == ETransferPut)
        {
        iBTTransferState = ETransferPutCancel;
        }
    else // go to idle for all other states
        {
        iBTTransferState = ETransferIdle;
        }
    }
// ---------------------------------------------------------
// TransportUpIndication()
// ---------------------------------------------------------
//
void CBIPController::TransportUpIndication()
    {
    TRACE_FUNC
    if (!iFs.Handle())
        {
        TRACE_INFO( (_L( "[bipreceiveservice] TransportUpIndication iFs.Connect()" )) ); 
        if (iFs.Connect())   // error value not checked, iFs.Handle() checked one more time before first useage
            {
            TRACE_INFO( (_L( "[bipreceiveservice] TransportUpIndication iFs.Connect() failed" )) ); 
            }
        }
    
    iFile = RFile();
    iFullPathFilename.Zero();
    iCapabilityFileName.Zero();
    }

// ---------------------------------------------------------
// ObexConnectIndication()
// ---------------------------------------------------------
//
TInt CBIPController::ObexConnectIndication( const TObexConnectInfo& /*aRemoteInfo*/, const TDesC8& /*aInfo*/ )
    {
    TRACE_FUNC
    
    // Get remote device socket address and bluetooth name
    // Remote bluetooth name will be displayed in the new message in inbox.
    //
    TSockAddr addr;
    iBTObexServer->RemoteAddr(addr);
    TBTDevAddr tBTDevAddr = static_cast<TBTSockAddr>(addr).BTAddr();
    
    TBTRegistrySearch nameSearch;
    nameSearch.FindAddress(tBTDevAddr);
    
    iResultArray->Reset();
    // Ignore any errors here, if we don't get the name, we don't get the name.
    // It is also possible that the name is received too late....
    static_cast<void>(iDevMan->GetDevices(nameSearch, iResultArray));
    
    return KErrNone;
    }

// ---------------------------------------------------------
// ObexDisconnectIndication()
// ---------------------------------------------------------
//
void CBIPController::ObexDisconnectIndication(const TDesC8& /*aInfo*/)
    {
    TRACE_FUNC
    }

// ---------------------------------------------------------
// TransportDownIndication()
// ---------------------------------------------------------
//
void CBIPController::TransportDownIndication()
    {
    TRACE_FUNC   
    // Remove receiving buffer and files used during file receiving.
    //
    delete iBTObject;
    iBTObject = NULL;
    TRAP_IGNORE(TObexUtilsMessageHandler::RemoveTemporaryRFileL (iFullPathFilename));  
    iFs.Close();
    }

// ---------------------------------------------------------
// PutRequestIndication()
// ---------------------------------------------------------
//
CObexBufObject* CBIPController::PutRequestIndication()
    {   
    TRACE_FUNC_ENTRY
    iLengthHeaderReceived = EFalse; // New put request so clear header based state
    iBTTransferState = ETransferPut;
    
    // Checking if backup is running now - if backup process is active, then we
    // need to cancel transfer - otherwise phone will freeze during receiving
    // data
    if ( TObexUtilsUiLayer::IsBackupRunning() )
        {
        TRACE_INFO ( _L ("Backup in progress! Canceling incoming transfer."));
        iBTTransferState = ETransferPutInitError;
        return NULL;
        }
        
    TRAPD(err, HandlePutImageRequestL());
    if (err == KErrNone)
        {
        return iBTObject;
        }
    if (iBTTransferState != ETransferPutInitError)
        {
        iBTTransferState = ETransferPutDiskError;
        }
    TRACE_FUNC_EXIT
    return NULL;
    }

// ---------------------------------------------------------
// PutPacketIndication() 
// ---------------------------------------------------------
//
TInt CBIPController::PutPacketIndication()
    {
    TRACE_FUNC_ENTRY
    if(iBTTransferState == ETransferPutCancel)
        {
        // User cancelled the put request, so error the next packet to terminate the put request.
        // BIP considers the Unauthorized error response suitable for this...
        HandleError(ETrue); // reset state and clear up
        return KErrIrObexRespUnauthorized;
        }
    
    if (iBTObject)
        {
        iTotalSizeByte = iBTObject->Length();     // get size of receiving file
        iReceivingFileName = iBTObject->Name();   // get name of receiving file
        
        // Check that capacity is suitable as soon as possible
        if(!iLengthHeaderReceived && iTotalSizeByte > 0)
            {
            iLengthHeaderReceived = ETrue; // total size value is from length header
            TBool capacity = ETrue;
            TRAPD(retTrap, capacity = CheckCapacityL());
            if(retTrap != KErrNone)
                {
                return KErrGeneral;
                }
            if(!capacity)
                {
                TRAP_IGNORE(TObexUtilsUiLayer::ShowGlobalConfirmationQueryL(R_OUT_OF_MEMORY));
                return KErrDiskFull;
                }
            }
        if(iBTObject->Name().Length() > KMaxFileName)
            {
            return KErrAccessDenied;
            }
        if(iBTTransferState == ETransferPutDiskError)
            {
            return KErrDiskFull;
            }
        // successfully received put packet if we reached here
        iBTTransferState = ETransferPut;
        
        // Now we need to either create (in the first instance) or update the dialog on the UI.
        if(ReceivingIndicatorActive())
            {
            UpdateReceivingIndicator();
            }
        else if(!iNoteDisplayed)
            {
            // No note launched yet, so try to launch
            TRAPD(err, LaunchReceivingIndicatorL());
            iNoteDisplayed = (err == KErrNone);
            }
        }
    
    TRACE_FUNC_EXIT
    return KErrNone;
    }

// ---------------------------------------------------------
// PutCompleteIndication() 
// ---------------------------------------------------------
//
TInt CBIPController::PutCompleteIndication()  // Once receive has completed.
    {
    TRACE_FUNC_ENTRY
    TInt retVal = KErrNone;
    if(iBTTransferState == ETransferPutCancel)
        {
        retVal = KErrIrObexRespUnauthorized;
        HandleError(ETrue);
        }
    else
        {
        retVal = HandlePutCompleteIndication();
        iBTTransferState = ETransferIdle;
        CloseReceivingIndicator();
        }
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------
// GetRequestIndication()
// ---------------------------------------------------------
//
CObexBufObject* CBIPController::GetRequestIndication( CObexBaseObject* aRequiredObject )
    {
    TRACE_FUNC_ENTRY
    iBTTransferState = ETransferGet;
    TInt err = RemoveCapabilityObject();
    if (err == KErrNone)
        {
        if( aRequiredObject->Type() == KBipCapabilityType )
            {
            TRAP( err, HandleGetCapabilityRequestL( ) );
            if( err == KErrNone )
                {
                return iGetObject;
                }
            }
        }
    TRACE_FUNC_EXIT
    return NULL;
    }

// ---------------------------------------------------------
// GetPacketIndication()
// ---------------------------------------------------------
//
TInt CBIPController::GetPacketIndication()
    {
    TRACE_FUNC_ENTRY
    return KErrNone;
    }

// ---------------------------------------------------------
// GetCompleteIndication()
// ---------------------------------------------------------
//
TInt CBIPController::GetCompleteIndication()
    {
    TRACE_FUNC_ENTRY
    delete iGetObject;
    iGetObject=NULL;
    TInt err = RemoveCapabilityObject();
    if (err != KErrNone)
        {
        err = KErrGeneral;
        }
    iBTTransferState = ETransferIdle;
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------
// SetPathIndication()
// ---------------------------------------------------------
//
TInt CBIPController::SetPathIndication( const CObex::TSetPathInfo& /*aPathInfo*/, 
                                        const TDesC8& /*aInfo*/)
    {
    TRACE_FUNC
    // SetPath is not implemented in BIP - so following IrOBEX guidance, return
    // the Forbidden response code.
    return KErrIrObexRespForbidden;
    }

/**
* This function is implementation for mixin-class for Obexutils.
* It is called every time another instance modifies disk status,
* for example when it cross warning/critical level.
* Parameters:
* @param aCheckType             Disktype changes.
*/
void CBIPController::HandleNotifyL(TMemoryPropertyCheckType aCheckType)
    {    
    TRACE_FUNC_ENTRY    
    // Only interested on this notification if we are receiving something
    if ( iBTTransferState == ETransferPut )
        {
       // Check the keys, what has been changed.
       if ( aCheckType == ECheckPhoneMemory )
           {
           if ( SysUtil::FFSSpaceBelowCriticalLevelL( NULL, 0 ) )
               {                
               TRACE_INFO( _L( "[obexreceiveservicebip] CBIPController: Obex Server error diskfull:\t" ) );
               iBTTransferState = ETransferPutDiskError;
               }
           }
       else if ( aCheckType == ECheckMMCMemory )
           {                
           if ( SysUtil::MMCSpaceBelowCriticalLevelL( NULL, 0 ) )
               {                    
               TRACE_INFO( _L( "[obexreceiveservicebip] CBIPController: Obex Server error diskfull:\t" ) );
               iBTTransferState = ETransferPutDiskError;
               }
           }            
        }        
    TRACE_FUNC_EXIT    
    }
    
// ---------------------------------------------------------
// HandleGetCapabilityRequestL()
// ---------------------------------------------------------
//
void CBIPController::HandleGetCapabilityRequestL()
    {
    TRACE_FUNC_ENTRY    
    CBIPCapabilityHandler* capHandler = CBIPCapabilityHandler::NewL();
    CleanupStack::PushL(capHandler);
    capHandler->CreateCapabilityObjectL(iCapabilityFileName);
    delete iGetObject;
    iGetObject = NULL; 
    iGetObject = CObexBufObject::NewL(NULL);
    iGetObject->SetDataBufL(iCapabilityFileName);
    CleanupStack::PopAndDestroy(capHandler);
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// HandlePutImageRequest()
// ---------------------------------------------------------
//
void CBIPController::HandlePutImageRequestL()
    {
    TRACE_FUNC_ENTRY
    
    delete iBTObject;
    iBTObject = NULL;

    if (!iFs.Handle())
        {
        User::Leave(KErrGeneral);
        }
    
    // Assign an initial value to iDrive
    iDrive = GetDriveWithMaximumFreeSpaceL();    
    
    // If iDrive is at critical space level, we immediately show out_of_memory.
    //
    if (SysUtil::DiskSpaceBelowCriticalLevelL(&iFs, 0, iDrive))
        {
        TRAP_IGNORE(TObexUtilsUiLayer::ShowGlobalConfirmationQueryL(R_OUT_OF_MEMORY));
        User::Leave(KErrGeneral);
        }
    
    iBTObject = CObexBufObject::NewL(NULL);    
    
    delete iBuf;
    iBuf = NULL;
    
    TChar driveLetter;
    iDefaultFolder.Zero();
    iFs.DriveToChar(iDrive, driveLetter);
    iDefaultFolder.Append(driveLetter);
    if ( iDrive == EDriveC )
        {
        iDefaultFolder.Append(_L(":\\data\\"));
        }
    else
        {
        iDefaultFolder.Append(_L(":\\"));
        }
    
    iDefaultFolder.Append(iCenRepFolder);
    
    iFile = RFile();
    iFullPathFilename.Zero();
    TObexUtilsMessageHandler::CreateReceiveBufferAndRFileL(iFile, iDefaultFolder, iFullPathFilename, iBuf, KBufferSize);
    User::LeaveIfError(iFile.Open(iFs, iFullPathFilename, EFileWrite));
    TObexRFileBackedBuffer bufferdetails(*iBuf, iFile, CObexBufObject::EDoubleBuffering);
    TRAPD(err, iBTObject->SetDataBufL(bufferdetails));
    if (err != KErrNone)
        {
        iBTTransferState = ETransferPutInitError;
        User::Leave(KErrGeneral); 
        }
        
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------
// HandlePutCompleteIndication()
// ---------------------------------------------------------
//
TInt CBIPController::HandlePutCompleteIndication()
	{
    TRACE_FUNC_ENTRY
    TInt retVal = KErrNone;

    iDefaultFolder.Zero();
    TChar driveLetter;
    if ( iDrive == EDriveC )
        {
        iFs.DriveToChar(iDrive, driveLetter);
        iDefaultFolder.Append(driveLetter);
        iDefaultFolder.Append(_L(":\\data\\"));
        iDefaultFolder.Append(iCenRepFolder);
        }
    else
        {
        iFs.DriveToChar(iDrive, driveLetter);
        iDefaultFolder.Append(driveLetter);
        iDefaultFolder.Append(_L(":\\"));
        iDefaultFolder.Append(iCenRepFolder);
        }
    
    iFullPathFilename.Zero();
    iFullPathFilename.Append(iDefaultFolder);
    TRAP ( retVal, TObexUtilsMessageHandler::SaveFileToFileSystemL(iBTObject,
                                                                   KUidMsgTypeBt,
                                                                   iMsvIdParent,
                                                                   iFullPathFilename,
                                                                   iFile,
                                                                   iRemoteDeviceName));
    if ( retVal == KErrNone)
        {
        TRAP (retVal, TObexUtilsMessageHandler::AddEntryToInboxL(iMsvIdParent, iFullPathFilename));
        }
    
    
    if( retVal != KErrNone )
        {
        TRACE_ERROR((_L( "[obexreceiveservicebip] CBIPController: HandlePutCompleteIndication error:\t %d" ), retVal ) );              
        TRAP( retVal, TObexUtilsMessageHandler::RemoveInboxEntriesL(iBTObject, iMsvIdParent));
        retVal = KErrDiskFull;
        }
    // Even if the object saving fails we must return image handle with error code
    TRAP_IGNORE( iBIPImageHandler->AddImageHandleHeaderL( iBTObexServer ) );            
    TRACE_INFO( _L( "[obexreceiveservicebip] HandlePutCompleteIndication Done\t" ) );
    delete iBTObject;
    iBTObject = NULL;
    delete iBuf;
    iBuf = NULL;
    iPreviousDefaultFolder = iDefaultFolder;  // save the last file path where file is successfully saved to file system.
    iMsvIdParent = KMsvNullIndexEntryId;
    TRACE_FUNC_EXIT     
    return retVal;
	}
// ---------------------------------------------------------
// RemoveCapabilityObject()
// ---------------------------------------------------------
//		
TInt CBIPController::RemoveCapabilityObject()
    {
    TRACE_FUNC_ENTRY
    if ( (iCapabilityFileName.Compare(KNullDesC)) == 0 )
        {
        return KErrNone; 
        }     
    if ( !iFs.Handle() )
        {
        TRACE_INFO( (_L( "[bipreceiveservice] RemoveCapabilityObject return" )) ); 
        return KErrGeneral;
        }               
    iFs.Delete(iCapabilityFileName);        
    iCapabilityFileName=KNullDesC; 
    TRACE_FUNC_EXIT 
    return KErrNone;    
    }
	 
// ---------------------------------------------------------
// CheckCapacity()
// ---------------------------------------------------------
//	    
TBool CBIPController::CheckCapacityL()
    {
    TRACE_FUNC_ENTRY   
    
    iDrive = EDriveZ; // Intialize iDrive to Z
    TInt filesize = iBTObject->Length();
    
    TInt mmcDrive = KDefaultDrive;   // External memory card  
    TInt imsDrive = KDefaultDrive;   // Internal mass storage
    
    User::LeaveIfError(DriveInfo::GetDefaultDrive(DriveInfo::EDefaultMassStorage, imsDrive));
    User::LeaveIfError(DriveInfo::GetDefaultDrive(DriveInfo::EDefaultRemovableMassStorage, mmcDrive));      
     
    TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL imsDrive=%d; mmcDrive=%d\t" ),imsDrive, mmcDrive ) );

    TVolumeInfo volumeInfo;
    TInt err = iFs.Volume(volumeInfo, imsDrive);
    
    // If err != KErrNone, Drive is not available.
    //
    if ( !err )
        {
        // Check capacity on Internal mass storage            
        TRACE_INFO( (_L( "[obexreceiveservicebip] CheckCapacityL Internal mass storage \t" )) );
        if ( !SysUtil::DiskSpaceBelowCriticalLevelL( &iFs, filesize, imsDrive ) )
            {
            iDrive = imsDrive;            
            }
        }
    if ( iDrive == EDriveZ )
        {
        err = iFs.Volume(volumeInfo, mmcDrive);
        if ( !err )
            {
            // Check capacity on memory card    
            TRACE_INFO( (_L( "[obexreceiveservicebip] CheckCapacityL Checking memory card\t" )) );
            if ( !SysUtil::DiskSpaceBelowCriticalLevelL( &iFs, filesize, mmcDrive ) )
                {                    
                iDrive = mmcDrive;
                }   
            }
        }           
    if ( iDrive == EDriveZ )
        {
        TRACE_INFO( (_L( "[obexreceiveservicebip] CheckCapacityL Checking phone memory\t" )) );
        // Phone memory
        if( !SysUtil::DiskSpaceBelowCriticalLevelL( &iFs, filesize, EDriveC ))
            {
            iDrive = EDriveC;
            }
        }
  
    TRACE_INFO( (_L( "[obexreceiveservicebip] CheckCapacityL iDrive = %d\t" ),iDrive ) );                   
    TRACE_FUNC_EXIT
    
    if (iDrive == EDriveZ)
        {
        // If there is no free space for receiving file, we need to set iPreviousDefaultFolder back to iDefaultFolder.
        // In order to show the file receveing dialog correctly.
        iDefaultFolder = iPreviousDefaultFolder;
        return EFalse;
        }
    return ETrue;
    }    


// ---------------------------------------------------------
// IsOBEXActive()
// ---------------------------------------------------------
//
TBool CBIPController::IsOBEXActive()
    {    
    TRACE_FUNC    
    return ETrue;
    }

// ---------------------------------------------------------
// SetMediaType()
// ---------------------------------------------------------
//
void CBIPController::SetMediaType( TSrcsMediaType __DEBUG_ONLY(aMediaType) ) 
    {
    TRACE_FUNC
    // BIP is only for Bluetooth, so that is all we expect
    __ASSERT_DEBUG(aMediaType == ESrcsMediaBT, User::Panic(KBipPanicCategory, EBipPanicNotBluetoothMediaType));
    }

// ---------------------------------------------------------
// SetObexServer()
// ---------------------------------------------------------
//
TInt CBIPController::SetObexServer( CObexServer* aServer)
    {
    TInt retVal=KErrNone;
    if (aServer)
        {
        iBTObexServer=aServer;
        retVal=aServer->Start(this);
        }
    return retVal;
    }    

void CBIPController::LaunchReceivingIndicatorL()
    {
    if(ReceivingIndicatorActive())
        {
        return;
        }
    
    if(iTotalSizeByte > 0)
        {
        iProgressDialog = CGlobalProgressDialog::NewL(this);  
        if(iReceivingFileName.Length() > 0)
            {
            iProgressDialog->ShowProgressDialogNameSizeL(iReceivingFileName, iTotalSizeByte);
            }
        else
            {
            iProgressDialog->ShowProgressDialogL(R_BT_RECEIVING_DATA);
            }
        }
    else
        {
        iWaitDialog = CGlobalDialog::NewL(this);
        iWaitDialog->ShowNoteDialogL(R_BT_RECEIVING_DATA, ETrue);
        }
    }

void CBIPController::UpdateReceivingIndicator()
    {
    if(iProgressDialog)
        {
        iProgressDialog->UpdateProgressDialog(iBTObject->BytesReceived(), iTotalSizeByte);
        }
    // else we are using a wait note, so no "need" to update
    }

void CBIPController::HandleGlobalProgressDialogL( TInt aSoftkey )
    {
    TRACE_FUNC
    
    if(aSoftkey == EAknSoftkeyCancel)
        {
        CancelTransfer();
        }
    else if(aSoftkey == EAknSoftkeyHide)
        {
        CloseReceivingIndicator(EFalse); // Don't reset state as only hiding
        }
    }

void CBIPController::HandleGlobalNoteDialogL( TInt aSoftkey )
    {
    TRACE_FUNC
    
    if( aSoftkey == EAknSoftkeyCancel )
        {
        CancelTransfer();
        }
    else if( aSoftkey == EAknSoftkeyHide)
        {
        CloseReceivingIndicator(EFalse); // Don't reset state as only hiding
        }
    }

void CBIPController::CloseReceivingIndicator(TBool aResetDisplayedState)
    {
    TRACE_FUNC
    if(aResetDisplayedState)
        {
        iNoteDisplayed = EFalse;
        }
    if(iProgressDialog)
        {
        iProgressDialog->ProcessFinished();
        delete iProgressDialog;
        iProgressDialog = NULL;
        }
    if(iWaitDialog)
        {
        iWaitDialog->ProcessFinished();
        delete iWaitDialog;
        iWaitDialog = NULL;
        }
    }
 
 // ---------------------------------------------------------
 // GetDriveWithMaximumFreeSpace()
 // ---------------------------------------------------------
 // 
 TInt CBIPController::GetDriveWithMaximumFreeSpaceL()
     {
     // Get drive with maximum freespace among phone memory, MMC, internal mass storage.
     //
      TRACE_FUNC   
     
     TVolumeInfo volumeInfoC;
     TVolumeInfo volumeInfoE;
     TVolumeInfo volumeInfoF;
     TInt64 max = 0;
     TInt drive = 0;
     
     TInt err = iFs.Volume(volumeInfoC, EDriveC);
    
     if ( !err )
         {
         // set initial values to max and drive.
         max = volumeInfoC.iFree;
         drive = EDriveC;
         }
          
     err = iFs.Volume(volumeInfoE, EDriveE);     
     if ( !err )
         {
         if (volumeInfoE.iFree >= max)
             {
             max = volumeInfoE.iFree;
             drive = EDriveE;             
             }
         
         }
          
     err = iFs.Volume(volumeInfoF, EDriveF);
     if ( !err )
         {
         if (volumeInfoF.iFree >= max)
             {
             max = volumeInfoF.iFree;
             drive = EDriveF;             
             }
         }
     max = 0;
     return drive;
     }
 
 // ----------------------------------------------------------
 // COPPController::HandleDevManComplete
 // Callback from devman
 // ----------------------------------------------------------
 //    
 // 
 void CBIPController::HandleGetDevicesComplete(TInt aErr, CBTDeviceArray* /*aDeviceArray*/)
    {
    if ( aErr == KErrNone )
        {
        if ( iResultArray->Count())
            {
            iRemoteDeviceName.Zero();
            if ( iResultArray->At(0)->FriendlyName().Length() > 0 )
                {
                TRACE_INFO( _L( "[CBIPController] HandleGetDevicesComplete: got friendly name \t" ) );
                iRemoteDeviceName.Copy(iResultArray->At(0)->FriendlyName());
                }
            else
                {
                TRACE_INFO( _L( "[CBIPController] HandleGetDevicesComplete: got devciename name \t" ) );                
                TRAP_IGNORE(iRemoteDeviceName.Copy(BTDeviceNameConverter::ToUnicodeL(iResultArray->At(0)->DeviceName())));
                }
            }
        }
    }
 
//////////////////////////// Global part ////////////////////////////

//  End of File
