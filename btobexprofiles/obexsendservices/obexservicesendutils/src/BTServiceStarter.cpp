/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Service starter implementation
*
*/



// INCLUDE FILES
#include "BTServiceStarter.h"
#include "BTSUDebug.h"
#include "BTSOPPController.h"
#include "BTSBIPController.h"
#include "BTSBPPController.h"

#include <Obexutils.rsg>
#include <obexutilsuilayer.h>
#include <obexutilsmessagehandler.h>
#include <btnotif.h>
#include <featmgr.h>
#include "BTSProgresstimer.h"
#include <hbdevicenotificationdialogsymbian.h>
#include <btservices/bluetoothdevicedialogs.h>

// CONSTANTS

// From BT SIG - Assigned numbers
const TUint KBTServiceOPPSending        = 0x1105;
const TUint KBTServiceDirectPrinting    = 0x1118;
const TUint KBTServiceImagingResponder  = 0x111B;

const TUint KBTProgressInterval         = 1000000;


_LIT(KSendingDialog,"com.nokia.hb.btdevicedialog/1.0");

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTServiceStarter::CBTServiceStarter
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTServiceStarter::CBTServiceStarter() 
    : CActive( CActive::EPriorityStandard ),
      iBTEngDiscovery(NULL),
      iService( EBTSendingService ),
      iServiceStarted( EFalse ),
      iMessageServerIndex(0),
      iBytesSendWithBIP(0),
      iProgressDialogActive(EFalse),
      iUserCancel(EFalse), 
      iFeatureManagerInitialized(EFalse),
      iTriedBIP(EFalse),
      iTriedOPP(EFalse)
    {    
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::ConstructL()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ConstructL()"));
    iDevice = CBTDevice::NewL();
//    iDialog = CObexUtilsDialog::NewL( this );
    iDeviceDialog = CHbDeviceDialogSymbian::NewL();
    iProgressDialog = CHbDeviceProgressDialogSymbian::NewL(CHbDeviceProgressDialogSymbian::EWaitDialog,this);
    
    FeatureManager::InitializeLibL();
    iFeatureManagerInitialized = ETrue;
    FLOG(_L("[BTSU]\t CBTServiceStarter::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTServiceStarter* CBTServiceStarter::NewL()
    {
    CBTServiceStarter* self = new( ELeave ) CBTServiceStarter();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

CBTServiceStarter::~CBTServiceStarter()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::Destructor()"));
    if ( iMessageServerIndex != 0 )
        {
        TRAPD( notUsedRetVal, TObexUtilsMessageHandler::DeleteOutboxEntryL( iMessageServerIndex ) );
        notUsedRetVal=notUsedRetVal;
        FTRACE(FPrint(_L("[BTSU]\t ~CBTServiceStarter() delete ob entry %d"), notUsedRetVal ) );
        }
    StopTransfer(KErrCancel); // Cancels active object
    
    delete iList;
    delete iDevice;

    delete iController;
    delete iBTEngDiscovery;
 //   delete iDialog;
    delete iDeviceDialog;
    delete iProgressDialog;
    if(iProgressTimer)
        {
        delete iProgressTimer;
        }

    if(iWaiter && iWaiter->IsStarted() )
        {
        iWaiter->AsyncStop();
        }
    delete iBTEngSettings;
    
    if ( iFeatureManagerInitialized )
        {
        FeatureManager::UnInitializeLib();
        }
    
    FLOG(_L("[BTSU]\t CBTServiceStarter::Destructor() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::StartServiceL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::StartServiceL( TBTServiceType aService, 
                                       CBTServiceParameterList* aList,
                                       CActiveSchedulerWait* aWaiter )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::StartServiceL()"));
   
	// Delete old list and take ownership of the new one
    //
	
	delete iList;
	iList = aList;

    if ( iServiceStarted )
        {
        User::Leave( KErrInUse );
        }
    if ( !ValidParameters( aService, aList ) )
        {
        User::Leave( KErrArgument );
        }	
    
    // Store the waiter pointer, a NULL value is also accepted
    //
    iWaiter = aWaiter;

    // Store the requested service
    //
    iService = aService;

    if( !iBTEngSettings )
        {
        iBTEngSettings = CBTEngSettings::NewL( this );
        }
    TBTPowerStateValue power = EBTPowerOff;
    User::LeaveIfError( iBTEngSettings->GetPowerState( power ) );
    TBool offline = EFalse;
    if( !power )
        {
        //offline = CheckOfflineModeL();
         offline = EFalse;
        }
    if( !offline )
        {
        FLOG(_L("[BTSU]\t CBTServiceStarter::StartServiceL() Phone is online, request temporary power on."));
		TurnBTPowerOnL( power );
        }

    FLOG(_L("[BTSU]\t CBTServiceStarter::StartServiceL() completed"));
    }
    


// -----------------------------------------------------------------------------
// CBTServiceStarter::ControllerComplete
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::ControllerComplete( TInt aStatus )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ControllerComplete()"));    
    TInt error;
    if ( iAllSend ) //stop transfer if everything is send
        {
        StopTransfer( aStatus );	      	        
        return;    
        }
    if(( aStatus==KErrNone || aStatus==EBTSNoError ) && 
        iState == EBTSStarterFindingBIP )
        {
        iBytesSendWithBIP=0;
        if( iProgressGetter )
            {
            iBytesSendWithBIP=iProgressGetter->GetProgressStatus();
            iProgressGetter=NULL;
            delete iController; 
            iController = NULL;            
            }
        if ( !iBTEngDiscovery )
            {
            TRAPD(err, iBTEngDiscovery = CBTEngDiscovery::NewL(this) );
            if (err != KErrNone )
                {
                StopTransfer(EBTSPuttingFailed);	      	
                return;
                }
            }
        error=iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),
                               TUUID(KBTServiceOPPSending));	                 
        if( error == KErrNone )
	      	{
	      	iState = EBTSStarterFindingOPP;	
        	}
        else
        	{
        	StopTransfer(EBTSPuttingFailed);	      	
       		}		                           
        }
    else
        {
        StopTransfer( aStatus );	      	        
        }   
    FLOG(_L("[BTSU]\t CBTServiceStarter::ControllerComplete() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::GetProgressStatus
// -----------------------------------------------------------------------------
//
TInt CBTServiceStarter::GetProgressStatus()
    {    
    if ( iProgressGetter )
        {
        return iProgressGetter->GetProgressStatus()+iBytesSendWithBIP;       
        }
    else
        {
        return iBytesSendWithBIP;
        }
    }

void CBTServiceStarter::UpdateProgressInfoL()
    {
    HBufC* key = HBufC::NewL(50);
    CleanupStack::PushL(key);
   
    HBufC* value = HBufC::NewL(50);
    CleanupStack::PushL(value);
    
    CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();
    CleanupStack::PushL(map);
    
    TInt progress = GetProgressStatus();
    
    key->Des().Copy(_L("progressValue"));
    CHbSymbianVariant* progressvalue = CHbSymbianVariant::NewL(&progress, CHbSymbianVariant::EInt);
    map->Add(*key,progressvalue);
    
    key->Des().Copy(_L("currentFileIdx"));
    value->Des().AppendNum(iFileIndex);
    CHbSymbianVariant* currentFileIdx = CHbSymbianVariant::NewL(value, CHbSymbianVariant::EDes);
    map->Add(*key,currentFileIdx);

    
    TInt ret = iDeviceDialog->Update(*map);
    
    CleanupStack::PopAndDestroy(map);
    CleanupStack::PopAndDestroy(value);            
    CleanupStack::PopAndDestroy(key);
    
    
    if ( iProgressTimer )
        {
        iProgressTimer->Tickle();
        }
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::ValidParameters
// -----------------------------------------------------------------------------
//
TBool CBTServiceStarter::ValidParameters( 
    TBTServiceType aService, const CBTServiceParameterList* aList) const
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ValidParameters()"));

    TBool result = EFalse;

    if ( aList != NULL )
        {
        switch ( aService )
            {
            case EBTSendingService:
                {
                // Sending service must have object or image parameters
                //
                if ( aList->ObjectCount() > 0 || aList->ImageCount() > 0 )
                    {
                    result = ETrue;
                    }
                break;
                }
            case EBTPrintingService:
                {
                // Printing service must have xhtml parameters
                //
                if ( aList->XhtmlCount() > 0 )
                    {
                    result = ETrue;
                    }       
                break;
                }
            case EBTObjectPushService:
                {
                // Sending service must have object or image parameters
                //
                if ( aList->ObjectCount() > 0 || aList->ImageCount() > 0 )
                    {
                    result = ETrue;
                    }      
                break;
                }
            default:
                {
                result = EFalse;
                break;
                }
            }
        }
    FTRACE(FPrint(_L("[BTSU]\t CBTServiceStarter::ValidParameters() completed with %d"), result ) );

    return result;
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::StartProfileSelectL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::StartProfileSelectL( TBTServiceType aService )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL()"));    
    LaunchWaitNoteL();   
    iAllSend=EFalse;  
    iSendToBIPOnlyDevice = EFalse;
    
    if ( !iBTEngDiscovery )
        {
        iBTEngDiscovery = CBTEngDiscovery::NewL(this);
        }
    
    if ( !FeatureManager::FeatureSupported( KFeatureIdBtImagingProfile ) && (aService != EBTPrintingService) )
        {
        // If BTimagingProfile is disabled, use OPP instead.
        User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(), 
                                                                                       TUUID(KBTServiceOPPSending)));
        iState = EBTSStarterFindingOPP;  
        return;
        }
    
    switch ( aService  )
        {
        case EBTSendingService: // Find OPP
            {            
            if ( iList->ObjectCount() > 0 )  // We try to send files with OPP profile as long as it contains non-bip objects
                {
                FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() OPP"));    
                User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(), 
                                                                                TUUID(KBTServiceOPPSending)));
                iState = EBTSStarterFindingOPP;          
                }
            else if(iList->ObjectCount() == 0 && iList->ImageCount() > 0)
                {
                FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() BIP")); 
                User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),
                                                                                TUUID(KBTServiceImagingResponder)));
                iState = EBTSStarterFindingBIP;
                }
            break;
            }
        case EBTPrintingService: // Find BPP
            {
            FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() BPP"));
            User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),
                TUUID(KBTServiceDirectPrinting)));
            iState = EBTSStarterFindingBPP;            
            break;
            }
        case EBTObjectPushService: // Find BIP
            {
            FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() BIP"));
            User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),
                TUUID(KBTServiceOPPSending)));
            iState = EBTSStarterFindingOPP;            
            break;
            }
        default:
            {
            FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() ERROR, unhandled case"));            
            break;
            }
        }

    FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() completed"));
    }
    

