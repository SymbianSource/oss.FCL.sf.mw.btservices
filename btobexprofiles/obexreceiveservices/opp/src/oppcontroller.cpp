/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of CBtListenActive
*
*/


// INCLUDE FILES
#include    <avkon.hrh>                    // AVKON components
#include    "oppcontroller.h"
#include    "btengdevman.h"
#include    <obexutilsmessagehandler.h>
#include    "debug.h"
#include    <Obexutils.rsg>
#include    <bautils.h>
#include    <UiklafInternalCRKeys.h>
#include    <obexutilsuilayer.h>
#include    <btengdomaincrkeys.h>
#include    <AiwServiceHandler.h> // The AIW service handler
#include    <sysutil.h>
#include    <btengdomaincrkeys.h> 
#include    <msvids.h>
#include    <driveinfo.h> 
#include    <es_sock.h>
#include    <bt_sock.h>

// CONSTANTS

const TInt    KBufferSize = 0x10000;  // 64 kB

// ================= MEMBER FUNCTIONS =======================


COPPController* COPPController::NewL()
    {
    COPPController* self = new ( ELeave ) COPPController();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	return self;
    }
    
COPPController::COPPController()
	{
    TRACE_FUNC
	}

void COPPController::ConstructL()	
	{
    TRACE_FUNC
    iObexTransferState = ETransferIdle;
    iLowMemoryActiveCDrive = CObexUtilsPropertyNotifier::NewL(this, ECheckPhoneMemory);
    iLowMemoryActiveMMC = CObexUtilsPropertyNotifier::NewL(this, ECheckMMCMemory);        
    iDevMan=CBTEngDevMan::NewL(this);  
    iResultArray = new (ELeave) CBTDeviceArray(1);
    // Get default folder from CenRep 
    TObexUtilsMessageHandler::GetCenRepKeyStringValueL(KCRUidBluetoothEngine, 
                                                       KLCReceiveFolder,
                                                       iCenRepFolder);
	} 

COPPController::~COPPController()
    {
    TRACE_FUNC
    delete iObexObject;
    delete iBuf;
    delete iLowMemoryActiveCDrive;
    delete iLowMemoryActiveMMC;
    delete iProgressDialog;
    delete iWaitDialog;
    delete iDevMan;
    if (iResultArray)
        {
        iResultArray->ResetAndDestroy();
        delete iResultArray;
        }
    iFs.Close();
    }

// ---------------------------------------------------------
// ErrorIndication()
// ---------------------------------------------------------
//
void COPPController::ErrorIndication( TInt TRACE_ONLY(aError) )
    {
    TRACE_FUNC
    TRACE_ERROR((_L( "[oppreceiveservice] COPPController: ErrorIndication error:\t %d" ),aError));
    HandleError(EFalse); // false because not explicit abort
    }

// ---------------------------------------------------------
// AbortIndication() 
// ---------------------------------------------------------
//
void COPPController::AbortIndication()
	{
	TRACE_FUNC
	HandleError(ETrue); // true because explicit abort
	}

void COPPController::HandleError(TBool aAbort)
    {
    TRACE_ERROR((_L( "[oppreceiveservice] COPPController:HandleError" )));
    if( iObexTransferState == ETransferPut || iObexTransferState == ETransferPutDiskError )
        {        
        if(iObexObject)
            {
            iObexObject->Reset();
            }
        CancelTransfer();
        if(!aAbort)
        	{
			if(iMediaType == ESrcsMediaBT)
				{
				TRAP_IGNORE( TObexUtilsUiLayer::ShowGlobalConfirmationQueryL( R_BT_FAILED_TO_RECEIVE ) );
				}
			TRACE_ASSERT(iMediaType != ESrcsMediaIrDA, KErrNotSupported)
        	}
        }
    delete iBuf;
    iBuf = NULL;
    iObexTransferState = ETransferIdle;
    TRAP_IGNORE(TObexUtilsMessageHandler::RemoveInboxEntriesL(iObexObject, iMsvIdParent));
    TRAP_IGNORE(TObexUtilsMessageHandler::RemoveTemporaryRFileL (iFullPathFilename));
    }

