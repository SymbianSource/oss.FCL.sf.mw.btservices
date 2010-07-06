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
#include <hb/hbcore/hbsymbianvariant.h>
#include "btnotifdeviceselector.h"

#include "btnotifserver.h"
#include "btnotificationmanager.h"
#include "btnotifclientserver.h"
// Key description length
const TInt KMaxKeyDesCLength  = 20;

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
                if(iServer.DevRepository().AllDevices().Count()==0)
                     {
                     PrepareNotificationL(TBluetoothDialogParams::EDeviceSearch, ENoResource);
                     iDevices.ResetAndDestroy();
                     iDiscoverer->DiscoverDeviceL();
                     }
                else
                     {
                     iDevices.ResetAndDestroy();
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
    if(aData.Keys().MdcaPoint(0).Compare(_L("selectedindex"))==KErrNone)
        {
        TInt val = *(static_cast<TInt*>(aData.Get(_L("selectedindex"))->Data()));
        BOstrace1( TRACE_DEBUG, TNAME_DEVLIST_2, "MBRDataReceived, val %d", val );

        if ( !iMessage.IsNull() )
            {
           // TInt sel = val;// - TBluetoothDialogParams::EDialogExt;
            TBTDeviceResponseParamsPckg devParams;    
            if (  val > -1 && val < iDevices.Count() )
                {
                devParams().SetDeviceAddress( iDevices[val]->Addr() );
                devParams().SetDeviceClass(iDevices[val]->Device().DeviceClass());
                devParams().SetDeviceName(iDevices[val]->Alias());
                err = iMessage.Write( EBTNotifSrvReplySlot, devParams );
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
    // Todo: look for this device in repository before creating it.
    CBtDevExtension* devext = CBtDevExtension::NewLC( aAddr, aName );
    iDevices.AppendL( devext );
    CleanupStack::Pop( devext );
    
    if(iNotification)
        {// conditional check required as CAdvanceDevDiscoverer sends discovered devices at times
         // even after canceldiscovery is issued and notification is set to NULL
         // this causes EExcDataAbort
    CHbSymbianVariantMap* map = iNotification->Data();
        TBuf<KMaxKeyDesCLength> keyStr;
    CHbSymbianVariant* devEntry;

    keyStr.Num( TBluetoothDialogParams::EDialogExt + iDevices.Count() - 1 );
    devEntry = CHbSymbianVariant::NewL( (TAny*) &(devext->Alias()), 
            CHbSymbianVariant::EDes );
    map->Add( keyStr, devEntry );
    iNotification->Update();
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
        if(iServer.DevRepository().AllDevices().Count()==0)
             {
             iDevices.ResetAndDestroy();
             TRAP(err, {
             PrepareNotificationL(TBluetoothDialogParams::EDeviceSearch, ENoResource);
             iDiscoverer->DiscoverDeviceL(); } );
             }
        else
             {
             iDevices.ResetAndDestroy();
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
      const TTime& usedTime = devArray[i]->Device().Used();
        TTime monthBack;
        monthBack.HomeTime();
        monthBack -= TTimeIntervalDays(30);
        if(usedTime >= monthBack)
            {
            iDevices.AppendL( devArray[i]->CopyL() );
            CHbSymbianVariantMap* map = iNotification->Data();
            TBuf<KMaxKeyDesCLength> keyStr;
            CHbSymbianVariant* devEntry;

            keyStr.Num( TBluetoothDialogParams::EDialogExt + iDevices.Count() - 1 );
            devEntry = CHbSymbianVariant::NewL( (TAny*) &(devArray[i]->Alias()), 
                    CHbSymbianVariant::EDes );
            map->Add( keyStr, devEntry );
            iNotification->Update();
            }
        }
    }