// -----------------------------------------------------------------------------
// CBTServiceStarter::StartProfileL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::StartProfileL( TBTServiceProfile aProfile )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileL()"));

    switch ( aProfile )
        {
        case EBTSBPP:
            {            
            iController = CBTSBPPController::NewL( this, iClientChannel, 
                                                   iDevice->BDAddr(), iList,
                                                   iBTEngDiscovery );
            break;
            }
        case EBTSOPP:
            {
            iController = CBTSOPPController::NewL( this, iClientChannel, 
                                                   iDevice->BDAddr(), iList );
            break;
            }
        case EBTSBIP:
            {
            iController = CBTSBIPController::NewL( this, iClientChannel, 
                                                   iDevice->BDAddr(), iList );
            break;
            }
        case EBTSNone:
        default:
            {
            FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileL() ERROR, unhandled case"));            
            break;
            }
        }

    FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::LaunchWaitNoteL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::LaunchWaitNoteL()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::LaunchWaitNoteL()"));
    if ( iService == EBTPrintingService )
        {
 //       iDialog->LaunchWaitDialogL( R_BT_PRINTING_WAIT_NOTE );
        }
    else
        {
        //       iDialog->LaunchWaitDialogL( R_BT_CONNECTING_WAIT_NOTE );
        _LIT(KConnectText, "Connecting...");
        iProgressDialog->SetTextL(KConnectText);
        iProgressDialog->ShowL();
        
        }    
    FLOG(_L("[BTSU]\t CBTServiceStarter::LaunchWaitNoteL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::CancelWaitNote
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::CancelWaitNote()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::CancelWaitNote()"));

  //  if ( iDialog )
  //      {
        //       TRAP_IGNORE( iDialog->CancelWaitDialogL() );
        if(iProgressDialog)
            {
            //This has to be tested
            iProgressDialog->Close();
            }
    //    }

    FLOG(_L("[BTSU]\t CBTServiceStarter::CancelWaitNote() completed"));
    }


// -----------------------------------------------------------------------------
// CBTServiceStarter::LaunchProgressNoteL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::LaunchProgressNoteL( MBTServiceProgressGetter* aGetter,
                                             TInt aTotalSize, TInt aFileCount)
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::LaunchProgressNoteL()"));
    (void) aTotalSize;
    
    if ( iService != EBTPrintingService )
        {    
        iProgressGetter = aGetter;     
        CancelWaitNote();        
        
        if ( !iProgressDialogActive )
        	{
            iFileCount = aFileCount;
        	iMessageServerIndex = TObexUtilsMessageHandler::CreateOutboxEntryL( 
            KUidMsgTypeBt, R_BT_SEND_OUTBOX_SENDING );        
   //     	iDialog->LaunchProgressDialogL( this, aTotalSize, 
     //        								R_BT_SENDING_DATA, KBTProgressInterval );	
            CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();
            CleanupStack::PushL(map);
            if ( iProgressTimer )
                {
                iProgressTimer->Cancel();
                delete iProgressTimer;
                iProgressTimer = NULL;
                }

            iProgressTimer = CBTSProgressTimer::NewL( this );
            iProgressTimer->SetTimeout( KBTProgressInterval );
            
            iProgressTimer->Tickle();

            CHbSymbianVariant* value = NULL;
            TBuf<6> key;
            TInt data = TBluetoothDialogParams::ESend;
            key.Num(TBluetoothDialogParams::EDialogType);
            value = CHbSymbianVariant::NewL( (TAny*) &data, CHbSymbianVariant::EInt );
            User::LeaveIfError(map->Add( key, value ));   // Takes ownership of value
            iDeviceDialog->Show(KSendingDialog(),*map,this);
            CleanupStack::PopAndDestroy(map);
        	}        
        iProgressDialogActive=ETrue;     
        }

    FLOG(_L("[BTSU]\t CBTServiceStarter::LaunchProgressNoteL() completed"));
    }

