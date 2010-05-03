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
            PrepareNotificationL(TBluetoothDialogParams::EDeviceSearch, ENoResource);
            iDevices.ResetAndDestroy();
            iDiscoverer->DiscoverDeviceL();
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
//    const CHbSymbianVariant* value = aData.Get(_L("selectedindex"));
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
                err = iMessage.Write( EBTNotifSrvReplySlot, devParams );
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
        delete iDiscoverer;
        iDiscoverer = NULL;
        iDiscoverer = CAdvanceDevDiscoverer::NewL( iServer.DevRepository(), *this );
        iDiscoverer->DiscoverDeviceL();    
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
    CHbSymbianVariantMap* map = iNotification->Data();
    TBuf<8> keyStr;
    CHbSymbianVariant* devEntry;

    keyStr.Num( TBluetoothDialogParams::EDialogExt + iDevices.Count() - 1 );
    devEntry = CHbSymbianVariant::NewL( (TAny*) &(devext->Alias()), 
            CHbSymbianVariant::EDes );
    map->Add( keyStr, devEntry );
    iNotification->Update();
    }

// ---------------------------------------------------------------------------
// HandleDiscoveryCompleted
// ---------------------------------------------------------------------------
//
void CBTNotifDeviceSelector::HandleDiscoveryCompleted( TInt aErr )
    {
    (void) aErr;
    // todo: update dialog
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
    iNotification = iServer.NotificationManager()->GetNotification();
    User::LeaveIfNull( iNotification ); // For OOM exception, leaves with KErrNoMemory
    iNotification->SetObserver( this );
    iNotification->SetNotificationType( aType, aResourceId );

    /*
    _LIT(KTitleValue, "BT Search");
    TPtrC ptr;
    ptr.Set( KTitleValue );
    iNotification->SetData( TBluetoothDialogParams::EDialogTitle, ptr );
    */
    
    /*err = */ iServer.NotificationManager()->QueueNotification( iNotification );
    //NOTIF_NOTHANDLED( !err )
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }
