/*
* ============================================================================
*  Name        : btnotifsession.cpp
*  Part of     : bluetoothengine / btnotif
*  Description : Session class for handling commands from clients.
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

#include "btnotifsession.h"
#include <btextnotifiers.h>
#include "btnotifclientserver.h"
#include "btnotifsettingstracker.h"
#include "btnotifconnectiontracker.h"
#include "btnotifdeviceselector.h"


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Start the server.
// ---------------------------------------------------------------------------
//
void LeaveIfNullL( const TAny* aPtr, TInt aLeaveCode )
    {
    if( aPtr == NULL )
        {
        User::Leave( aLeaveCode );
        }
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifSession::CBTNotifSession()
:   CSession2()
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifSession::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotifSession* CBTNotifSession::NewL()
    {
    CBTNotifSession* self = new( ELeave ) CBTNotifSession();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifSession::~CBTNotifSession()
    {
    for( TInt i = 0; i < iMessageQ.Count(); i++ )
        {
        // Complete all outstanding messages with error code
        iMessageQ[i].Complete( KErrSessionClosed );
        }
    iMessageQ.Close();  // Cleans up all message objects.
    Server()->RemoveSession();
    }

// ---------------------------------------------------------------------------
// From class CSession2.
// Receives a message from a client.
// ---------------------------------------------------------------------------
//
void CBTNotifSession::ServiceL( const RMessage2& aMessage )
    {
    TInt opCode = aMessage.Function();
    if ( opCode == EBTNotifCancelNotifier || 
         opCode == EBTNotifStartSyncNotifier ||
         opCode == EBTNotifStartAsyncNotifier ||
         opCode == EBTNotifUpdateNotifier )
        {
        TInt uid = aMessage.Int0();
        if( uid == KDeviceSelectionNotifierUid.iUid )
            {
            // Note for implementers:
            // message queue is not used in this notifier handling (due
            // to its drawbacks for exception handlings in various situations). 
            // implementations using message queue will be migrated step
            // by step.
        
            TRAPD( err, {
                CBTNotifDeviceSelector& selector = Server()->DeviceSelectorL();
                selector.DispatchNotifierMessageL( aMessage ); }
                );
            if ( err )
                {
                aMessage.Complete( err );
                }
            // deviceselector takes the ownership of aMessage.
            return;
            }
        }

    // Messages are completed by message handlers, not here.
    // Queue the message already so that handlers can find it from the queue.
    iMessageQ.AppendL( aMessage );
    // The position is assumed to not change during the execution of this function.
    TInt handle = aMessage.Handle();    // Store the handle for de-queueing
    TRAPD( err, DispatchMessageL( aMessage ) );
    if( err || ( aMessage.IsNull() && FindMessageFromHandle( handle ) ) )
        {
        // If the message has been completed by now (handle is null and the message 
        // still in the queue), we remove it again from the queue. Otherwise it 
        // will be completed by the handling handler when it has handled the handling.
        for( TInt i = 0; i < iMessageQ.Count(); i++ )
            {
            // This may be replaced by RArray::Find with appropriate key
            if( iMessageQ[i].Handle() == handle )
                {
                iMessageQ.Remove( i );
                }
            }
        }
    if( err && !aMessage.IsNull() )
        {
        aMessage.Complete( err );
        }
    }


// ---------------------------------------------------------------------------
// From class CSession2.
// Completes construction of the session.
// ---------------------------------------------------------------------------
//
void CBTNotifSession::CreateL()
    {
    Server()->AddSession();
    }

// ---------------------------------------------------------------------------
// Complete a client message from a message handle with given data.
// If a zero-length descriptor is passed, no data will be written back.
// ---------------------------------------------------------------------------
//
TInt CBTNotifSession::CompleteMessage( TInt aHandle, TInt aReason, const TDesC8& aReply )
    {
    TInt err = KErrNotFound;
    // This may be replaced by RArray::Find with appropriate key
    for( TInt i = 0; i < iMessageQ.Count(); i++ )
        {
        if( iMessageQ[i].Handle() == aHandle )
            {
            err = KErrNone;
            if( aReply.Length() )
                {
                // For now, assume a fixed index for the result.
                // Change this if a the client can pass more arguments!
                // ToDo: replace with constant!
                err = iMessageQ[i].Write( EBTNotifSrvReplySlot, aReply );
                // Should the result be passed back to the calller,
                // or used to complete the message?
                }
            iMessageQ[i].Complete( aReason );
            iMessageQ.Remove( i );
            break;
            }
        }
    return err;
    }


// ---------------------------------------------------------------------------
// Find a client message from an RMessage2 handle.
// ---------------------------------------------------------------------------
//
const RMessage2* CBTNotifSession::FindMessageFromHandle( TInt aHandle ) const
    {
    // This may be replaced by RArray::Find with appropriate key
    for( TInt i = 0; i < iMessageQ.Count(); i++ )
        {
        if( iMessageQ[i].Handle() == aHandle )
            {
            return &iMessageQ[i];
            }
        }
    return NULL;
    }


// ---------------------------------------------------------------------------
// Process a client message.
// The processing here relies on RNotifier backend server for queueing 
// notifiers on the same channel. Therefore pairing (SSP and legacy) and 
// authorization notifiers arrive in order, not simultaneously, even if 
// they use arrive on different session instances.
// ---------------------------------------------------------------------------
//
void CBTNotifSession::DispatchMessageL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aMessage.Function() );
    CBTNotifSettingsTracker* settTracker = Server()->SettingsTracker();
    CBTNotifConnectionTracker* connTracker = Server()->ConnectionTracker();
    LeaveIfNullL( settTracker, KErrNotReady );
    TInt opcode = aMessage.Function();
    if( opcode < EBTNotifMinValue )
        {
        User::Leave( KErrArgument );
        }
    switch( opcode )
        {
        case EBTNotifCancelNotifier:
        case EBTNotifStartSyncNotifier:
        case EBTNotifStartAsyncNotifier:
        case EBTNotifUpdateNotifier:
            {
            // All these messages get the same treatment: forward 
            // to settings and connection tracker, who will deal with it appropriately.
            // First the settings tracker handles the message.
            settTracker->DispatchNotifierMessageL( aMessage );
            if( connTracker && !aMessage.IsNull() )
                {
                // Pass it on to the connection tracker, if it hasn't been completed yet.
                connTracker->DispatchNotifierMessageL( aMessage );
                }
            else
                {
                // Power is off, can't do this now.
                LeaveIfNullL( connTracker, KErrNotReady );
                }
            if( opcode != EBTNotifStartAsyncNotifier && !aMessage.IsNull() )
                {
                // Nobody has yet completed the message, and it is a synchronous
                // one so we'll do it here to allow the notifier to keep on going.
                aMessage.Complete( KErrNone );
                }
            }
            break;
        case EBTEngPrepareDiscovery:
            {
            // This is functionality only related to existing connections.
            // Can't do when power is off though.
            LeaveIfNullL( connTracker, KErrNotReady );
            //connTracker->HandlePairingRequestL( aMessage );
            }
            break;
        case EBTEngPairDevice:
        case EBTEngCancelPairDevice:
            {
            // This is functionality only related to connections.
            // Can't do when power is off though.
            LeaveIfNullL( connTracker, KErrNotReady );
            connTracker->HandleBondingRequestL( aMessage );
            }
            break;
        default:
            // Communicate result back.
            User::Leave( KErrNotSupported );
            break;
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Find a client message from an RNotifier UID.
// ---------------------------------------------------------------------------
//
const RMessage2* CBTNotifSession::FindMessageFromUid( TInt aUid ) const
    {
    // This may be replaced by RArray::Find with appropriate key
    for( TInt i = 0; i < iMessageQ.Count(); i++ )
        {
        if( iMessageQ[i].Int0() == aUid )
            {
            return &iMessageQ[i];
            }
        }
    return NULL;
    }