// ---------------------------------------------------------
// TransportUpIndication()
// ---------------------------------------------------------
//
void COPPController::TransportUpIndication()
	{
	TRACE_FUNC    
    iObexTransferState = ETransferIdle;	
 
	if ( !iFs.Handle() )
	    {
	    TRACE_INFO( (_L( "[oppreceiveservice] TransportUpIndication iFs.Connect()" )) ); 
	    if ( iFs.Connect() )   // error value not preserved, iFs.Handle() checked one more time before first useage
	        {
	        TRACE_INFO( (_L( "[oppreceiveservice] TransportUpIndication iFs.Connect() failed" )) ); 
	        }
	    }

    iFile = RFile();
    iFullPathFilename.Zero();
	}

// ---------------------------------------------------------
// ObexConnectIndication()
// ---------------------------------------------------------
//
TInt COPPController::ObexConnectIndication( const TObexConnectInfo& /*aRemoteInfo*/,
                                            const TDesC8& /*aInfo*/)
    {
    TRACE_FUNC  
    if ( iMediaType == ESrcsMediaBT )
        {
        TRACE_INFO( _L( "[oppreceiveservice] ObexConnectIndication: BT media \t" ) );
        
        // Get remote device socket address and bluetooth name
        // Remote bluetooth name will be displayed in the new message in inbox.
        //
        TSockAddr addr;
        iObexServer->RemoteAddr(addr);
        TBTDevAddr tBTDevAddr = static_cast<TBTSockAddr>(addr).BTAddr();
        
        TBTRegistrySearch nameSearch;
        nameSearch.FindAddress(tBTDevAddr);
        
        iResultArray->Reset();
        // ignore any errors here, if we don't get the name, we don't get the name
        static_cast<void>(iDevMan->GetDevices(nameSearch, iResultArray));
        }
    
    return KErrNone;
    }

// ---------------------------------------------------------
// ObexDisconnectIndication(
// ---------------------------------------------------------
//
void COPPController::ObexDisconnectIndication(const TDesC8& /*aInfo*/)
    {
    TRACE_FUNC

    }

// ---------------------------------------------------------
// TransportDownIndication()
// ---------------------------------------------------------
//
void COPPController::TransportDownIndication()
    {
    TRACE_FUNC
    // Remove receiving buffer and files used during file receiving.
    //
    delete iObexObject;
    iObexObject = NULL;
    TRAP_IGNORE(TObexUtilsMessageHandler::RemoveTemporaryRFileL (iFullPathFilename)); 
    iFs.Close();
    }

// ---------------------------------------------------------
// PutRequestIndication()
// ---------------------------------------------------------
//
CObexBufObject* COPPController::PutRequestIndication()
    {
    TRACE_FUNC
    iLengthHeaderReceived = EFalse; // New put request so clear header based state
    iObexTransferState = ETransferPut;
    
    // Checking if backup is running now - if backup process is active, then we
    // need to cancel transfer - otherwise phone will freeze during receiving
    // data
    if ( TObexUtilsUiLayer::IsBackupRunning() )
        {
        TRACE_INFO ( _L ("Backup in progress! Canceling incoming transfer."));
        iObexTransferState = ETransferPutInitError;
        return NULL;
        }
    
    TRAPD(err, HandlePutRequestL());
    if(err == KErrNone)
        {
        return iObexObject;
        }
    TRACE_INFO( _L( "[oppreceiveservice] COPPController: PutRequestIndication end\t" ) );
    if (iObexTransferState != ETransferPutInitError)
        {
        iObexTransferState = ETransferPutDiskError;
        }
    return NULL;
    }

// ---------------------------------------------------------
// PutPacketIndication()    
// ---------------------------------------------------------
//
TInt COPPController::PutPacketIndication()
    {
    TRACE_FUNC
    if(iObexTransferState == ETransferPutCancel)
        {
        // User cancelled the put request, so error the next packet to terminate the put request.
        // BIP considers the Unauthorized error response suitable for this...
        HandleError(ETrue); // reset state and clear up
        return KErrIrObexRespUnauthorized;
        }
    
    iTotalSizeByte = iObexObject->Length();     // get size of receiving file
    iReceivingFileName = iObexObject->Name();   // get name of receiving file
    
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
    if(iObexObject->Name().Length() > KMaxFileName)
        {
        return KErrAccessDenied;
        }
    if(iObexTransferState == ETransferPutDiskError)
        {
        return KErrDiskFull;
        }
    // successfully received put packet if we reached here
    iObexTransferState = ETransferPut;
    
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
    
    return KErrNone;
    }