void CBTServiceStarter::UpdateProgressNoteL(TInt aFileSize,TInt aFileIndex, const TDesC& aFileName )
    {
    HBufC* key = HBufC::NewL(50);
    CleanupStack::PushL(key);
   
    HBufC* value = HBufC::NewL(50);
    CleanupStack::PushL(value);
    
    CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();
    CleanupStack::PushL(map);
    
    iFileIndex = aFileIndex+1;
    key->Des().Copy(_L("currentFileIdx"));
    value->Des().AppendNum(aFileIndex+1);
    CHbSymbianVariant* currentFileIdx = CHbSymbianVariant::NewL(value, CHbSymbianVariant::EDes);
    map->Add(*key,currentFileIdx);
    
    key->Des().Copy(_L("totalFilesCnt"));
    value->Des().Zero();
    value->Des().AppendNum(iFileCount);
    CHbSymbianVariant* totalFilesCnt = CHbSymbianVariant::NewL(value, CHbSymbianVariant::EDes);
    map->Add(*key,totalFilesCnt);

    
    key->Des().Copy(_L("destinationName"));
    if ( iDevice->IsValidFriendlyName() )
        {
        value->Des().Copy( iDevice->FriendlyName() );
        }
    else 
        {
        value->Des().Copy( BTDeviceNameConverter::ToUnicodeL(iDevice->DeviceName()));
        }

    CHbSymbianVariant* destinationName = CHbSymbianVariant::NewL(value, CHbSymbianVariant::EDes);
    map->Add(*key,destinationName);
    
    key->Des().Copy(_L("fileName"));
    value->Des().Copy(aFileName);
    CHbSymbianVariant* fileName = CHbSymbianVariant::NewL(value, CHbSymbianVariant::EDes);
    map->Add(*key,fileName);
    
    key->Des().Copy(_L("fileSzTxt"));
    value->Des().Zero();
    if(aFileSize < 1024)
        {
        value->Des().AppendNum(aFileSize);
        value->Des().Append(_L(" Bytes"));
        }
    else
        {
        TInt filesize =  aFileSize/1024;
        value->Des().AppendNum(filesize);
        value->Des().Append(_L(" KB"));
        }

    CHbSymbianVariant* fileSzTxt = CHbSymbianVariant::NewL(value, CHbSymbianVariant::EDes);
    map->Add(*key,fileSzTxt);
    
    
    key->Des().Copy(_L("fileSz"));
    CHbSymbianVariant* fileSz = CHbSymbianVariant::NewL(&aFileSize, CHbSymbianVariant::EInt);
    map->Add(*key,fileSz);


    
    TInt ret = iDeviceDialog->Update(*map);
    CleanupStack::PopAndDestroy(map);
    CleanupStack::PopAndDestroy(value);            
    CleanupStack::PopAndDestroy(key);
    }
// -----------------------------------------------------------------------------
// CBTServiceStarter::CancelProgressNote
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::CancelProgressNote()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::CancelProgressNote()"));

 //   if ( iDialog )
        {
  //      TRAP_IGNORE( iDialog->CancelProgressDialogL() );
    if ( iProgressTimer )
         {
         iProgressTimer->Cancel();
         delete iProgressTimer;
         iProgressTimer = NULL;
         }
        }
    if(iDeviceDialog)
         {
         iDeviceDialog->Cancel();
         }
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::DialogDismissed
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::DialogDismissed( TInt aButtonId )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::DialogDismissed()"));   
    if( aButtonId == EAknSoftkeyCancel )
        {
        FLOG(_L("[BTSU]\t CBTServiceStarter::DialogDissmissed(), cancelled by user"));        
        iUserCancel=ETrue;
        if ( iController )
            {
            iController->Abort();
            }
        else 
           {
           StopTransfer(KErrCancel);
           }    
        }
    else if ( aButtonId == EAknSoftkeyNo )
        {
        // user abortion
        //
        iUserCancel = ETrue;
        StopTransfer( KErrCancel );
        }
    FLOG(_L("[BTSU]\t CBTServiceStarter::DialogDismissed() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::ShowNote
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::ShowNote( TInt aReason ) const
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ShowNote()"));
     
//    TInt resource = 0;
    TBuf<255> buf;

    switch ( aReason )
        {
        case EBTSNoError:
            {
    /*        if ( iService == EBTPrintingService )
                {
               // resource = R_BT_DATA_SENT2;
                
                _LIT(KText, "Data Sent");
                buf.Copy(KText);
                }
            else
                {
                //resource = R_BT_DATA_SENT;*/
                _LIT(KText, "All files Sent to ");
                buf.Copy(KText);
                if ( iDevice->IsValidFriendlyName() )
                    {
                    buf.Append( iDevice->FriendlyName() );
                    }
                else 
                    {
                   TRAP_IGNORE( buf.Append( BTDeviceNameConverter::ToUnicodeL(iDevice->DeviceName())));
                    }


//                }
            break;
            }
        case EBTSConnectingFailed:
            {
            //resource = R_BT_DEV_NOT_AVAIL;
            _LIT(KText, "Cannot establish Bluetooth connection");
            buf.Copy(KText);
            break;
            }
        case EBTSGettingFailed:
        case EBTSPuttingFailed:
            {
            if ( iService == EBTPrintingService )
                {
                _LIT(KText, "Sending failed");
                //resource = R_BT_FAILED_TO_SEND2;
                buf.Copy(KText);
                }
            else
                {
                _LIT(KText, "Failed to send Data");
                //resource = R_BT_FAILED_TO_SEND;
                buf.Copy(KText);
                }
            break;
            }
        case EBTSNoSuitableProfiles:
            {
            if ( iService == EBTPrintingService )
                {
                _LIT(KText, "Printer not supported");
                buf.Copy(KText);
            //    resource = R_BT_PRINTING_NOT_SUPPORTED;
                }
            else
                {
                _LIT(KText, "Failed to send Data");
                buf.Copy(KText);
         //       resource = R_BT_FAILED_TO_SEND;
                }
            break;
            }
        case EBTSBIPSomeSend:
        	{
        	_LIT(KText, "Failed to send Data");
        	 buf.Copy(KText);
        	//resource = R_BT_FAILED_TO_SEND;
        	break;	
        	}    
        case EBTSBIPOneNotSend:
        	{
        	_LIT(KText, "Receiving device does not support this image format.");
        	 buf.Copy(KText);
        	//resource = R_BT_NOT_RECEIVE_ONE;
        	break;
        	}
        case EBTSBIPNoneSend:
        	{
        	_LIT(KText, "Receiving device does not support the needed image formats.");
        	 buf.Copy(KText);
        	//resource = R_BT_NOT_RECEIVE_ANY;
        	break;
        	}	
        default:
            {            
           // resource = R_BT_DEV_NOT_AVAIL;
            _LIT(KText, "Cannot establish Bluetooth connection");
            buf.Copy(KText);
            break;
            }
        }        
    
//	TRAP_IGNORE(TObexUtilsUiLayer::ShowInformationNoteL( resource ) );	
//    CHbDeviceMessageBoxSymbian::InformationL(buf);
      TRAP_IGNORE(CHbDeviceNotificationDialogSymbian::NotificationL(KNullDesC, buf, KNullDesC));
    FLOG(_L("[BTSU]\t CBTServiceStarter::ShowNote() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::LaunchConfirmationQuery
// -----------------------------------------------------------------------------
//
TInt CBTServiceStarter::LaunchConfirmationQuery(TInt /*aResourceId*/)
	{
	TInt keypress=0;
//	TRAP_IGNORE( keypress = iDialog->LaunchQueryDialogL(  aResourceId ));	
	if ( !keypress )
		{
		FLOG(_L("[BTSU]\t CBTServiceStarter::LaunchConfirmationQuery(), cancelled by user"));
		DialogDismissed(EAknSoftkeyNo);
		CancelWaitNote();		
		}   		
	return keypress;
	}

// -----------------------------------------------------------------------------
// CBTServiceStarter::StopTransfer
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::StopTransfer(TInt aError)
	{
      
	FLOG(_L("[BTSU]\t CBTServiceStarter::StopTransfer()"));
    Cancel();
	if( !iUserCancel )
	    {
	    CancelWaitNote();
        CancelProgressNote();
	
        if ( aError != KErrCancel )
            {
            ShowNote( aError );
            }     
        }
    if ( iMessageServerIndex != 0 )
        {                 
        TRAPD( notUsedRetVal, TObexUtilsMessageHandler::DeleteOutboxEntryL( iMessageServerIndex ) );
        notUsedRetVal=notUsedRetVal;
        iMessageServerIndex=0;
        FTRACE(FPrint(_L("[BTSU]\t CBTServiceStarter::StopTransfer() delete ob entry %d"), notUsedRetVal ) );
        }
    // Release resources
    //
    if ( iList )
        {
        delete iList;
        iList = NULL;    
        }
        
    if ( iController )
        {
        delete iController; 
        iController = NULL;
        }
    
    if ( iNotifier.Handle() )
        {
        iNotifier.Close();
        }

    // Reset states
    //
    iServiceStarted = EFalse;
    if ( iWaiter && iWaiter->IsStarted() )
        {                
        iWaiter->AsyncStop();                    
        }    
        
    iState = EBTSStarterStoppingService;	    
	}
	
// -----------------------------------------------------------------------------
// CBTServiceStarter::ConnectTimedOut()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::ConnectTimedOut()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ConnectTimedOut()"));            
    StopTransfer(EBTSConnectingFailed);    
    FLOG(_L("[BTSU]\t CBTServiceStarter::ConnectTimedOut() completed"));
    }
// -----------------------------------------------------------------------------
// CBTServiceStarter::ServiceSearchComplete()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::ServiceSearchComplete( const RSdpRecHandleArray& /*aResult*/, 
                                         TUint /*aTotalRecordsCount*/, TInt /*aErr */)
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ServiceSearchComplete()"));        
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::AttributeSearchComplete()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::AttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, 
                                           const RSdpResultArray& /*aAttr*/, 
                                           TInt /*aErr*/ )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::AttributeSearchComplete()"));           
    }
