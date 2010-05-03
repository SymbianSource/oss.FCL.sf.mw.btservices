/*
* ============================================================================
*  Name        : btnotifconnectiontracker.cpp
*  Part of     : bluetoothengine / btnotif
*  Description : Bluetooth connection tracker and manager.
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

#include "btnotifconnectiontracker.h"
#include <btextnotifiers.h>
#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifierspartner.h>
#endif

#include "btnotifconnection.h"
#include "btnotifsession.h"
#include "btnotifclientserver.h"
#include "bluetoothtrace.h"

/**  Id for the link key watcher active object. */
const TInt KLinkCountWatcher = 30;
/**  Id for the pairing result watcher active object. */
const TInt KSspResultWatcher = 31;
/**  Id for the registry watcher active object (TEMP!). */
const TInt KRegistryWatcher = 41;
/**  Time window for determining if there are too many requests. */
#ifndef __WINS__
#define KDENYTHRESHOLD TTimeIntervalSeconds(3)
#else   //__WINS__
#define KDENYTHRESHOLD TTimeIntervalSeconds(5)
#endif  //__WINS__


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Checks if the notifier is one launched by the security manager of the
// protocol stack. These notifiers need to be served unless really not possible.
// ---------------------------------------------------------------------------
//
TBool IsStackSecmanNotifier( TInt aUid )
    {
    TBool result = EFalse;
    if( aUid == KBTManAuthNotifierUid.iUid || aUid == KBTManPinNotifierUid.iUid ||
        aUid == KBTPinCodeEntryNotifierUid.iUid || aUid == KBTNumericComparisonNotifierUid.iUid ||
        aUid == KBTPasskeyDisplayNotifierUid.iUid )
        {
        result = ETrue;
        }
    return result;
    }


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifConnectionTracker::CBTNotifConnectionTracker( CBTNotifServer* aServer )
:   iServer( aServer )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::ConstructL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // Start watching the number of baseband links.
    TInt err = iLinkCount.Attach( KPropertyUidBluetoothCategory,
                KPropertyKeyBluetoothGetPHYCount );
    // There is not much point to continue if we can't attach to
    // the link count key.
    User::LeaveIfError( err );
    iLinkCountActive = CBtSimpleActive::NewL( *this, KLinkCountWatcher );
    iLinkCount.Subscribe( iLinkCountActive->RequestStatus() );
    iLinkCountActive->GoActive();
    // Open a handle to the registry server
    User::LeaveIfError( iBTRegistrySession.Connect() );
    // Open a handle to the socket server
    User::LeaveIfError( iSockServ.Connect() );
    iPairingServ = new( ELeave ) RBluetoothPairingServer();
    if( iPairingServ->Connect() )
        {
        // Delete in case of error - there is no good other way to keep track.
        delete iPairingServ;
        iPairingServ = NULL;
        }
    else
        {
        iSspResultActive = CBtSimpleActive::NewL( *this, KSspResultWatcher );
        User::LeaveIfError( iSspResultSession.Open( *iPairingServ ) );
        iSspResultSession.SimplePairingResult( iSspResultAddr, iSspResultActive->RequestStatus() );
        iSspResultActive->GoActive();
        }
    iConnMan = CBTEngConnMan::NewL( this );
    iPhyLinks = CBluetoothPhysicalLinks::NewL( *this, iSockServ );
// ToDo: remove this when registry notifications API is available!!
    err = iRegistryChange.Attach( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothRegistryTableChange );
    User::LeaveIfError( err );
    iRegistryActive = CBtSimpleActive::NewL( *this, KRegistryWatcher );
    iRegistryChange.Subscribe( iRegistryActive->RequestStatus() );
    iRegistryActive->GoActive();