// ---------------------------------------------------------
// PutCompleteIndication()
// ---------------------------------------------------------
//
TInt COPPController::PutCompleteIndication()
    {
    TRACE_FUNC
    TInt retVal = KErrNone;
    if(iObexTransferState == ETransferPutCancel)
        {
        retVal = KErrIrObexRespUnauthorized;
        HandleError(ETrue);
        }
    else
        {
        retVal = HandlePutCompleteIndication();
        iObexTransferState = ETransferIdle;
        CloseReceivingIndicator();
        }
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------
// GetRequestIndication()
// ---------------------------------------------------------
//
CObexBufObject* COPPController::GetRequestIndication( CObexBaseObject* /*aRequiredObject*/)
    {
    TRACE_FUNC
    return NULL;
    }

// ---------------------------------------------------------
// GetPacketIndication()
// ---------------------------------------------------------
//
TInt COPPController::GetPacketIndication()
    {
    TRACE_FUNC
    return KErrNone;
    }

// ---------------------------------------------------------
// GetCompleteIndication()
// ---------------------------------------------------------
//
TInt COPPController::GetCompleteIndication()
    {
    TRACE_FUNC
    return KErrNone;
    }

// ---------------------------------------------------------
// SetPathIndication()
// ---------------------------------------------------------
//
TInt COPPController::SetPathIndication( const CObex::TSetPathInfo& /*aPathInfo*/, 
                                        const TDesC8& /*aInfo*/)
    {
    TRACE_FUNC
    // SetPath is not implemented in OPP - so following IrOBEX guidance, return
    // the Forbidden response code.
    return KErrIrObexRespForbidden;
    }



// ---------------------------------------------------------
// HandleNotifyL()
// ---------------------------------------------------------
//
void COPPController::HandleNotifyL( TMemoryPropertyCheckType aCheckType )
    {    
    TRACE_FUNC        
    
    // Only interested on this notification if we are receiving something
    if ( iObexTransferState == ETransferPut )
        {
        // Check the keys, what has been changed.
        TRACE_INFO( _L( "[oppreceiveservice] COPPController::HandleNotifyL\t" ) );
        if ( aCheckType == ECheckPhoneMemory )
            {            
            if ( SysUtil::FFSSpaceBelowCriticalLevelL( NULL, 0 ) )
                {
                TRACE_INFO( _L( "[oppreceiveservice] COPPController: Obex Server error diskfull\t" ) );
                iObexTransferState = ETransferPutDiskError;                    
                }
            }
        else if ( aCheckType == ECheckMMCMemory )
            {                                
            if ( SysUtil::MMCSpaceBelowCriticalLevelL( NULL, 0 ) )
                {                        
                TRACE_INFO( _L( "[oppreceiveservice] COPPController: Obex Server error diskfull\t" ) );
                iObexTransferState = ETransferPutDiskError;
                }
            }            
        }        
    }
// ---------------------------------------------------------
// HandlePutRequestL()
// ---------------------------------------------------------
//
void COPPController::HandlePutRequestL()
    {
    TRACE_FUNC
    
    delete iObexObject;
    iObexObject = NULL;
    
    iFile = RFile();
    
    if ( !iFs.Handle() )
        {
        User::Leave(KErrGeneral);
        }

    // Assign an initial value to iDrive
    iDrive = GetDriveWithMaximumFreeSpaceL();    
     
    // If iDrive is at critical space level, we immediately show out_of_memory.
    //
    if (SysUtil::DiskSpaceBelowCriticalLevelL( &iFs, 0, iDrive ))
        {
        TRAP_IGNORE(TObexUtilsUiLayer::ShowGlobalConfirmationQueryL(R_OUT_OF_MEMORY));
        User::Leave(KErrGeneral);
        }
        
    TRACE_INFO( (_L( "[oppreceiveservice] HandlePutRequestL %d\t" ),iDrive ) ); 
    
    iObexObject = CObexBufObject::NewL( NULL );    
    
    delete iBuf;
    iBuf = NULL;
    
    TRACE_ASSERT(iMediaType != ESrcsMediaIrDA, KErrNotSupported);
    if ( iMediaType == ESrcsMediaBT )
        {
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
        TRAPD(err, TObexUtilsMessageHandler::CreateReceiveBufferAndRFileL(iFile,
                                                                              iDefaultFolder,
                                                                              iFullPathFilename,
                                                                              iBuf,
                                                                              KBufferSize));
        if(err != KErrNone)
            {
            iObexTransferState = ETransferPutInitError;
            User::Leave(KErrGeneral);
            }
        }
    else
        {
        iObexTransferState = ETransferPutInitError;
        User::Leave(KErrGeneral);
        }
    
    User::LeaveIfError(iFile.Open(iFs,iFullPathFilename,EFileWrite));   
    TObexRFileBackedBuffer bufferdetails(*iBuf,iFile,CObexBufObject::EDoubleBuffering);    
    
    TRAPD(err, iObexObject->SetDataBufL( bufferdetails) );
    if (err != KErrNone)
        {
        iObexTransferState = ETransferPutInitError;
        User::Leave(KErrGeneral);  // set to != KErrNone
        }
    
    TRACE_INFO( _L( "[oppreceiveservice] COPPController: HandlePutRequestL completed\t" ) );
    }


// ---------------------------------------------------------
// HandlePutCompleteIndication()
// ---------------------------------------------------------
//
TInt COPPController::HandlePutCompleteIndication()
	{
	TRACE_FUNC        
    TInt retVal = KErrNone;

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
	iFullPathFilename.Zero();
	iFullPathFilename.Append(iDefaultFolder);
       
        
	TRACE_INFO( (_L( "[oppreceiveservice] HandlePutCompleteIndication %d\t" ),iDrive ) ); 
	
	if (iMediaType==ESrcsMediaBT)
	    {
	    TRAP ( retVal, TObexUtilsMessageHandler::SaveFileToFileSystemL(iObexObject,
	                                                                   KUidMsgTypeBt,
	                                                                   iMsvIdParent,
	                                                                   iFullPathFilename,
	                                                                   iFile,
	                                                                   iRemoteDeviceName));
	    }
	TRACE_ASSERT( iMediaType!=ESrcsMediaIrDA, KErrNotSupported);
	if ( retVal == KErrNone)
	    {
	    TRAP (retVal, TObexUtilsMessageHandler::AddEntryToInboxL(iMsvIdParent, iFullPathFilename));		    
                
    if( retVal != KErrNone )
        {
        TRACE_INFO( (_L( "[oppreceiveservice] HandlePutCompleteIndication AddEntryToInboxL() failed  %d \t" ),retVal ) );                 	
        TRAP_IGNORE(TObexUtilsMessageHandler::RemoveInboxEntriesL(iObexObject, iMsvIdParent));        
        }
        }
	else
	    {
	    TRACE_INFO( (_L( "[oppreceiveservice] HandlePutCompleteIndication failed  %d \t" ),retVal ) ); 
	    }
	

    delete iObexObject;
    iObexObject = NULL;

    delete iBuf;
    iBuf = NULL;
    
    iPreviousDefaultFolder = iDefaultFolder;  // save the last file path where file is successfully saved to file system.
    iMsvIdParent = KMsvNullIndexEntryId; 
    TRACE_INFO( _L( "[oppreceiveservice] HandlePutCompleteIndication Done\t" ) );    
    return retVal;
	}

  
// ---------------------------------------------------------
// CheckCapacity()
// ---------------------------------------------------------
//	    
TBool COPPController::CheckCapacityL()
    {
    TRACE_FUNC_ENTRY   
    
    iDrive = EDriveZ; // Intialize iDrive to Z
    TInt filesize = iObexObject->Length();
    
    RFs rfs ;
    User::LeaveIfError(rfs.Connect());
         
    TInt mmcDrive = KDefaultDrive;   // External memroy card  
    TInt imsDrive = KDefaultDrive;   // Internal mass storage   

    User::LeaveIfError(DriveInfo::GetDefaultDrive(DriveInfo::EDefaultMassStorage, imsDrive));
    User::LeaveIfError(DriveInfo::GetDefaultDrive(DriveInfo::EDefaultRemovableMassStorage, mmcDrive));      
    
    TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL imsDrive=%d; mmcDrive=%d\t" ),imsDrive, mmcDrive ) );
    
    TVolumeInfo volumeInfo;
    TInt err = rfs.Volume(volumeInfo, imsDrive);
    
    // If err != KErrNone, Drive is not available.
    //
    if ( !err )
        {
        // Check capacity on Internal mass storage            
        TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL Internal mass storage\t" )) );
        if ( !SysUtil::DiskSpaceBelowCriticalLevelL( &rfs, filesize, imsDrive ) )
            {
            iDrive = imsDrive;            
            }
        }
    
    if ( iDrive == EDriveZ)
        {
        err = rfs.Volume(volumeInfo, mmcDrive);
        if ( !err )
            {
            // Check capacity on Internal mass storage    
            TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL Checking memory card\t" )) );
            if ( !SysUtil::DiskSpaceBelowCriticalLevelL( &rfs, filesize, mmcDrive ) )
                {                    
                iDrive = mmcDrive;
                }   
            }
        }           
    if ( iDrive == EDriveZ )
        {
        TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL Checking phone memory\t" )) );
        // Phone memory
        if( !SysUtil::DiskSpaceBelowCriticalLevelL( &rfs, filesize, EDriveC ))
            {
            iDrive = EDriveC;
            }
        }
    rfs.Close();
    TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL iDrive = %d\t" ),iDrive ) );
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
TBool COPPController::IsOBEXActive()
    {
    TRACE_FUNC
    return ETrue;
    }

// ---------------------------------------------------------
// SetMediaType()
// ---------------------------------------------------------
//
void COPPController::SetMediaType( TSrcsMediaType aMediaType ) 
    {
    TRACE_FUNC    
    iMediaType=aMediaType;    
    }
    
// ---------------------------------------------------------
// SetObexServer()
// ---------------------------------------------------------
//
TInt COPPController::SetObexServer( CObexServer* aServer)
	{	
	TInt retVal=KErrNone;    
	
	if (aServer)
	    {  
	    iObexServer = aServer;
	    retVal=aServer->Start(this);    
	    }
	return retVal;
	}
   
// ---------------------------------------------------------
// CancelTransfer()
// ---------------------------------------------------------
//
void COPPController::CancelTransfer()
    {
    TRACE_FUNC
    CloseReceivingIndicator();
    if(iObexTransferState == ETransferPut)
        {
        iObexTransferState = ETransferPutCancel;
        }
    else // go to idle for all other states
        {
        iObexTransferState = ETransferIdle;
        }
    }

void COPPController::LaunchReceivingIndicatorL()
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
            if(iMediaType == ESrcsMediaBT)
                {
                iProgressDialog->ShowProgressDialogL(R_BT_RECEIVING_DATA);
                }
            TRACE_ASSERT(iMediaType != ESrcsMediaIrDA, KErrNotSupported);
            }
        }
    else
        {
        iWaitDialog = CGlobalDialog::NewL(this);
        if(iMediaType == ESrcsMediaBT)
            {
            iWaitDialog->ShowNoteDialogL(R_BT_RECEIVING_DATA, ETrue);
            }
        TRACE_ASSERT(iMediaType != ESrcsMediaIrDA, KErrNotSupported);
        }
    }

