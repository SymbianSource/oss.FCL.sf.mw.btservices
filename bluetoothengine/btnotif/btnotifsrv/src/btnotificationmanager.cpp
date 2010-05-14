/*
* ============================================================================
*  Name        : btnotificationmanager.cpp
*  Part of     : bluetoothengine / btnotif
*  Description : Class for managing user notification and query objects, and for serializing access to the notification server.
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

#include "btnotificationmanager.h"
#include "btnotifserver.h"

#include "bluetoothnotification.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotificationManager::CBTNotificationManager( const CBTNotifServer* aServer )
:   iServer( aServer )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotificationManager::ConstructL()
    {
    iAsyncCb = new( ELeave ) CAsyncCallBack( iServer->Priority() );
    TCallBack cb( AsyncCallback, this );
    iAsyncCb->Set( cb );
    }


// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotificationManager* CBTNotificationManager::NewL( const CBTNotifServer* aServer )
    {
    CBTNotificationManager* self = new( ELeave ) CBTNotificationManager( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotificationManager::~CBTNotificationManager()
    {
    iNotificationQ.ResetAndDestroy();
    iNotificationQ.Close();
    iUnusedQ.ResetAndDestroy();
    iUnusedQ.Close();
    delete iAsyncCb;
    }


// ---------------------------------------------------------------------------
// Get a new notification
// ---------------------------------------------------------------------------
//
CBluetoothNotification* CBTNotificationManager::GetNotification()
    {
    CBluetoothNotification* notification = NULL;
    if( iUnusedQ.Count() )
        {
        // Re-use the first idle notification.
        notification = iUnusedQ[0];
        iUnusedQ.Remove( 0 );
        }
    else
        {
        TRAP_IGNORE( notification = CBluetoothNotification::NewL( this ) );
        }
    if( notification )
        {
        if( iNotificationQ.Append( notification ) )
            {
            // In case the appending fails, we just delete the notification.
            // Otherwise we cannot keep track of it anymore.
            delete notification;
            notification = NULL;
            }
        }
    return notification;
    }


// ---------------------------------------------------------------------------
// Release the notification
// ---------------------------------------------------------------------------
//
void CBTNotificationManager::ReleaseNotification( CBluetoothNotification* aNotification )
    {
    __ASSERT_ALWAYS( aNotification, PanicServer( EBTNotifPanicBadArgument ) );
    TInt pos = iNotificationQ.Find( aNotification );
    __ASSERT_ALWAYS( pos > KErrNotFound, PanicServer( EBTNotifPanicMissing ) );
    // ToDo: Cancel outstanding notification!
    iNotificationQ.Remove( pos );
    TInt err = iUnusedQ.Append( aNotification );
    aNotification->Reset();  // Clean up notification's resources
    if( err )
        {
        // Just delete the notification.
        delete aNotification;
        }
    if( !iAsyncCb->IsActive() )
        {
        if( !iNotificationQ.Count() )
            {
            // Set the priority so that this is the last scheduled active object to execute.
            iAsyncCb->SetPriority( CActive::EPriorityIdle );
            }
        iAsyncCb->CallBack();
        }
    }


// ---------------------------------------------------------------------------
// Queue the notification with given priority
// ---------------------------------------------------------------------------
//
TInt CBTNotificationManager::QueueNotification( CBluetoothNotification* aNotification,
    TNotificationPriority aPriority )
    {
    TInt pos = iNotificationQ.Find( aNotification );
    __ASSERT_ALWAYS( pos > KErrNotFound, PanicServer( EBTNotifPanicMissing ) );
    if( aPriority == EPriorityHigh && pos != 0 )
        {
        // ToDo:  Move the note to the front of the queue
        }
    if( !iAsyncCb->IsActive() )
        {
		if( iAsyncCb->Priority() != iServer->Priority() )
			{
			// Reset priority back to original value
			// We first check the current priority, otherwise CActive will do an
			// unnecessary removal and adding of the callback from the active scheduler. 
			iAsyncCb->SetPriority( iServer->Priority() );
			}
        iAsyncCb->CallBack();
        }
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// Process the notification queue and launch the next notification.
// ---------------------------------------------------------------------------
//
void CBTNotificationManager::ProcessNotificationQueueL()
    {
    if( iNotificationQ.Count() )
        {
        TInt err = iNotificationQ[0]->Show();
        // If the note is already showing, it will return KErrAlreadyExists
        (void) err; // ToDo: add error handling!!
        NOTIF_NOTHANDLED( !err || err == KErrAlreadyExists || err == KErrNotFound )
        }
    else
        {
        // No outstanding notifications, and unused notifications.
        // Clean up the unused notifications.
        iUnusedQ.ResetAndDestroy();
        iNotificationQ.Reset(); // the queue is empty, reset it.
        // Also clean up any resources.
        }
    }


// ---------------------------------------------------------------------------
// Callback for asynchronous processing of queued notification requests.
// ---------------------------------------------------------------------------
//
TInt CBTNotificationManager::AsyncCallback( TAny* aPtr )
    {
    TRAPD( err, ( (CBTNotificationManager*) aPtr )->ProcessNotificationQueueL() );
    return err;
    }