// -----------------------------------------------------------------------------
// CBTServiceStarter::ServiceAttributeSearchComplete()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::ServiceAttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, 
                                                          const RSdpResultArray& aAttr, 
                                                          TInt aErr )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ServiceAttributeSearchComplete()"));               
    TInt err = KErrNone;
    if (aErr==KErrEof && aAttr.Count()>0 )
        {            
        RSdpResultArray results=aAttr;    
        iBTEngDiscovery->ParseRfcommChannel(results,iClientChannel);          

        iBTEngDiscovery->CancelRemoteSdpQuery();
               
        switch (iState)
            {
            case EBTSStarterFindingBIP:
                {
                TRAP(err, StartProfileL( EBTSBIP ));  
                iTriedBIP = ETrue;
                if (err != KErrNone)
                    {
                    StopTransfer(EBTSConnectingFailed);        
                    }
                if ( iSendToBIPOnlyDevice )
                    {
                    iAllSend = ETrue;
                    }
                else
                    {
                    if(iList->ObjectCount() == 0)
                        {
                        iAllSend=ETrue;
                        }
                    }
                break;
                }
            case EBTSStarterFindingOPP:
                {
                TRAP(err, StartProfileL( EBTSOPP ) ); 
                iTriedOPP = ETrue;
                if (err != KErrNone)
                    {
                    StopTransfer(EBTSConnectingFailed);        
                    }
                iAllSend=ETrue;  
                break;
                }            
            case EBTSStarterFindingBPP:          
                {
                TRAP(err, StartProfileL( EBTSBPP ));           
                if (err != KErrNone)
                    {
                    StopTransfer(EBTSConnectingFailed);        
                    }               
                break;    
                }            
            }       
        }      
    else if ( aErr==KErrEof && aAttr.Count()==0 && 
              iState == EBTSStarterFindingBIP && !iTriedBIP )
        {
        iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),TUUID(KBTServiceOPPSending));
        iState = EBTSStarterFindingOPP;    
		iTriedBIP = ETrue;
        }    
    else if ( aErr==KErrEof && aAttr.Count()==0 &&
              iState == EBTSStarterFindingOPP && !iTriedOPP &&
              iList->ImageCount() > 0 &&
              FeatureManager::FeatureSupported( KFeatureIdBtImagingProfile ) )
        {
        iSendToBIPOnlyDevice = ETrue;
        iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),TUUID(KBTServiceImagingResponder));
        iState = EBTSStarterFindingBIP;  
        iTriedOPP = ETrue;
        }
    else
        {
        delete iBTEngDiscovery;
        iBTEngDiscovery = NULL;    
        StopTransfer(EBTSConnectingFailed);    
        }    
    FLOG(_L("[BTSU]\t CBTServiceStarter::ServiceAttributeSearchComplete() done"));               
    }
// -----------------------------------------------------------------------------
// CBTServiceStarter::DeviceSearchComplete()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::DeviceSearchComplete( CBTDevice* /*aDevice*/, TInt aErr )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::DeviceSearchComplete()"));          
    FTRACE(FPrint(_L("[BTSU]\t CBTServiceStarter DeviceSearchComplete()aErr = %d"), aErr) );     
    if ( aErr == KErrNone )
        {    
        TRAPD(err, StartProfileSelectL( iService ));
        if (err != KErrNone )
            {
            StopTransfer(err);            
            }
            
        iServiceStarted = ETrue;
        }
    else
        {
        if ( aErr == KErrCancel )
            {
            iUserCancel=ETrue;
            }
        StopTransfer(aErr);    
        }    
    FLOG(_L("[BTSU]\t CBTServiceStarter::DeviceSearchComplete() done"));                   
    }        

// -----------------------------------------------------------------------------
// From class MBTEngSettingsObserver.
// Power has changed, start searching for BT devices.
// -----------------------------------------------------------------------------
//  
void CBTServiceStarter::PowerStateChanged( TBTPowerStateValue aState )
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::PowerStateChanged()"));	
    if( aState == EBTPowerOn && iWaitingForBTPower )
        {
        iWaitingForBTPower = EFalse;
        TRAP_IGNORE( StartDiscoveryL() );
        }
	FLOG(_L("[BTSU]\t CBTServiceStarter::PowerStateChanged() - completed"));
    }