void COPPController::UpdateReceivingIndicator()
    {
    if(iProgressDialog)
        {
        iProgressDialog->UpdateProgressDialog(iObexObject->BytesReceived(), iTotalSizeByte);
        }
    // else we are using a wait note, so no "need" to update
    }

void COPPController::HandleGlobalProgressDialogL( TInt aSoftkey )
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

void COPPController::HandleGlobalNoteDialogL( TInt aSoftkey )
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

void COPPController::CloseReceivingIndicator(TBool aResetDisplayedState)
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
  TInt COPPController::GetDriveWithMaximumFreeSpaceL()
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
 // COPPController::HandleGetDevicesComplete
 // Callback from devman
 // ----------------------------------------------------------
 //
 void COPPController::HandleGetDevicesComplete(TInt aErr, CBTDeviceArray* /*aDeviceArray*/)
    {
    TRACE_INFO( _L( "[oppreceiveservice] HandleGetDevicesComplete: enter \t" ) );
    if ( aErr == KErrNone )
        {
        if ( iResultArray->Count())
            {             
            iRemoteDeviceName.Zero();
            if ( iResultArray->At(0)->FriendlyName().Length() > 0 )
                {
                TRACE_INFO( _L( "[oppreceiveservice] HandleGetDevicesComplete: got friendly name \t" ) );
                iRemoteDeviceName.Copy(iResultArray->At(0)->FriendlyName());
                }
            else
                {
                TRACE_INFO( _L( "[oppreceiveservice] HandleGetDevicesComplete: got devciename name \t" ));
                TRAP_IGNORE(iRemoteDeviceName.Copy( BTDeviceNameConverter::ToUnicodeL(iResultArray->At(0)->DeviceName())));
                }
            }
        }
    }
 
//////////////////////////// Global part ////////////////////////////

//  End of File  
