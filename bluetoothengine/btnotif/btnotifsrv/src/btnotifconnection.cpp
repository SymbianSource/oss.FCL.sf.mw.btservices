/*
* ============================================================================
*  Name        : btnotifconnection.cpp
*  Part of     : bluetoothengine / btnotif
*  Description : Class for observing events of a single connection, and for 
*                managing any user notifications related to the connection.
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.1
*/

#include "btnotifconnection.h"
#include <btextnotifiers.h>
#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifierspartner.h>
#endif

#include "btnotifconnectiontracker.h"
#include "btnotifpairinghelper.h"
#include "btnotificationmanager.h"
#include "btnotifclientserver.h"
#include "bluetoothtrace.h"

/**  Id for the baseband connection watcher active object. */
const TInt KConnectionWatcher = 40;
/**  Id for the registry watcher active object. */
const TInt KRegistryWatcher = 41;
/**  Id for the active object for updating the registry. */
const TInt KRegistryRetriever = 42;
/**  Event mask for subscribing to baseband connection events  
 * (need to check if these are appropriate). */
const TInt KBbEventMask = ENotifyAnyRole | ENotifyAuthenticationComplete |
    ENotifyPhysicalLinkUp | ENotifyPhysicalLinkDown | ENotifyPhysicalLinkError;


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Decide the device name to display from the device information, and 
// converts the name if necessary.
// ---------------------------------------------------------------------------
//
void GetDeviceNameL( TBTDeviceName& aName, const CBTDevice& aDevice )
    {
    if( aDevice.IsValidFriendlyName() )
        {
        aName.Copy( aDevice.FriendlyName() );
        }
    else
        {
        aName.Zero();
        if( aDevice.IsValidDeviceName() )
            {
            aName = BTDeviceNameConverter::ToUnicodeL( aDevice.DeviceName() );
            }
        }
    }


// ---------------------------------------------------------------------------
// Compare 2 device records device pairing has succeeded.
// aDev2 is the updated device record, aDev1 is the previous record.
// ---------------------------------------------------------------------------
//
TBool CheckRegistryPairedStatus( const CBTDevice* aOrig, const CBTDevice* aNew )
    {
    TBool result = EFalse;
    // Use the device class to check that this has any valid information.
    if( aOrig->AsNamelessDevice().IsValidDeviceClass() &&
        !( aOrig->IsValidPaired() && aOrig->IsPaired() ) ||
        aOrig->LinkKeyType() == ELinkKeyUnauthenticatedUpgradable )
        {
        // Only consider the result if the original device is not marked as paired.
        if( aNew->IsValidPaired() && aNew->IsPaired() && aNew->IsValidLinkKey() && 
            aNew->LinkKeyType() != ELinkKeyUnauthenticatedUpgradable )
            {
            // The new device record has valid pairing information, so
            // this device is now paired.
            result = ETrue;
            }
        }
    return result;
    }


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifConnection::CBTNotifConnection(  const TBTDevAddr& aAddr,
    CBTNotifConnectionTracker* aTracker )