// -----------------------------------------------------------------------------
// From class MBTEngSettingsObserver.
// Visibility has changed, ignore event.
// -----------------------------------------------------------------------------
//  
void CBTServiceStarter::VisibilityModeChanged( TBTVisibilityMode aState )
    {
    (void) aState;
    }

// -----------------------------------------------------------------------------
// Check if the phone is in offline mode, and ask the user if it is.
// -----------------------------------------------------------------------------
//  
TBool CBTServiceStarter::CheckOfflineModeL()
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::CheckOfflineModeL()"));	
    TCoreAppUIsNetworkConnectionAllowed offline = ECoreAppUIsNetworkConnectionNotAllowed;
    TBTEnabledInOfflineMode offlineAllowed = EBTDisabledInOfflineMode;
    User::LeaveIfError( iBTEngSettings->GetOfflineModeSettings( offline, offlineAllowed ) );
    if( offline == ECoreAppUIsNetworkConnectionNotAllowed && 
         offlineAllowed == EBTEnabledInOfflineMode )
        {
        User::LeaveIfError( iNotifier.Connect() );
        TBTGenericQueryNotiferParamsPckg pckg;
        pckg().iMessageType = EBTActivateOffLineQuery;
        pckg().iNameExists = EFalse;
        iActiveNotifier = EOfflineQuery;
        iNotifier.StartNotifierAndGetResponse( iStatus, KBTGenericQueryNotifierUid, 
                                                pckg, iOffline );
        SetActive();
        }
    else if( offline == ECoreAppUIsNetworkConnectionNotAllowed && 
            offlineAllowed == EBTDisabledInOfflineMode )
           {
           StopTransfer( KErrNotSupported );
           }
	FLOG(_L("[BTSU]\t CBTServiceStarter::CheckOfflineModeL() - completed"));
    return ( offline == ECoreAppUIsNetworkConnectionNotAllowed );
    }


// -----------------------------------------------------------------------------
// Start BT device discovery.
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::StartDiscoveryL()
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::StartDiscoveryL()"));	
    if( !iBTEngDiscovery )
        {
        iBTEngDiscovery = CBTEngDiscovery::NewL(this);
        }
    TInt err = iBTEngDiscovery->SearchRemoteDevice(iDevice );
    if( err )
        {
        StopTransfer( err );
        }
	FLOG(_L("[BTSU]\t CBTServiceStarter::StartDiscoveryL() - completed"));		
    }


// -----------------------------------------------------------------------------
// Turn BT on and start BT device discovery if possible.
// -----------------------------------------------------------------------------
//  
void CBTServiceStarter::TurnBTPowerOnL( const TBTPowerStateValue aState )
    {
	FLOG( _L("[BTSU]\t CBTServiceStarter::TurnBTPowerOnL()") );
//    if (iName() != EFalse) 
    	{
    	if( !iBTEngSettings )
	        {
	        iBTEngSettings = CBTEngSettings::NewL( this );
	        }
	    TInt err = iBTEngSettings->ChangePowerStateTemporarily();
	    iWaitingForBTPower = ETrue;
	    if( err )
	        {
	        iWaitingForBTPower = EFalse;
	        StopTransfer( err );
	        }
	    else if( aState == EBTPowerOn )
	        {
	        // Power is already on, we just registered for turning it off if needed.
	        // Since there is no callback at this point (power is already on), start 
	        // searching straight away.
	        iWaitingForBTPower = EFalse;
	        StartDiscoveryL();
	        }
    	}
 /*   else
    	{
        if ( !iNotifier.Handle() )
	        {
			User::LeaveIfError( iNotifier.Connect() );
	        }
		TBTGenericQueryNotiferParamsPckg pckg;
        pckg().iMessageType = EBTNameQuery;
        pckg().iNameExists = EFalse;
        iActiveNotifier = ENameQuery;
        iNotifier.StartNotifierAndGetResponse( iStatus, KBTGenericQueryNotifierUid, 
                                                  pckg, iName );
        SetActive();
    	}*/
	FLOG(_L("[BTSU]\t CBTServiceStarter::TurnBTPowerOnL() - completed"));
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been cancelled.
// ---------------------------------------------------------------------------
//
void CBTServiceStarter::DoCancel()
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::DoCancel()"));
    iNotifier.CancelNotifier( KBTGenericQueryNotifierUid );
    iNotifier.Close();
	FLOG(_L("[BTSU]\t CBTServiceStarter::DoCancel() - completed"));
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been completed.
// ---------------------------------------------------------------------------
//
void CBTServiceStarter::RunL()
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::RunL()"));
    TInt err = iStatus.Int();
    if( !err )
        {
        if ( (iActiveNotifier == ENameQuery && iName() != EFalse ) || ( iActiveNotifier == EOfflineQuery && iOffline() != EFalse ) )
            {
            TBTPowerStateValue power = EBTPowerOff;
            if ( iNotifier.Handle() )
                {
                iNotifier.Close();
                }
            User::LeaveIfError( iBTEngSettings->GetPowerState( power ) );
            TurnBTPowerOnL( power );
            }
        else
            {
            err = KErrCancel;
            }    
        }

    if( err )
        {
      
        err = ( err == KErrNotSupported ? KErrCancel : err );
        if ( iWaiter && err != KErrInUse && err != KErrCancel )
            {
            err = EBTSPuttingFailed;
            }
        StopTransfer( err );
        }
    
    FLOG(_L("[BTSU]\t CBTServiceStarter::RunL() - completed"));	
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when an error in RunL has occurred.
// ---------------------------------------------------------------------------
//
TInt CBTServiceStarter::RunError( TInt aError )
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::RunError()"));
    StopTransfer( aError );
	FLOG(_L("[BTSU]\t CBTServiceStarter::RunError() - completed"));
    return KErrNone;
    }




void CBTServiceStarter::ProgressDialogCancelled(const CHbDeviceProgressDialogSymbian*/*  aDialog*/)
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ProgressDialogCancelled(), cancelled by user"));        
    iUserCancel=ETrue;
    if ( iController )
        {
        iController->Abort();
        }
    else 
       {
       StopTransfer(KErrCancel);
       }    
    }


void CBTServiceStarter::ProgressDialogClosed(const CHbDeviceProgressDialogSymbian* /* aDialog*/)
    {
    }


void CBTServiceStarter::DataReceived(CHbSymbianVariantMap& /*aData*/)
    {
    
    }


void CBTServiceStarter::DeviceDialogClosed(TInt /* aCompletionCode*/)
    {
    TBuf<255> buf;
    _LIT(KText, "Sending Cancelled to ");
    buf.Copy(KText);
    if ( iDevice->IsValidFriendlyName() )
        {
        buf.Append( iDevice->FriendlyName() );
        }
    else 
        {
        TRAP_IGNORE(buf.Append( BTDeviceNameConverter::ToUnicodeL(iDevice->DeviceName())));
        }

    iUserCancel=ETrue;
    if ( iController )
        {
        iController->Abort();
        }
    else 
       {
       StopTransfer(KErrCancel);
       }    
    
    if ( iProgressTimer )
        {
        iProgressTimer->Cancel();
        delete iProgressTimer;
        iProgressTimer = NULL;
        }
    TRAP_IGNORE(CHbDeviceNotificationDialogSymbian::NotificationL(KNullDesC, buf, KNullDesC));
    }
