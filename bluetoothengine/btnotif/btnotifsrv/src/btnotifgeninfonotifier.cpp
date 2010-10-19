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

#include <e32base.h>
#include <hb/hbcore/hbdevicedialogsymbian.h>
#include <btnotif.h>
#include <btservices/btdevextension.h>
#include <btservices/btdevrepository.h>
#include "btnotifgeninfonotifier.h"

#include "btnotifserver.h"
#include "btnotifclientserver.h"
#include "bluetoothnotification.h"
#include "btnotificationmanager.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifGenInfoNotifier::CBTNotifGenInfoNotifier( CBTNotifServer* aServer )
:   iServer( aServer ),iNotification(NULL),iEnterSAPNotification(NULL)
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifGenInfoNotifier::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotifGenInfoNotifier* CBTNotifGenInfoNotifier::NewL( CBTNotifServer* aServer )
    {
    CBTNotifGenInfoNotifier* self = new( ELeave ) CBTNotifGenInfoNotifier( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifGenInfoNotifier::~CBTNotifGenInfoNotifier()
    {
    if( iNotification )
        {
        // Clear the notification callback, we cannot receive them anymore.
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        }
    }

// ---------------------------------------------------------------------------
// Process a client message related to notifiers.
// ---------------------------------------------------------------------------
//
void CBTNotifGenInfoNotifier::HandleNotifierRequestL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt ( DUMMY_LIST, this, aMessage.Function() );
    TInt opcode = aMessage.Function();
    TInt uid = aMessage.Int0();
    TInt err = KErrNone;
    switch ( opcode ) 
        {
        case EBTNotifCancelNotifier:
            {
            aMessage.Complete( err );
            break;
            }
        case EBTNotifUpdateNotifier:
            {
            aMessage.Complete( err );
            break;
            }
        case EBTNotifStartSyncNotifier:
            {
            TRAP(err,ShowNotificationL(aMessage));
            aMessage.Complete( err );
            break;
            }
        case EBTNotifStartAsyncNotifier:
            {
            TRAP(err,ShowNotificationL(aMessage));
            aMessage.Complete( err );
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
// From class MBTNotificationResult.
// Handle a result from a user query.
// ---------------------------------------------------------------------------
//
void CBTNotifGenInfoNotifier::MBRDataReceived( CHbSymbianVariantMap& aData )
    {
    (void) aData;
    }


// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// The notification is finished.
// ---------------------------------------------------------------------------
//
void CBTNotifGenInfoNotifier::MBRNotificationClosed( TInt aError, const TDesC8& aData  )
    {
    (void) aError;
    (void) aData;
    iNotification->RemoveObserver();
    iNotification = NULL;
    }

// ---------------------------------------------------------------------------
// Get and configure a notification.
// ---------------------------------------------------------------------------
//
void CBTNotifGenInfoNotifier::ShowNotificationL(const RMessage2& aMessage )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // read the message parameters
    RBuf8 params;
    params.CreateL( aMessage.GetDesLengthL( EBTNotifSrvParamSlot ) );
    aMessage.ReadL( EBTNotifSrvParamSlot, params );
    TBTGenericInfoNotifierParams notifparams;
    TPckgC<TBTGenericInfoNotifierParams> paramsPckg( notifparams );
    paramsPckg.Set( params );
    TInt notifType = paramsPckg().iMessageType;

    TInt err = KErrNone;

    if(iNotification)
        {
        iNotification->RemoveObserver();
        iNotification = NULL;
        }
    iNotification = iServer->NotificationManager()->GetNotification();
    User::LeaveIfNull( iNotification ); // For OOM exception, leaves with KErrNoMemory
    iNotification->SetObserver( this );
    
    switch(notifType)
        {
        case EBTSwitchedOn:
        case EBTSwitchedOff:
            {
            User::LeaveIfError(iNotification->SetData( TBluetoothDeviceDialog::EAdditionalInt, notifType));
            iNotification->SetNotificationType( TBluetoothDialogParams::ENote, EGenericInfo );
            }break;
        case EBTEnterSap:
            // save the pointer in order to dismiss the dialog later on
            iEnterSAPNotification = iNotification;
            iNotification->SetNotificationType( TBluetoothDialogParams::bt_052_d_entering, EUnusedResource );
            break;
        case EBTSapNoSim:
            iNotification->SetNotificationType( TBluetoothDialogParams::bt_053_d_unable_to_use_no_sim, EUnusedResource );
            break;
        case EBTSapFailed:
            iNotification->SetNotificationType( TBluetoothDialogParams::bt_053_d_unable_to_use, EUnusedResource );
            break;
        case EBTSapOk:
            if(iEnterSAPNotification)
                iEnterSAPNotification->Close();
            iNotification->SetNotificationType( TBluetoothDialogParams::bt_054_d_entered_popup, EGenericInfo );
            // Todo: handler err in a proper way
            err = iNotification->SetData( TBluetoothDeviceDialog::EAdditionalInt, notifType);
            break;
        default:
            {
            iNotification->SetNotificationType( TBluetoothDialogParams::ENote, EGenericInfo );
            TBTDevAddr addr(paramsPckg().iRemoteAddr);

            // Get the device name
            const CBtDevExtension* dev = iServer->DevRepository().Device(addr);
            if(dev)
                {
                if(0 != dev->Device().FriendlyName().Length()&& dev->Device().IsValidFriendlyName())
                    {
                    // If we have a friendly name use it for the UI
                    err = iNotification->SetData( TBluetoothDeviceDialog::EDeviceName, dev->Device().FriendlyName() );
                    }
                else
                    {
                    err = iNotification->SetData( TBluetoothDeviceDialog::EDeviceName, dev->Alias() );
                    }
                if(!err)
                    {
                    err = iNotification->SetData( TBluetoothDeviceDialog::EDeviceClass, dev->Device().DeviceClass().DeviceClass());
                    }
                }
            else
                {
                TBTDeviceName name;
                addr.GetReadable(name);
                err = iNotification->SetData( TBluetoothDeviceDialog::EDeviceName, name);
                }
            if(!err)
                {
                err = iNotification->SetData( TBluetoothDeviceDialog::EAdditionalInt, notifType);
                }
            }break;
        }
    
    iServer->NotificationManager()->QueueNotificationL( iNotification,CBTNotificationManager::EPriorityHigh );
    params.Close();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