// End ToDo
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotifConnectionTracker* CBTNotifConnectionTracker::NewL( CBTNotifServer* aServer )
    {
    CBTNotifConnectionTracker* self = new( ELeave ) CBTNotifConnectionTracker( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifConnectionTracker::~CBTNotifConnectionTracker()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    iConnArray.ResetAndDestroy();
    iConnArray.Close();
    iDeniedRequests.Close();
    delete iLinkCountActive;
    iLinkCount.Close();
    
    delete iConnMan;
    delete iPhyLinks;
    iSockServ.Close();
    delete iSspResultActive;
    iSspResultSession.Close();
    if( iPairingServ )
        {
        iPairingServ->Close();
        delete iPairingServ;
        }
    delete iRegistryActive;
    iRegistryChange.Close();
    iBTRegistrySession.Close();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Process a client message related to notifiers.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::DispatchNotifierMessageL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt ( DUMMY_LIST, this, aMessage.Function() );
    TInt opcode = aMessage.Function();
    TInt uid = aMessage.Int0();
    const RMessage2* message = &aMessage;
    // Use a pointer to the original message, so that we don't duplicate it.
    // Then we avoid any bookkeeping for keeping them in sync.
    if( opcode == EBTNotifCancelNotifier )
        {
        // We only accept a cancel message from the same session as the original
        // request (this is enforced by the RNotifier backend). So we use the
        // session of the cancel request (if this would change, the same way as
        // for updates can be followed).
        // We need to find the original request to identify the handler of the 
        // connection; the uid points to the original request.
        message = ( (CBTNotifSession *) aMessage.Session() )->FindMessageFromUid( uid );
        }
    else if( opcode == EBTNotifUpdateNotifier )
        {
        // We accept a update messages from any client, although in practice,
        // they will all come from the same session (through RNotifier).
        // We need to find the original request to identify the handler of the 
        // connection (the uid points to the original request). Through the 
        // server, we get it from any session.
        message = iServer->FindMessageFromUid( uid );
        }
    if( !message )
        {
        // It's hard to continue if we don't know where to route the message.
        User::Leave( KErrDisconnected );
        }
    TBuf8<0x250> paramsBuf;    // Size needs to be long enough to read all possible parameter sizes.
    CBTNotifConnection* connection = FindConnectionFromMessageL( opcode, *message, paramsBuf );
    if( !connection )
        {
        User::Leave( KErrDisconnected );
        }
    switch( opcode )
        {
        case EBTNotifStartSyncNotifier:
        case EBTNotifStartAsyncNotifier:
            connection->HandleNotifierRequestL( paramsBuf, aMessage );
            break;
        case EBTNotifUpdateNotifier:
            connection->HandleNotifierUpdateL( paramsBuf, aMessage );
            break;
        case EBTNotifCancelNotifier:
            // Complete the cancel message already here, so that the caller can
            // continue, and the next operation can close sessions with the caller.
            aMessage.Complete( KErrNone );
            connection->CancelNotifierRequestL( *message );
            break;
        default:
            break;
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Handle a request related to pairing.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::HandleBondingRequestL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt ( DUMMY_LIST, this, aMessage.Function() );
    // Bonding is an infrequently occurring operation, so we don't waste memory
    // to keep a copy of the parameters. Instead we read them again when needed.
    TPckgBuf<TBTDevAddr> addrBuf;
    TInt opcode = aMessage.Function();
    if( opcode == EBTEngPairDevice )
        {
        aMessage.ReadL( EBTNotifSrvParamSlot, addrBuf );
        }
    else if( opcode == EBTEngCancelPairDevice )
        {
        const RMessage2* message =
                ( (CBTNotifSession *) aMessage.Session() )->FindMessageFromUid( EBTEngPairDevice );
        message->ReadL( EBTNotifSrvParamSlot, addrBuf );
        }
    BtTraceBtAddr1( TRACE_DEBUG, DUMMY_LIST, "CBTNotifConnectionTracker::HandleBondingRequestL() addr=", addrBuf() );
	TInt err = KErrNotFound;
	CBTNotifConnection* connection = FindConnectionHandler( addrBuf() );
    if( opcode == EBTEngPairDevice )
        {
        if( !connection )
            {
            // Create a connection first, then tell it to bond.
            err = iPhyLinks->CreateConnection( addrBuf() );
            connection = CBTNotifConnection::NewLC( addrBuf(), this );
            iConnArray.AppendL( connection );
            CleanupStack::Pop( connection );
            }
        else
            {
            // There is an existing connection. Care must be taken, the connection
            // _should_ be disconnect first if this device is already paired, so that
            // we are sure that we don't mix up the state of the connection.
            RBTPhysicalLinkAdapter link;
            err = link.Open( iSockServ, addrBuf() );
            TUint32 linkState = 0;
            if( !err )
                {
                err = link.PhysicalLinkState( linkState );
                }
            if( !err && linkState & ( ENotifyAuthenticationComplete | ENotifyEncryptionChangeOn ) )
                {
                // For now, we just reject the request.
                err = KErrAlreadyExists;
                }
            link.Close();
            }
        if( !err )
            {
            // Start bonding immediately so that the connection object is in the right state.
            connection->StartBondingL( aMessage );
            }
        }
    else if( opcode == EBTEngCancelPairDevice && connection )
        {
        connection->CancelBondingL();
        err = KErrNone;
        aMessage.Complete( err );
        }
	// KErrNotFound is returned for a request to cancel pairing that has no connection.
    if( err )
        {
        aMessage.Complete( err );
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Handle a change in the number of connections.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::HandleLinkCountChangeL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    TInt linkCount = 0;
    User::LeaveIfError( iLinkCount.Get( linkCount ) );
    if( linkCount )
        {
        RBTDevAddrArray links;
        CleanupClosePushL( links );
        User::LeaveIfError( iPhyLinks->Enumerate( links, 10 ) );
        __ASSERT_ALWAYS( links.Count(), PanicServer( EBTNotifPanicBadState ) );
        for( TInt i = iConnArray.Count() -1; i >= 0 ; i-- )
            {
            // Loop backwards, as we may remove entries from the array.

            // First check the existing connections, and 
            // remove disconnected links
            TBTDevAddr addr = iConnArray[i]->Address();
            TInt pos = links.Find( addr );
            if( pos > KErrNotFound )
                {
                // The link we know is still connected,
                // remove the watceher from the array.
                links.Remove( pos );
                // ToDo: see comment below!
                }
            else if( iConnArray[i]->CurrentOperation() == CBTNotifConnection::EIdle )
                {
                // This link is no more connected and idle, remove.
                CBTNotifConnection* connection = iConnArray[i];
                iConnArray.Remove( i ); // Does not delete the object.
                delete connection;
                }
            // else we wait for the link to complete its operations.
            }
        // Now we have an array with only the new connections.
        // Add new watchers.
        for( TInt i = 0; i < links.Count(); i++ )
            {
            CBTNotifConnection* connection = CBTNotifConnection::NewLC( links[i], this );
            iConnArray.AppendL( connection );
            CleanupStack::Pop( connection );
            }
        // Close the links RBTDevAddrArray, needed before going out of scope.
        CleanupStack::PopAndDestroy();
        }
    else
        {
        for( TInt i = iConnArray.Count() -1; i >= 0 ; i-- )
            {
            if( iConnArray[i]->CurrentOperation() == CBTNotifConnection::EIdle )
                {
                // This link is now idle, so we can remove it safely.
                CBTNotifConnection* connection = iConnArray[i];
                iConnArray.Remove( i ); // Does not delete the object.
                delete connection;
                }
            }
        if( !iConnArray.Count() )
            {
            // The array is idle, clean up the array resources.
            iConnArray.Reset();
            }
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Check if this device has been denied a connection already before.
// Also check if a previous connection attempt has just been rejected.
// ---------------------------------------------------------------------------
//
TBool CBTNotifConnectionTracker::UpdateBlockingHistoryL( const CBTDevice* aDevice, 
    TBool aAccepted )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    __ASSERT_ALWAYS( aDevice, PanicServer( EBTNotifPanicBadArgument ) );
    // Check the time since the previous event.
    TBool result = RecordConnectionAttempts( aAccepted );
    TInt pos = iDeniedRequests.Find( aDevice->BDAddr() );
    if( !aAccepted )
        {
        if( pos == KErrNotFound )
            {
            // The user denied the request from a new device, record the device address.
            if( aDevice->IsValidPaired() && aDevice->IsPaired() )
                //[MCL]: && iDevice->LinkKeyType() != ELinkKeyUnauthenticatedUpgradable )
                {
                // Paired devices are allowed one time rejection without a prompt for blocking.
                result = EFalse;
                }
            iDeniedRequests.AppendL( aDevice->BDAddr() );
            }
        // Nothing needed here if the address is already in the array.
        }
    else if( pos > KErrNotFound )
        {
        // The user accepted a request, and it was from a device he/she 
        // previously rejected. Clear the history for this device from the array.
        iDeniedRequests.Remove( pos );
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    return result;
    }


// ---------------------------------------------------------------------------
// From class MBluetoothPhysicalLinksNotifier.
// Handle baseband connection completion.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::HandleCreateConnectionCompleteL( TInt aErr )
    {
    BOstraceFunctionEntryExt ( DUMMY_LIST, this, aErr );
	// We only connect links for starting outgoing bonding.
	const RMessage2* message = iServer->FindMessageFromUid( (TInt) EBTEngPairDevice );
	if( message )
		{
        TPckgBuf<TBTDevAddr> addrBuf;
        message->ReadL( EBTNotifSrvParamSlot, addrBuf );
        CBTNotifConnection* connection = FindConnectionHandler( addrBuf() );
        __ASSERT_ALWAYS( connection, PanicServer( EBTNotifPanicBadState ) );
        if( !aErr && connection->CurrentOperation() == CBTNotifConnection::EIdle )
            {
			TRAP( aErr, connection->StartBondingL( *message ) );
			}
        if( aErr && connection->CurrentOperation() == CBTNotifConnection::EBonding )
            {
            connection->PairingResult( aErr );  // Launch error note
            }
		}
	BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// From class MBluetoothPhysicalLinksNotifier.
// Handle baseband disconnection.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::HandleDisconnectCompleteL( TInt aErr )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
	// We only disconnect links for starting outgoing bonding.
	const RMessage2* message = iServer->FindMessageFromUid( (TInt) EBTEngPairDevice );
	if( message )
		{
        TPckgBuf<TBTDevAddr> addrBuf;
        message->ReadL( EBTNotifSrvParamSlot, addrBuf );
		if( !aErr )
			{
			aErr = iPhyLinks->CreateConnection( addrBuf() );
			}
		if( aErr )
			{
			iServer->CompleteMessage( message->Handle(), aErr, KNullDesC8 );
            CBTNotifConnection* connection = FindConnectionHandler( addrBuf() );
            __ASSERT_ALWAYS( connection, PanicServer( EBTNotifPanicBadState ) );
            connection->PairingResult( aErr );  // Launch error note
			}
    	}
	BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// From class MBluetoothPhysicalLinksNotifier.
// Handle disconnection of all links.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::HandleDisconnectAllCompleteL( TInt aErr )
    {
    (void) aErr;
    }


// ---------------------------------------------------------------------------
// From class MBTEngConnObserver.
// Handle service-level connection completion.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::ConnectComplete( TBTDevAddr& aAddr, 
    TInt aErr, RBTDevAddrArray* aConflicts )
    {
    (void) aAddr;
    (void) aErr;
    (void) aConflicts;
    }


// ---------------------------------------------------------------------------
// From class MBTEngConnObserver.
// Handle service-level disconnection.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
    {
    (void) aAddr;
    (void) aErr;
    }


// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Handle the active object completion.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::RequestCompletedL( CBtSimpleActive* aActive,
    TInt aStatus )
    {
    BOstraceFunctionEntryExt ( DUMMY_LIST, this, aActive->RequestId() );
    BOstraceExt2( TRACE_DEBUG, DUMMY_DEVLIST, 
            "CBTNotifConnectionTracker::MBAORequestCompletedL() requestid=%d status=%d", 
            aActive->RequestId(), aStatus);
    if( aActive->RequestId() == KLinkCountWatcher )
        {
        iLinkCount.Subscribe( aActive->RequestStatus() );
        aActive->GoActive();
        if( !aStatus )
            {
            // HandleLinkCountChangeL();
            }
        }
// ToDo: remove this when registry notifications API is available!!
    else if( aActive->RequestId() == KRegistryWatcher )
        {
        // BTRegistry notifies of a change. Check which one.
        iRegistryChange.Subscribe( aActive->RequestStatus() );
        aActive->GoActive();
        TInt tableChanged = 0;
        if( !aStatus && !iRegistryChange.Get( tableChanged ) &&
            tableChanged == KRegistryChangeRemoteTable )
            {
            // A record for a remote device has changed. Tell all 
            // connections to update their record.
            for( TInt i = 0; i < iConnArray.Count(); i++ )
                {
                // Reuse the functionality in the connection
                if( iConnArray[i]->CurrentOperation() < CBTNotifConnection::EReadingRegistry )
                    {
                    iConnArray[i]->RequestCompletedL( aActive, aStatus );
                    }
                }
            }
        }
// End ToDo
    else if( aActive->RequestId() == KSspResultWatcher )
        {
        iSspResultSession.SimplePairingResult( iSspResultAddr, iSspResultActive->RequestStatus() );
        iSspResultActive->GoActive();
        CBTNotifConnection* connection = FindConnectionHandler( iSspResultAddr );
        // ToDo: how to handle a result of a link that already disconnected? 
        if( connection )
            {
            connection->PairingResult( aStatus );
            }
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Cancel and clean up all requests related to the active object.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::CancelRequest( TInt aRequestId )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if( aRequestId == KLinkCountWatcher )
        {
        iLinkCount.Cancel();
        }
    else if( aRequestId == KSspResultWatcher )
        {
        iSspResultSession.CancelSimplePairingResult();
        }
    else if ( aRequestId == KRegistryWatcher )
        {
        iRegistryChange.Cancel();
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// 
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::HandleError( CBtSimpleActive* aActive, 
        TInt aError )
    {
    (void) aActive;
    (void) aError;
    }

// ---------------------------------------------------------------------------
// Parse the details from a client message and find the associated handler.
// ---------------------------------------------------------------------------
//
CBTNotifConnection* CBTNotifConnectionTracker::FindConnectionFromMessageL(
    TInt aOpcode, const RMessage2& aMessage, TDes8& aBuffer )
    {
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    TInt uid = aMessage.Int0();
    aMessage.ReadL( EBTNotifSrvParamSlot, aBuffer );
    TBTDevAddr addr = ParseAddressL( uid, aBuffer );
    // If this is a 
    CBTNotifConnection* connection = FindConnectionHandler( addr );
    if( !connection && IsStackSecmanNotifier( uid ) &&
        ( aOpcode == EBTNotifStartAsyncNotifier || aOpcode == EBTNotifStartSyncNotifier ) )
        {
        // A notifier from stack. This happens if e.g. the pairing
        // request comes in before the link count changes (like security
        // mode 3). Create the handler and queue the request.
        // And note that 
        connection = CBTNotifConnection::NewLC( addr, this );
        iConnArray.AppendL( connection );
        CleanupStack::Pop( connection );
        }
    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, connection );
    return connection;
    }


// ---------------------------------------------------------------------------
// read the address from a client message.
// ---------------------------------------------------------------------------
//
TBTDevAddr CBTNotifConnectionTracker::ParseAddressL( TInt aUid,
    const TDesC8& aParamsBuf ) const
    {
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    TBTDevAddr addr;
    if( IsStackSecmanNotifier( aUid ) )
        {
        // For all these, the address is the first data member,
        // so can be read using the TBTNotifierParams data structure.
        TBTNotifierParams params;
        TPckgC<TBTNotifierParams> paramsPckg( params );
        paramsPckg.Set( aParamsBuf );
        addr = paramsPckg().iBDAddr;
        }
    //else if(  ) other notifier types
    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, &addr );  
    return addr;
    }


// ---------------------------------------------------------------------------
// Find a specific connection.
// ---------------------------------------------------------------------------
//
CBTNotifConnection* CBTNotifConnectionTracker::FindConnectionHandler(
    const TBTDevAddr& aAddr ) const
    {
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    CBTNotifConnection* conn = NULL;
    if( aAddr != TBTDevAddr() )
        {
        // This may be replaced by RArray::Find with appropriate key
        for( TInt i = 0; i < iConnArray.Count(); i++ )
            {
            if( iConnArray[i]->Address() == aAddr )
                {
                conn = iConnArray[i];
                break;
                }
            }
        }
    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, conn );
    return conn;
    }


// ---------------------------------------------------------------------------
// Record and check the time between connection attempts.
// ---------------------------------------------------------------------------
//
TBool CBTNotifConnectionTracker::RecordConnectionAttempts( TBool aAccepted )
    {
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    TBool result = ETrue;
    TTime now( 0 );
    if( !aAccepted )
        {
        now.UniversalTime();
        if( iLastReject )
            {
            // Check the time between denied connections, that it does not go too fast.
            TTimeIntervalSeconds prev( 0 );
            if( !now.SecondsFrom( TTime( iLastReject ), prev ) )
                {
                if( prev <= KDENYTHRESHOLD )
                    {
                    // We are getting the requests too fast. Present the user with
                    // an option to turn BT off.
                    //iServer->SettingsTracker()->SetPower( EFalse );
                    result = EFalse;
                    }
                }
            }
        }
    // Record the current timestamp.
    // It is reset in case the user accepted the request.
    iLastReject = now.Int64();
    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, result );
    return result;
    }