:   iAddr( aAddr ),
    iTracker( aTracker )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::ConstructL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    iDevice = CBTDevice::NewL( iAddr );
    iPhyActive = CBtSimpleActive::NewL(*this, KConnectionWatcher );
    iRegActive = CBtSimpleActive::NewL( *this, KRegistryRetriever );
    // ToDo: need to check if this succeeds if a connection is 
    // being created, in case of outgoing pairing.
    User::LeaveIfError( iPhyLink.Open( iTracker->SocketServerSession(), iAddr ) );
    // Subscribe to events.
    iBasebandEvent.FillZ(); // To be sure that we are not reading false events.
    iPhyLink.NotifyNextBasebandChangeEvent( iBasebandEvent,
                iPhyActive->RequestStatus(), KBbEventMask );
    iPhyActive->GoActive();
    // Get the details from BT registry
    TBTRegistrySearch pattern;
    pattern.FindAddress( iAddr );
    User::LeaveIfError( iRegistry.Open( iTracker->RegistryServerSession() ) );
    iRegistry.CreateView( pattern, iRegActive->RequestStatus() );
    iRegActive->GoActive();
    iCurrentOp = EReadingRegistry;
    iRegDevArray  = new CBTDeviceArray(1);  // only 1 entry ever used
    // ToDo: more initialization needed?
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// NewLC.
// ---------------------------------------------------------------------------
//
CBTNotifConnection* CBTNotifConnection::NewLC( const TBTDevAddr& aAddr,
    CBTNotifConnectionTracker* aTracker )
    {
    CBTNotifConnection* self = new( ELeave ) CBTNotifConnection( aAddr, aTracker );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifConnection::~CBTNotifConnection()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if( iNotification )
        {
        // Clear the notification callback, we cannot receive them anymore.
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        }
    delete iRegActive;
    delete iRegistryResponse;
    iRegistry.Close();
    delete iDevMan;

    delete iPhyActive;
    iPhyLink.Close();
    delete iDevice;
    delete iPairingHelper;

    while( iMsgHandleQ.Count() )
        {
        CompleteClientRequest( KErrDisconnected, KNullDesC8 );
        }
    iMsgHandleQ.Close();
    iAcceptedConnections.Close();
    iDeniedConnections.Close();
    iRegDevArray->ResetAndDestroy(); 
    delete iRegDevArray;
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Check what to do next.
// This function should be called whenever we may be ready for the next
// request/action, which is from any callback function i.e. 
// MBAORequestCompletedL, MBRNotificationClosed, HandleNotifierRequestL and
// CancelNotifierRequestL.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::CheckNextOperationL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if( iCurrentOp == EIdle )
        {
        // Check the link state, to see if it has gone down already.
        TUint32 linkState = 0;
        TInt err = iPhyLink.PhysicalLinkState( linkState );
        TBool linkDown = linkState & ENotifyPhysicalLinkDown;
        if( ( !err && linkDown ) || err == KErrDisconnected )
            {
            // The link state tells us that the link is down,
            // inform the connection tracker the we are done.
            iTracker->HandleLinkCountChangeL();
            // Note that we may be deleted now!
            }
        else if( iMsgHandleQ.Count() )
            {
            // Get the next request and handle it.
            // ToDo: differentiate between notifier and pairing message!
            const RMessage2* message = iTracker->Server()->FindMessageFromHandle( iMsgHandleQ[0] );
            NOTIF_NOTHANDLED( message )
            TInt opcode = message->Function();
            if( opcode <= EBTNotifUpdateNotifier )
                {
                TBuf8<0x250> paramsBuf;    // Size needs to be long enough to read all possible parameter sizes.
                message->ReadL( EBTNotifSrvParamSlot, paramsBuf );
                HandleNotifierRequestL( paramsBuf, *message );
                }
            else
                {
                iMsgHandleQ.Remove( 0 );
                StartBondingL( *message );
                }
            }
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Complete the first outstanding client request and removes it from the queue.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::CompleteClientRequest( TInt aReason, const TDesC8& aReply )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    NOTIF_NOTHANDLED( iMsgHandleQ.Count() )
    TInt err = iTracker->Server()->CompleteMessage( iMsgHandleQ[0], aReason, aReply );
    NOTIF_NOTHANDLED( !err )
    iMsgHandleQ.Remove( 0 );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Distinguish a request and pass to corresponding handle.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::HandleNotifierRequestL( const TDesC8& aParams,
    const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aMessage.Int0() );
    if( !iMsgHandleQ.Count() || iMsgHandleQ[0] != aMessage.Handle() )
        {
        // If we are processing a queued request, we of course don't queue 
        // it again. In that case we are handling the first request from the queue.
        iMsgHandleQ.AppendL( aMessage.Handle() );
        }
    if( iCurrentOp == EIdle || iCurrentOp == EBonding )
        {
        // ToDo: check non-pairing operation when bonding
        TInt uid = aMessage.Int0();
        if( uid == KBTManAuthNotifierUid.iUid )
            {
            HandleAuthorizationReqL( aParams );
            }
        else if( uid == KBTManPinNotifierUid.iUid ||
                 uid == KBTPinCodeEntryNotifierUid.iUid ||
                 uid == KBTNumericComparisonNotifierUid.iUid ||
                 uid == KBTPasskeyDisplayNotifierUid.iUid )
            {
            if( !iPairingHelper )
                {
                BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST, 
                        "[BTNOTIF]\t CBTNotifConnection::HandleNotifierRequestL: creating CBTNotifPairingHelper");
                iPairingHelper = CBTNotifPairingHelper::NewL( this, iTracker );
                }
            if( iCurrentOp != EBonding  )
                {
                iCurrentOp = EPairing;
                }
            iPairingHelper->StartPairingNotifierL( uid, aParams );
            }
        // We may be done with the current request, proceed to the next one
        CheckNextOperationL();
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Update a notifier, update the outstanding dialog if the notifier request 
// is currently being served.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::HandleNotifierUpdateL( const TDesC8& aParams,
    const RMessage2& aMessage )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    NOTIF_NOTHANDLED( iCurrentOp != EIdle )
    (void) aParams;
    TBuf8<0x250> paramsBuf;    // Size needs to be long enough to read all possible sizes.
    aMessage.ReadL( EBTNotifSrvParamSlot, paramsBuf );
    TInt uid = aMessage.Int0();
    if( uid == KBTManAuthNotifierUid.iUid )
        {
        TBTNotifierUpdateParams params;
        TPckgC<TBTNotifierUpdateParams> paramsPckg( params );
        paramsPckg.Set( paramsBuf );
        // The result means result of conversion to unicode
        if( !paramsPckg().iResult )
            {
            // Only update locally, registry will update us with the same info later on.
            iDevice->SetDeviceNameL( BTDeviceNameConverter::ToUTF8L( paramsPckg().iName ) );
            if( iNotification )
                {
                // Update the dialog with the new name. It is up to the dialog to 
                // determine the validity (in case another dialog is shown).
                //iNotification->Update(  )
                }
            }
        }
    else if( iPairingHelper && ( uid == KBTPinCodeEntryNotifierUid.iUid ||
             uid == KBTNumericComparisonNotifierUid.iUid ||
             uid == KBTPasskeyDisplayNotifierUid.iUid ) )
        {
        // Just forward to pairing helper
        iPairingHelper->UpdatePairingNotifierL( uid, paramsBuf );
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Cancel a request, dismiss the outstanding dialog if the notifier request 
// is currently being served.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::CancelNotifierRequestL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    NOTIF_NOTHANDLED( iCurrentOp != EIdle )
    TInt pos = iMsgHandleQ.Find( aMessage.Handle() );
    if( pos > KErrNotFound )
        {
        // We have queued the message, remove it from the queue.
        iMsgHandleQ.Remove( pos );
        // We use the supplied handle to remove it, as it may not be
        // the first in the queue.
        TInt err = iTracker->Server()->CompleteMessage( aMessage.Handle(), KErrCancel, KNullDesC8 );
        NOTIF_NOTHANDLED( !err )
        if( pos == 0 )
            {
            // There could be the case that we are still post-processing
            // the previous request (e.g. blocking query), then the next
            // notification is not yet started but the first in the queue.
            // We can see that from the current operation type.
            if( iNotification && iCurrentOp < EAdditionalNotes )
                {
                // Cancel the user query
                // This will also unregister us from the notification.
                TInt err = iNotification->Close();
                NOTIF_NOTHANDLED( !err )
                iNotification = NULL;
                iCurrentOp = EIdle;
                }
            if( iPairingHelper )
                {
                // The pairing helper calls back PairingCompleted and sets state.
                iPairingHelper->CancelPairingNotifierL( aMessage.Int0() );
                // The pairing helper may have now been deleted.
                }
            }
        }
    // We may be done with the current request, proceed to the next one
    CheckNextOperationL();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Start a bonding operation with the remote device.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::StartBondingL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aMessage.Function() );
    if( iCurrentOp == EIdle || iCurrentOp > EInternalOperations )
        {
        __ASSERT_ALWAYS( !iPairingHelper, PanicServer( EBTNotifPanicBadState ) );
        iPairingHelper = CBTNotifPairingHelper::NewL( this, iTracker );
        // The pairingg helper stored the handle, not in our queue here.
        // This is because bonding will generate a pairing notifier request, which 
        // will be completed first. The bookkeeping gets complicated if we have to
        // re-order the queue here.
        iPairingHelper->StartBondingL( aMessage.Handle() );
        iCurrentOp = EBonding;
        }
    else if( iCurrentOp == EPairing || iCurrentOp == EBonding )
        {
        // We only do one pairing at the time.
        User::Leave( KErrInUse );
        }
    else
        {
        // We only store it here if it is not handled immediately.
        iMsgHandleQ.AppendL( aMessage.Handle() );
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Cancel an ongoing bonding operation with the remote device.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::CancelBondingL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if( iPairingHelper )
        {
        iPairingHelper->CancelBondingL();
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// The pairing handler has completed a pairing operation.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::PairingCompleted()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    __ASSERT_ALWAYS( iPairingHelper, PanicServer( EBTNotifPanicNullMember ) );
    if( iPairingHelper->CurrentOperation() == CBTNotifPairingHelper::EIdle )
        {
        // We are still idle. Remove the pairing helper
        delete iPairingHelper;
        iPairingHelper = NULL;
        }
    if( iCurrentOp == EPairing || iCurrentOp == EBonding )
        {
        iCurrentOp = EIdle;
        TRAP_IGNORE( CheckNextOperationL() );
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Process a new pairing result, and determine if we need to show 
// anything to the user.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::PairingResult( TInt aError )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if( !iPairingHelper )
        {
        TRAP_IGNORE( iPairingHelper = CBTNotifPairingHelper::NewL( this, iTracker ) );
        }
    if( iPairingHelper )
        {
        iPairingHelper->HandleAuthenticationCompleteL( aError );
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Process the new service-level connection, and determine if we need to
// show anything to the user.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::ServiceConnectedL( TBTProfile aProfile )
    {
    (void) aProfile;
    }


// ---------------------------------------------------------------------------
// Process the new service-level disconnection, and determine if we need to
// show anything to the user.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::ServiceDisconnectedL( TBTProfile aProfile )
    {
    (void) aProfile;
    }


// ---------------------------------------------------------------------------
// Ask the user if he/she wants to block future connection requests. 
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::LaunchBlockingQueryL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    iCurrentOp = EBlocking;
    TBTDialogResourceId resourceId = EBlockUnpairedDevice;
    if( iDevice->IsValidPaired() && iDevice->IsPaired() &&
        iDevice->LinkKeyType() != ELinkKeyUnauthenticatedUpgradable )
        {
        resourceId = EBlockPairedDevice;
        }
    PrepareNotificationL( TBluetoothDialogParams::EQuery, resourceId );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Modify the record for the remote device in BTRegistry, with the changes 
// already made in the local record.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::UpdateRegistryEntryL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // We use CBTEngDevMan here. We could use the registry API directly, however,
    // using this convenience API makes the registry processing much simpler.
    if( !iDevMan )
        {
        iDevMan = CBTEngDevMan::NewL( this );
        }
    iDevMan->ModifyDevice( *iDevice );
    if( iCurrentOp == EIdle ||
        ( ( iCurrentOp == EPairing || iCurrentOp == EBonding ) &&
          iPairingHelper->CurrentOperation() == CBTNotifPairingHelper::EIdle ) )
        {
        // To make sure that we don't get deleted while updating.
        iCurrentOp = EUpdatingRegistry;
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Modify the record for the remote device in BTRegistry, if aTrusted == true, then
// update trusted status after reading device info from registry
//
// ---------------------------------------------------------------------------

void CBTNotifConnection::UpdateRegistryEntryL( TBool aTrusted )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aTrusted );
    if (!aTrusted) {
        UpdateRegistryEntryL();
        return;
        }
    // We use CBTEngDevMan here. We could use the registry API directly, however,
    // using this convenience API makes the registry processing much simpler.
    if( !iDevMan )
        {
        iDevMan = CBTEngDevMan::NewL( this );
        }
    // first read device info from registry, to make sure we have up-to-date local info
    iCurrentOp = EReadingRegistry;
    GetDeviceFromRegistry( iDevice->BDAddr() );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// Handle a result from a user query.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::MBRDataReceived( CHbSymbianVariantMap & aData )
    {
    (void) aData;
    NOTIF_NOTIMPL
    }


// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// The notification is finished.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::MBRNotificationClosed( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // ToDo: call leaving function from here!
    __ASSERT_ALWAYS( iCurrentOp != EIdle, PanicServer( EBTNotifPanicBadState ) );
    // First unregister from the notification, so we can already get the next one.
    iNotification->RemoveObserver();
    iNotification = NULL;
    TRAP_IGNORE( NotificationClosedL( aError, aData ) );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Handle the active object completion.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::RequestCompletedL( CBtSimpleActive* aActive,
    TInt aStatus )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aActive->RequestId() );
    switch( aActive->RequestId() )
        {
        case KRegistryRetriever:
            {
            BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST,  
                    "[BTNOTIF]\t CBTNotifConnection::MBAORequestCompletedL: KRegistryRetriever" );
            if( !iRegistryResponse )
                {
                // We have just created the view, now get the results.
                // There can only be one result, as the address is unique.
                // (note: how about LE random addresses?)
                // Or then we have tried to re-open an existing view, so we get KErrInuse.
                // We anyway just get the results.
                __ASSERT_ALWAYS( aStatus < 2, PanicServer( EBTNotifPanicCorrupt ) );
                iRegistryResponse = CBTRegistryResponse::NewL( iRegistry );
                iRegistryResponse->Start( iRegActive->RequestStatus() );
                iRegActive->GoActive();
                }
            else
                {
                if( !aStatus )
                    {
                    // There can be only one result.
                    __ASSERT_ALWAYS( iRegistryResponse->Results().Count() == 1, PanicServer( EBTNotifPanicCorrupt ) );
                    CBTDevice* regDevice = iRegistryResponse->Results()[0];
                    TBool paired = CheckRegistryPairedStatus( iDevice, regDevice );
                    iDevice->UpdateL( *regDevice );
                    if( paired )
                        {
                        __ASSERT_ALWAYS( iPairingHelper, PanicServer( EBTNotifPanicNullMember ) );
                        iPairingHelper->HandleAuthenticationCompleteL( KErrNone );
                        }
                    }
                // ToDo: error handling of registry response result?
                delete iRegistryResponse;
                iRegistryResponse = NULL;
                }
            if( iCurrentOp == EReadingRegistry && !iRegActive->IsActive() )
                {
                // If this is part of the sequence of operations, we are done.
                // Otherwise we just update with the latest info from registry,
                // and then we don't interrupt or change the state.
                iCurrentOp = EIdle;
                }
            }
            // ToDo: start registry watching (preferably using registry API when this is available)
           break;
       case KRegistryWatcher:
           {
           BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST,  
                   "[BTNOTIF]\t CBTNotifConnection::MBAORequestCompletedL: KRegistryWatcher" );
           NOTIF_NOTHANDLED( !aStatus )
           // Ignore updates while we are already retrieving.
           if( !iRegActive->IsActive() )
               {
               // Refresh our information with the latest from registry
               iRegActive->SetRequestId( KRegistryRetriever );
               TBTRegistrySearch pattern;
               pattern.FindAddress( iAddr );
               iRegistry.CreateView( pattern, iRegActive->RequestStatus() );
               iRegActive->GoActive();
               }
           }
           break;
        case KConnectionWatcher:
            {
            BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST, 
                    "[BTNOTIF]\t CBTNotifConnection::MBAORequestCompletedL: KConnectionWatcher" );
            TUint32 event = iBasebandEvent().EventType();
            // First subscribe to the next event notification.
            // This will overwrite iBasebandEvent().EventType() with KBbEventMask
            iPhyLink.NotifyNextBasebandChangeEvent( iBasebandEvent,
                        iPhyActive->RequestStatus(), KBbEventMask );
            iPhyActive->GoActive();
            // Link down and link error are handled in CheckNextOperationL below.
            // ToDo: handle events!
            if( event & ( ENotifyPhysicalLinkDown | ENotifyPhysicalLinkError ) )
                {
                // We re-use the error code to store the indication that the 
                // link has disconnected. This will only be overridden by next 
                // event, which can only be a connection up event.
                iBasebandEvent().SetErrorCode( KErrDisconnected );
                }
            if( iPairingHelper )
                {
                if( event & ( ENotifyPhysicalLinkDown | ENotifyPhysicalLinkError |
                    ENotifyAuthenticationComplete ) )
                    {
                    // Results interesting for pairing result processing.
                    iPairingHelper->HandleAuthenticationCompleteL( iBasebandEvent().SymbianErrorCode() );
                    }
                else if( event & ENotifyPhysicalLinkUp &&
                        iPairingHelper->CurrentOperation() == CBTNotifPairingHelper::EDedicatedBonding )
                    {
                    iPairingHelper->StartBondingL( 0 );
                    }
                }
            }
            break;
        default:
            PanicServer( EBTNotifPanicBadState );
            break;
        }
    // We may be done with the current request, proceed to the next one
    CheckNextOperationL();
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Cancel and clean up all requests related to the active object.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::CancelRequest( TInt aRequestId )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if ( aRequestId == KConnectionWatcher )
        {
        iPhyLink.CancelNextBasebandChangeEventNotifier();
        }
    else if ( aRequestId == KRegistryWatcher && iRegistryResponse )
        {
        iRegistryResponse->Cancel();
        }
    else if ( aRequestId == KRegistryRetriever )
        {
        iRegistry.CancelRequest( iRegActive->RequestStatus());
        }    
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// 
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::HandleError( CBtSimpleActive* aActive, 
        TInt aError )
    {
    (void) aActive;
    (void) aError;
    }

// ---------------------------------------------------------------------------
// From class MBTEngDevmanObserver.
// Registry modification has completed.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::HandleDevManComplete( TInt aErr )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    (void) aErr;
    NOTIF_NOTHANDLED( !aErr )
    if( iCurrentOp == EUpdatingRegistry )
        {
        // To make sure that we don't get deleted while updating.
        iCurrentOp = EIdle;
        }
    // Refresh is done separately, we will get notified through 
    // the registry watcher of the change.
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// From class MBTEngDevmanObserver.
// Callback for getting a device from the registry
//
// Currently only used in context of setting device to trusted
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::HandleGetDevicesComplete( 
        TInt err, CBTDeviceArray* deviceArray ) 
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, err );
    // err is not used here very much, -1 could be returned if there is no device in registry,
    // but this case is covered by examing mRegDevArray.
    if (!err && (iCurrentOp == EReadingRegistry) ) {
        CBTDevice* dev (0);
        if ( deviceArray->Count() ) {
            dev = deviceArray->At( 0 );
            }
        if ( dev ) {
            iDevice = dev->CopyL();
            }
        // Set device to trusted
        // Copy the existing security settings.
        TBTDeviceSecurity sec( iDevice->GlobalSecurity().SecurityValue(),
                      iDevice->GlobalSecurity().PasskeyMinLength() );
        sec.SetNoAuthorise( ETrue );  // new value:  set device as trusted
        iDevice->SetGlobalSecurity( sec );
        iDevMan->ModifyDevice( *iDevice );   // write trusted (& paired) status to registry
        // To make sure that we don't get deleted while updating.
        iCurrentOp = EUpdatingRegistry;
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// Retrieves device from registry based on BT address parameter
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::GetDeviceFromRegistry( const TBTDevAddr &addr )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    TBTRegistrySearch searchPattern;
    searchPattern.FindAddress( addr );
    // and get this device from registry
    iRegDevArray->ResetAndDestroy();
    iDevMan->GetDevices(searchPattern, iRegDevArray);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

// ---------------------------------------------------------------------------
// Get and configure a notification.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::PrepareNotificationL( TBluetoothDialogParams::TBTDialogType aType,
    TBTDialogResourceId aResourceId )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    iNotification = iTracker->NotificationManager()->GetNotification();
    User::LeaveIfNull( iNotification ); // For OOM exception, leaves with KErrNoMemory
    iNotification->SetObserver( this );
    iNotification->SetNotificationType( aType, aResourceId );
    // Set the address of the remote device
    TBuf<KBTDevAddrSize> addr;
    addr.Copy( iAddr.Des() );
    TInt err = iNotification->SetData( TBluetoothDialogParams::EAddress, addr );
    NOTIF_NOTHANDLED( !err )
    // Set the name of the remote device
    TBTDeviceName name;
    GetDeviceNameL( name, *iDevice );
    // ToDo: handle leave in name conversion!
    err = iNotification->SetData( (TInt) TBluetoothDeviceDialog::EDeviceName, name );
    NOTIF_NOTHANDLED( !err )
    // Queue the notification for displaying to the user
    err = iTracker->NotificationManager()->QueueNotification( iNotification );
    NOTIF_NOTHANDLED( !err )
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// The notification is finished, handle the result.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::NotificationClosedL( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    switch( iCurrentOp )
        {
        case EAuthorizing:
            CompleteAuthorizationReqL( aError, aData );
            break;
        case EBlocking:
            CompleteBlockingReqL( aError, aData );
            break;
        default:
            NOTIF_NOTIMPL
            break;
        }
    // We may be done with the current request, proceed to the next one
    CheckNextOperationL();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Handle a request for authorization of this connection.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::HandleAuthorizationReqL( const TDesC8& aParams )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    __ASSERT_ALWAYS( iCurrentOp == EIdle, PanicServer( EBTNotifPanicBadState ) );
    __ASSERT_ALWAYS( !iNotification, PanicServer( EBTNotifPanicCorrupt ) );
    TBTAuthorisationParams params;
    TPckgC<TBTAuthorisationParams> paramsPckg( params );
    paramsPckg.Set( aParams );
    iCurrentOp = EAuthorizing;
    // The name in the parameter package is the latest one, retrieved from 
    // the remote device during this connection.
    if( paramsPckg().iName.Length() )
        {
        // Update the local device record. No need to update the registry,
        // that will be done by the stack, and we will receive the update 
        // information when that has completed.
        iDevice->SetDeviceNameL( BTDeviceNameConverter::ToUTF8L( paramsPckg().iName ) );
        }
    PrepareNotificationL( TBluetoothDialogParams::EQuery, EAuthorization );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Process the user input and complete the outstanding authorization request. 
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::CompleteAuthorizationReqL( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // Set our state to idle for now. This may get changed if the user just chose 
    // to block, or if we have a pending request.
    iCurrentOp = EIdle;
    if( !aError )
        {
        TPckgC<TBool> result( EFalse );
        result.Set( aData );
        TBool proceed = iTracker->UpdateBlockingHistoryL( iDevice, result() );
        if( result() == EFalse && proceed )
            {
            // The user denied the connection, ask to block the device.
            LaunchBlockingQueryL();
            }
        CompleteClientRequest( KErrNone, aData );
        }
    else
        {
        CompleteClientRequest( aError, KNullDesC8 );
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Process the user input for blocking a device.
// ---------------------------------------------------------------------------
//
void CBTNotifConnection::CompleteBlockingReqL( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    TPckgC<TBool> result( EFalse );
    result.Set( KNullDesC8 );    // to test!
    result.Set( aData );
    iCurrentOp = EIdle; // May get changed if we have a pending request.
    if( !aError && result() )
        {
        // The user accepted to block this device.
        TBTDeviceSecurity sec;  // use default values when setting as banned.
        sec.SetBanned( ETrue );
        iDevice->SetGlobalSecurity( sec );
        if( iDevice->IsValidPaired() && iDevice->IsPaired() )
            {
            // Deleting the link key will also set the device as unpaired.
            iDevice->DeleteLinkKey();
            }
        UpdateRegistryEntryL();
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }
