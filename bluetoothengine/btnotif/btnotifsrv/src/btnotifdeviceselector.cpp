/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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


#include <btextnotifiers.h>
#include <btservices/advancedevdiscoverer.h>
#include <btservices/btdevextension.h>
#include <hb/hbcore/hbdevicedialogsymbian.h>
#include "btnotifdeviceselector.h"

#include "btnotifserver.h"
#include "btnotificationmanager.h"
#include "btnotifclientserver.h"

// Key description length
const TInt KMaxKeyDesCLength  = 20;
const TInt KLastUsedDevices = 5;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifDeviceSelector::CBTNotifDeviceSelector( CBTNotifServer& aServer )
:   iServer( aServer )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifDeviceSelector::ConstructL()
    {
    iServer.DevRepository().AddObserverL(this);
    iDiscoverer = CAdvanceDevDiscoverer::NewL( iServer.DevRepository(), *this );
    }

// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotifDeviceSelector* CBTNotifDeviceSelector::NewL( CBTNotifServer& aServer )
    {
    CBTNotifDeviceSelector* self = new( ELeave ) CBTNotifDeviceSelector( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifDeviceSelector::~CBTNotifDeviceSelector()
    {
    iServer.DevRepository().RemoveObserver(this);
    if( iNotification )
        {
        // Clear the notification callback, we cannot receive them anymore.
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        }
    iDevices.ResetAndDestroy();
    iDevices.Close();
    delete iDiscoverer;
    }

// ---------------------------------------------------------------------------
// Process a client message related to notifiers.
// ---------------------------------------------------------------------------
//
void CBTNotifDeviceSelector::DispatchNotifierMessageL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt ( DUMMY_LIST, this, aMessage.Function() );
    TInt opcode = aMessage.Function();
    TInt uid = aMessage.Int0();
    switch ( opcode ) 
        {
        case EBTNotifCancelNotifier:
            {
            // We only accept a cancel message from the same session as the original
            // request (this is enforced by the RNotifier backend).
            TInt err( KErrNotFound );
            if ( !iMessage.IsNull() && opcode == iMessage.Function() && 
                    aMessage.Session() == iMessage.Session() )
                {
                iMessage.Complete( KErrCancel );
                err = KErrNone;
                }
            aMessage.Complete( err );
            break;
            }
        case EBTNotifUpdateNotifier:
            {
            // not handling so far
            break;
            }
        case EBTNotifStartSyncNotifier:
            {
            // synch version of device searching is not supported:
            aMessage.Complete( KErrNotSupported );
            break;
            }
        case EBTNotifStartAsyncNotifier:
            {
            if ( !iMessage.IsNull() )
                {
                aMessage.Complete( KErrServerBusy );
                return;
                }
            
            iLoadDevices = EFalse;
            if(iServer.DevRepository().IsInitialized())
                {
                iLoadDevices = ETrue;
                iDevices.ResetAndDestroy();
                if(iServer.DevRepository().AllDevices().Count()==0)
                     {
                     PrepareNotificationL(TBluetoothDialogParams::EDeviceSearch, ENoResource);
                     iDiscoverer->DiscoverDeviceL();
                     }
                else
                     {
                     PrepareNotificationL(TBluetoothDialogParams::EMoreDevice, ENoResource);
                     LoadUsedDevicesL();
                     }
                }
            iMessage = aMessage;
            break;
            }
        default:
            {
            aMessage.Complete( KErrNotSupported );
            }
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Cancels an outstanding client message related to notifiers.
// ---------------------------------------------------------------------------
//
void CBTNotifDeviceSelector::CancelNotifierMessageL( const RMessage2& aMessage )
    {
    (void) aMessage;
    }

// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// Handle a result from a user query.
// ---------------------------------------------------------------------------
//
void CBTNotifDeviceSelector::MBRDataReceived( CHbSymbianVariantMap& aData )
    {
    TInt err = KErrCancel;
    if(aData.Keys().MdcaPoint(aData.Keys().MdcaCount()-1).Compare(_L("selectedindex"))==KErrNone)
        {
        TInt val = *(static_cast<TInt*>(aData.Get(_L("selectedindex"))->Data()));
        BOstrace1( TRACE_DEBUG, TNAME_DEVLIST_2, "MBRDataReceived, val %d", val );

        if ( !iMessage.IsNull() )
            {
            TBTDeviceResponseParamsPckg devParams;    
            if (  val > -1 && val < iDevices.Count() )
                {
                TRAP(err,SendSelectedDeviceL(aData));
                iNotification->RemoveObserver();
                iNotification->Close(); // Also dequeues the notification from the queue.
                iNotification = NULL;                
                }
            iMessage.Complete( err );
            }
        iDiscoverer->CancelDiscovery();
        }
    else if(aData.Keys().MdcaPoint(0).Compare(_L("Stop"))==KErrNone)
        {
         iDiscoverer->CancelDiscovery();
        }
    else if(aData.Keys().MdcaPoint(0).Compare(_L("Retry"))==KErrNone)
        {
        iDiscoverer->CancelDiscovery();
        iDevices.ResetAndDestroy();
        TRAP_IGNORE( iDiscoverer->DiscoverDeviceL() );
        
        }
    else if(aData.Keys().MdcaPoint(0).Compare(_L("MoreDevices"))==KErrNone)
        {
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        iDevices.ResetAndDestroy();
        TRAP_IGNORE( {
        PrepareNotificationL(TBluetoothDialogParams::EDeviceSearch, ENoResource);
        iDiscoverer->DiscoverDeviceL(); } );
        }
    }


// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// The notification is finished.
// ---------------------------------------------------------------------------
//
void CBTNotifDeviceSelector::MBRNotificationClosed( TInt aError, const TDesC8& aData  )
    {
    (void) aError;
    (void) aData;
    iNotification->RemoveObserver();
    iNotification = NULL;
    }

// ---------------------------------------------------------------------------
// HandleNextDiscoveryResultL
// ---------------------------------------------------------------------------
//
void CBTNotifDeviceSelector::HandleNextDiscoveryResultL( 
        const TInquirySockAddr& aAddr, const TDesC& aName )
    {
    CBtDevExtension* devext = GetDeviceFromRepositoryL(aAddr.BTAddr());
    
    if(!devext)
        {
        devext = CBtDevExtension::NewLC( aAddr, aName );
        }
    else
        {
        CleanupStack::PushL(devext);
        }
    iDevices.AppendL( devext );
    CleanupStack::Pop( devext );    
    
    if(iNotification)
        {// conditional check required as CAdvanceDevDiscoverer sends discovered devices at times
         // even after canceldiscovery is issued and notification is set to NULL
         // this causes EExcDataAbort
        LoadDeviceDetailsL(*devext);
        }
    }

// ---------------------------------------------------------------------------
// HandleDiscoveryCompleted
// ---------------------------------------------------------------------------
//
void CBTNotifDeviceSelector::HandleDiscoveryCompleted( TInt aErr )
    {
    (void) aErr;
    CHbSymbianVariantMap* map = iNotification->Data();
    TBuf<KMaxKeyDesCLength> keyStr;
    

    //TODO compile fail here we need to send the discovery completed text to the dialog 
    //TODO change the hardcoded string
    keyStr.Copy(_L("Search Completed"));
    CHbSymbianVariant* devEntry( NULL );
    TRAP_IGNORE( devEntry = CHbSymbianVariant::NewL( (TAny*) &(keyStr), 
            CHbSymbianVariant::EDes ) );
    if ( devEntry )
        {
        map->Add( keyStr, devEntry );
        iNotification->Update();
        }
    else
        {
        // todo: Complete client request with error
        }
    }

// From MBtDeviceRepositoryObserver

void CBTNotifDeviceSelector::RepositoryInitialized()
    {
    iRepositoryInitialized = ETrue;
    TInt err(KErrNone);
    if(!iLoadDevices)
        {
        iLoadDevices = ETrue;
        iDevices.ResetAndDestroy();
        if(iServer.DevRepository().AllDevices().Count()==0)
             {
             TRAP(err, {
             PrepareNotificationL(TBluetoothDialogParams::EDeviceSearch, ENoResource);
             iDiscoverer->DiscoverDeviceL(); } );
             }
        else
             {
             TRAP( err, 
                     {PrepareNotificationL(
                             TBluetoothDialogParams::EMoreDevice, ENoResource);
                      LoadUsedDevicesL();
                     } );
             }
        }
    if ( err )
        {
        // todo: complete client request
        }
    }

void CBTNotifDeviceSelector::DeletedFromRegistry( const TBTDevAddr& aAddr )
    {
    (void) aAddr;
    }

void CBTNotifDeviceSelector::AddedToRegistry( const CBtDevExtension& aDev )
    {
    (void) aDev;
    }

void CBTNotifDeviceSelector::ChangedInRegistry( const CBtDevExtension& aDev, TUint aSimilarity  ) 
    {
    (void) aDev;
    (void) aSimilarity;
    }

void CBTNotifDeviceSelector::ServiceConnectionChanged(const CBtDevExtension& aDev, TBool aConnected )
    {
    (void) aDev;
    (void) aConnected;
    }


// ---------------------------------------------------------------------------
// Get and configure a notification.
// ---------------------------------------------------------------------------
//
void CBTNotifDeviceSelector::PrepareNotificationL(
    TBluetoothDialogParams::TBTDialogType aType,
    TBTDialogResourceId aResourceId )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if(iNotification)
        {
        iNotification->RemoveObserver();
        iNotification = NULL;
        }
    iNotification = iServer.NotificationManager()->GetNotification();
    User::LeaveIfNull( iNotification ); // For OOM exception, leaves with KErrNoMemory
    iNotification->SetObserver( this );
    iNotification->SetNotificationType( aType, aResourceId );

    iServer.NotificationManager()->QueueNotificationL( iNotification,CBTNotificationManager::EPriorityHigh );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

void CBTNotifDeviceSelector::LoadUsedDevicesL()
    {
    const RDevExtensionArray& devArray= iServer.DevRepository().AllDevices();
    for(TInt i=0; i< devArray.Count(); i++ )
        {
        iDevices.AppendL( devArray[i]->CopyL() );
        }
        SortUsedDevicesL();
        RemoveUnWantedDevices();
        SendLastUsedDevicesL();
        SendPairedDevicesL();
        
    }


void CBTNotifDeviceSelector::SortUsedDevicesL() 
    { 
    TInt count  = iDevices.Count();
    for(TInt i=0;i<(count-1);i++) 
        {
        for(TInt j=0;j<(count-(i+1));j++)
            {
            if(iDevices[j]->Device().Used().Int64() < iDevices[j+1]->Device().Used().Int64())
                {
                CBtDevExtension* devextension = iDevices[j+1];
                iDevices.Remove(j+1);
                iDevices.InsertL(devextension,j);
                }
            }
        }
    } 


void CBTNotifDeviceSelector::RemoveUnWantedDevices()
    {
    for(TInt i=(iDevices.Count()-1);i>-1;i--)
        {
        TInt classOfDevice = iDevices[i]->Device().DeviceClass().DeviceClass();
        TBTDeviceClass codClass = TBTDeviceClass( classOfDevice );
        TUint8 majorDevCls = codClass.MajorDeviceClass();
        if((majorDevCls ==EMajorDeviceComputer)|| (majorDevCls ==EMajorDevicePhone))
            {
            }
        else
            {
            CBtDevExtension* devextension = iDevices[i];
            iDevices.Remove(i);
            delete devextension;
            devextension = NULL;
            iDevices.Compress();
            }
        }
    }

void CBTNotifDeviceSelector::SendLastUsedDevicesL()
    {
    TInt Count = iDevices.Count();
    if(Count > KLastUsedDevices)
        Count = KLastUsedDevices;
    for(TInt i=0;i < Count;i++)
        {
        LoadDeviceDetailsL(*(iDevices[i]));
        }
    }


void CBTNotifDeviceSelector::SendPairedDevicesL()
    {
    TInt count = iDevices.Count();
    if(count > KLastUsedDevices)
        {
        for(TInt i = KLastUsedDevices; i< count; i++)
            {
            if(isBonded( iDevices[i]->Device()))
                {
                LoadDeviceDetailsL(*(iDevices[i]));
                }
            }
        }
        
    }

void CBTNotifDeviceSelector::LoadDeviceDetailsL(const CBtDevExtension& aDevice)
    {
    TBuf<KBTDevAddrSize * 2> addr;
    TUint32 classOfDevice;
    TBool status;

    //TODO Need to create string constants Also try Enum value for device name rather 
    //than hadrdcoded string for key. It was not working previously
    CHbSymbianVariantMap* map = iNotification->CreateNotificationDataL();
    AddDataL(map,_L("Name"),(TAny*) &(aDevice.Alias()),CHbSymbianVariant::EDes);
    
    aDevice.Addr().GetReadable(addr);
    User::LeaveIfError(iNotification->SetData(TBluetoothDeviceDialog::EAddress,addr));
    
    classOfDevice =  aDevice.Device().DeviceClass().DeviceClass();
    User::LeaveIfError(iNotification->SetData(TBluetoothDeviceDialog::EDeviceClass,classOfDevice));
    
    status = isBonded( aDevice.Device());
    AddDataL(map,_L("Bonded"),&status,CHbSymbianVariant::EBool);
    
    status = aDevice.Device().GlobalSecurity().Banned();
    AddDataL(map,_L("Blocked"),&status,CHbSymbianVariant::EBool);
    
    status = aDevice.Device().GlobalSecurity().NoAuthorise();
    AddDataL(map,_L("Trusted"),&status,CHbSymbianVariant::EBool);
    
    status = aDevice.ServiceConnectionStatus() == EBTEngConnected;
    AddDataL(map,_L("Connected"),&status,CHbSymbianVariant::EBool);
    iNotification->Update();
    }

/*!
  Tells if the given device is bonded.
*/
TBool CBTNotifDeviceSelector::isBonded( const CBTDevice &dev ) const
{
    // todo: this has not addresses Just Works pairing mode yet.
    return dev.IsValidPaired() && dev.IsPaired() &&
        dev.LinkKeyType() != ELinkKeyUnauthenticatedUpgradable;
}

void CBTNotifDeviceSelector::AddDataL(CHbSymbianVariantMap* aMap, const TDesC& aKey, 
    const TAny* aData, CHbSymbianVariant::TType aDataType)
    {
    CHbSymbianVariant* value = CHbSymbianVariant::NewL(aData, aDataType);
    CleanupStack::PushL( value );
    User::LeaveIfError( aMap->Add( aKey, value ) ); // aMap takes the ownership of value
    CleanupStack::Pop( value );
    }

// ---------------------------------------------------------------------------
// Tells if these two instances are for the same remote device
// ---------------------------------------------------------------------------
//
TBool MatchDeviceAddress(const TBTDevAddr* aAddr, const CBtDevExtension& aDev)
    {
    return *aAddr == aDev.Device().BDAddr();
    }


CBtDevExtension* CBTNotifDeviceSelector::GetDeviceFromRepositoryL( const TBTDevAddr& aAddr ) 
{
    const RDevExtensionArray& devArray= iServer.DevRepository().AllDevices();
    
    TInt pos = devArray.Find( aAddr, MatchDeviceAddress);
    if(pos > -1)
        {
        return devArray[pos]->CopyL();
        }
    return NULL;
}


void CBTNotifDeviceSelector::SendSelectedDeviceL( CHbSymbianVariantMap& aData )
    {
    TBTDeviceResponseParamsPckg devParams;
    TBTDevAddr address; 
    User::LeaveIfError(address.SetReadable(
                                    *(static_cast<TDesC*>(aData.Get(_L("deviceaddress"))->Data()))));
    devParams().SetDeviceAddress( address );
    devParams().SetDeviceClass(*(static_cast<TUint32*>(aData.Get(_L("deviceclass"))->Data())));
    devParams().SetDeviceName(*(static_cast<TDesC*>(aData.Get(_L("devicename"))->Data())));
    User::LeaveIfError(iMessage.Write( EBTNotifSrvReplySlot, devParams ));
    }

