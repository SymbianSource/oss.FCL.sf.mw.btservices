/*
* ============================================================================
*  Name        : btnotifpairinghelper.cpp
*  Part of     : bluetoothengine / btnotif
*  Description : Helper class for processing pairing requests and results, as extended functionality for CBTNotifConnection.
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

#include "btnotifpairinghelper.h"
#include <bt_sock.h>
#include <btextnotifiers.h>
#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifierspartner.h>
#endif

#include "btnotifconnection.h"
#include "btnotifconnectiontracker.h"
#include "btnotificationmanager.h"
#include "btnotifserver.h"
#include "bluetoothtrace.h"

/**  Id for the active object for a dedicated bonding session. */
const TInt KDedicatedBonding = 50;
/**  Length of the default PIN. */
const TInt KDefaultPinLength = 4;
/**  Default PIN character. */
const TText8 KDefaultPinValue = '0';
/**  Format syntax for numeric comparison value. */
_LIT( KNumCompFormat, "%06u" );
/**  Format syntax for passkey display value. */
_LIT( KPassKeyFormat, "%06u" );


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// ?description
// ---------------------------------------------------------------------------
//
/*?type ?function_name( ?arg_type ?arg,
                      ?arg_type ?arg )
    {
    }
*/

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor.
// ---------------------------------------------------------------------------
//
CBTNotifPairingHelper::CBTNotifPairingHelper( CBTNotifConnection* aConnection,
    CBTNotifConnectionTracker* aTracker )
:   iConnection( aConnection ),
    iTracker( aTracker )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::ConstructL()
    {
    if( iConnection )
        {
        iDevice = iConnection->BTDevice();
        }
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTNotifPairingHelper* CBTNotifPairingHelper::NewL( CBTNotifConnection* aConnection,
    CBTNotifConnectionTracker* aTracker )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    CBTNotifPairingHelper* self = new( ELeave ) CBTNotifPairingHelper( aConnection, aTracker );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CBTNotifPairingHelper::~CBTNotifPairingHelper()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if( iNotification )
        {
        // Clear the notification callback, we cannot receive them anymore.
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        }
    delete iParams;
    if( iBondingActive )
        {
        iBondingActive->Cancel();   // Will close subsession;
        }
    delete iBondingActive;
    iBondingSession.Close();
    iBondingSocket.Close();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Handle the authentication result from the baseband. Show the result in a note.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::HandleAuthenticationCompleteL( TInt aError )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    if( iOperation == EDedicatedBonding || iOperation == EAwaitingPairingResult ||
        iOperation == EAutoPairing )
        {
        // Default case (aError == 0): Success, we are now paired.
        TBTDialogResourceId resourceId = EPairingSuccess;
        TBool autoPairing = ( iOperation == EAutoPairing ); // Remember the autopairing state
        iOperation = EShowPairingSuccess;
        if( aError && aError != KHCIErrorBase )
            {
            // Authentication failure, means pairing failed.
            resourceId = EPairingFailure;
            iOperation = EShowPairingFailure;
            // Communicate the error now that we still remember it.
            if( iDedicatedBonding )
                {
                if( autoPairing && aError == KHCIErrorBase - EAuthenticationFailure )
                    {
                    BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST, 
                            "[BTNOTIF]\t CBTNotifPairingHelper::HandleAuthenticationCompleteL: Autopairing failed, we need to try again.");
                    // Autopairing failed, we need to try again.
                    iOperation = EAutoPairing;  // Reset back
                    resourceId = ENoResource;
                    }
                CompleteBondingL( aError );
                }
            }
        if( resourceId )
            {
            // Inform the user of the result.
            BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST,  
                    "[BTNOTIF]\t CBTNotifPairingHelper::HandleAuthenticationCompleteL: pairing successful, inform user" );
            PrepareNotificationL( TBluetoothDialogParams::EGlobalNotif, resourceId );
            // MBRNotificationClosed will be called from this, which will 
            // check the next stage.
            }
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Start a bonding operation with the remote device.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::StartBondingL( TInt aHandle )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aHandle );
    __ASSERT_ALWAYS( iOperation == EIdle || iOperation == EDedicatedBonding
                || iOperation == EAutoPairing, PanicServer( EBTNotifPanicBadState ) );
    if( !iBondingActive )
        {
        iBondingActive = CBtSimpleActive::NewL( *this, KDedicatedBonding );
        }
    if( aHandle )
        {
        iDedicatedBonding = aHandle;
        }
    if( iOperation == EIdle )
        {
        iOperation = EDedicatedBonding;
        }
    if( iOperation == EDedicatedBonding && iTracker->PairingServerSession() )
        {
        if( !iBondingActive->IsActive() )
            {
            BtTraceBtAddr1( TRACE_DEBUG,DUMMY_LIST,"CBTNotifPairingHelper::StartBondingL()",iDevice->BDAddr() );
            iBondingSession.Start( *iTracker->PairingServerSession(),
                        iDevice->BDAddr(), iBondingActive->RequestStatus() );
            iBondingActive->GoActive();
            }
        }
    else
        {
        // We are doing autopairing (or the unlikely situation that the pairing server is unavailable)
        CompleteBondingL( KErrServerTerminated );
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this);
    }


// ---------------------------------------------------------------------------
// Cancel an ongoing bonding operation with the remote device.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CancelBondingL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if( iDedicatedBonding )
        {
        CompleteBondingL( KErrCancel ); // Closes sessions
        if( iNotification )
            {
            // Cancel the outstanding user query
            // This will also unregister us from the notification.
            TInt err = iNotification->Close();
            NOTIF_NOTHANDLED( !err )
            iNotification = NULL;
            }
        if( iNotifierUid )
            {
            // Also finish up the notifier processing.
            CompletePairingNotifierL( KErrCancel, EFalse, KNullDesC8 );
            }
        iOperation = EIdle;
        iConnection->PairingCompleted();   // This may delete us.
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Handle a notifier request for pairing with the remote device of this connection.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::StartPairingNotifierL( TInt aUid, const TDesC8& aParams )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aUid );
    if( iDevice->GlobalSecurity().Banned() && !iDedicatedBonding )
        {
        // ToDo: should this case actually be ignored, and presume that
        // the caller will take care of unblocking the device?
        iOperation = EIdle;
        User::Leave( KErrAccessDenied );
        }
    // Store the parameters locally, we need them later again.
    delete iParams;
    iParams = NULL;
    iParams = HBufC8::NewL( aParams.Length() );
    *iParams = aParams;
    iNotifierUid = aUid;

    if( iDevice->IsValidPaired() && iDevice->IsPaired() )
        {
        // The device is still paired, we unpair it first.
        // Deleting the link key will set the device as unpaired.
        iDevice->DeleteLinkKey();
        iOperation = EUnpairing;    // So that parent state does not get changed.
        iConnection->UpdateRegistryEntryL();
        // Note that this will only be done before trying autopairing, so
        // it not interfere with a second attempt;
        }
    // Update the device name
    TBTPasskeyDisplayParams params; // Enough for reading the base class type parameter
    TPckgC<TBTPasskeyDisplayParams> paramsPckg( params );
    paramsPckg.Set( *iParams );
    if( paramsPckg().DeviceName().Length() )
        {
        // The name in the parameter package is the latest one, retrieved from 
        // the remote device during this connection. Update locally.
        iDevice->SetDeviceNameL( BTDeviceNameConverter::ToUTF8L( paramsPckg().DeviceName() ) );
        }

    TBool locallyInitiated = EFalse;
    TBuf<8> numVal;
    TBluetoothDialogParams::TBTDialogType dialog = TBluetoothDialogParams::EInvalidDialog;
    TBTDialogResourceId resource = ENoResource;
    // Read the notifier parameters (sets iOperation as well)
    ParseNotifierReqParamsL( locallyInitiated, numVal, dialog, resource );
    // If this is an incoming pairing, we first ask the user to accept it.
    if( !locallyInitiated && !iDedicatedBonding )
        {
        // Ignore the initatior if we initiated bonding.
        StartAcceptPairingQueryL(); // Overrides iOperation
        }
    else
        {
        __ASSERT_ALWAYS( resource != ENoResource, PanicServer( EBTNotifPanicBadState ) );
        CheckAutoPairingL( locallyInitiated, numVal );
        // CheckAutoPairingL sets 
        if( iOperation != EAutoPairing )
            {
            PrepareNotificationL( dialog, resource );
            if( numVal.Length() )
                {
                TInt err = iNotification->SetData( TBluetoothDeviceDialog::EAdditionalDesc, numVal );
                NOTIF_NOTHANDLED( !err )
                }
            }
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Update a notifier, update the outstanding dialog if the notifier request 
// is currently being served.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::UpdatePairingNotifierL( TInt aUid, const TDesC8& aParams )
    {
    (void) aUid;
    TBTNotifierUpdateParams2 params;    // Enough for reading the base class type parameter
    TPckgC<TBTNotifierUpdateParams2> paramsPckg( params );
    paramsPckg.Set( aParams );
    if( paramsPckg().Type() == TBTNotifierUpdateParams2::EPasskeyDisplay )
        {
        // Paskey display update - keypress on remote device.
        }
    else
        {
        // name update
        TBTDeviceNameUpdateParams nameUpdate;
        TPckgC<TBTDeviceNameUpdateParams> nameUpdatePckg( nameUpdate );
        nameUpdatePckg.Set( aParams );
        // The result means result of conversion to unicode
        if( !nameUpdatePckg().Result() )
            {
            // Only update locally, registry will update us with the same info later on.
            iDevice->SetDeviceNameL( BTDeviceNameConverter::ToUTF8L( nameUpdatePckg().DeviceName() ) );
            if( iNotification )
                {
                // Update the dialog with the new name. It is up to the dialog to 
                // determine the validity (in case another dialog is shown).
                //iNotification->Update(  )
                }
            }
        }
    }


// ---------------------------------------------------------------------------
// Cancel a request, dismiss the outstanding dialog if the notifier request 
// is currently being served.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CancelPairingNotifierL( TInt aUid )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // ToDo: we need to check that the UID and the outstanding notification
    // type are matching?
    if( iOperation > EIdle && iOperation < EPostPairingOperations && aUid == iNotifierUid &&
        ( aUid == KBTPinCodeEntryNotifierUid.iUid ||
          aUid == KBTNumericComparisonNotifierUid.iUid ||
          aUid == KBTPasskeyDisplayNotifierUid.iUid ) )
        {
        if( iNotification )
            {
            // Cancel the user query
            // This will also unregister us from the notification.
            TInt err = iNotification->Close();
            NOTIF_NOTHANDLED( !err )
            iNotification = NULL;
            }
        iOperation = EIdle;
        iNotifierUid = 0;
        // We do not call pairing completed from here, our parent will
        // check our status by itself, and may delete us.

        // Any bonding requester needs to be informed though.
        CancelBondingL();
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::StartJustWorksProcessingL()
    {
    }


// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CancelJustWorksProcessingL()
    {
    }


// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// Handle a result from a user query.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::MBRDataReceived( CHbSymbianVariantMap& aData )
    {
    (void) aData;
    NOTIF_NOTIMPL
    }


// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// The notification is finished.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::MBRNotificationClosed( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    // First unregister from the notification, so we can already get the next one.
    iNotification->RemoveObserver();
    iNotification = NULL;
    TRAP_IGNORE( NotificationClosedL( aError, aData ) );
    if( iOperation == EIdle )
        {
        // Any error has been communicated already.
        iConnection->PairingCompleted();   // This may delete us.
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Handle the active object completion.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::RequestCompletedL( CBtSimpleActive* aActive,
    TInt aStatus )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    switch( aActive->RequestId() )
        {
        case KDedicatedBonding:
            {
            if( iDedicatedBonding )
                {
                // If the result hasn't been processed already.
                HandleAuthenticationCompleteL( aStatus );
                }
            }
            break;
        default:
            NOTIF_NOTIMPL
            break;
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Cancel and clean up all requests related to the active object.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CancelRequest( TInt aRequestId )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    switch( aRequestId )
        {
        case KDedicatedBonding:
            iBondingSession.Close();
            break;
        default:
            NOTIF_NOTIMPL
            break;
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// 
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::HandleError( CBtSimpleActive* aActive, 
        TInt aError )
    {
    (void) aActive;
    (void) aError;
    }

// ---------------------------------------------------------------------------
// Process the user input and complete the outstanding pairing request.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CompletePairingNotifierL( TInt aError, TBool aResult,
    const TDesC8& aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    TInt err = aError;
    TPtrC8 resultData;
    if( !err )
        {
        // The returned data is the entered passkey.
        TBool proceed = iTracker->UpdateBlockingHistoryL( iDevice, aResult );
        if( iOperation == ESspPairing && iNotifierUid == KBTNumericComparisonNotifierUid.iUid )
            {
            // Numeric comparison needs the boolean result passed back.
            TPckgBuf<TBool> userAcceptance( aResult );
            resultData.Set( userAcceptance );
            }
        if( aResult )
            {
            if( iOperation == ELegacyPairing || iOperation == EAutoPairing )
                {
                // Check the passkey entered by the user.
                // The length of the returned data equals the number of characters
                // entered by the user.
                TBTPinCode pinCode;
                pinCode().iLength = aData.Length();
                TUint minLen = 0;
                TBool locallyInitiated = EFalse; // Not used here.
                ParsePinCodeReqParamsL( locallyInitiated, minLen );
                if( aData.Length() >= minLen )
                    {
                    // Check that the length of the passkey meets the minimum 
                    // required pin code length
                    for( TInt i = 0; i < aData.Length(); i++ )
                        {
                        pinCode().iPIN[i] = aData[i];
                        }
                    resultData.Set( pinCode );
                    }
                else
                    {
                    // PIN wasn't long enough. This should be handled by the dialog though.
                    err = KErrCompletion;
                    }
                }
            // Now we just wait for the result to come in.
            if( iOperation != EAutoPairing )
                {
                iOperation = EAwaitingPairingResult; 
                }
            }
        else
            {
            err = KErrCancel;
            TBool locallyInitiated = EFalse;    // Needed below
            TBuf<8> numVal;     // Not needed here.
            TBluetoothDialogParams::TBTDialogType type = TBluetoothDialogParams::EInvalidDialog;
            TBTDialogResourceId resource = ENoResource; // Resources and type are not needed here.
            // Check the notifier parameters
            ParseNotifierReqParamsL( locallyInitiated, numVal, type, resource );
            if( proceed && locallyInitiated && !iDedicatedBonding )
                {
                // The user denied the connection, ask to block the device.
                // This is only for pairing (and not bonding) initiated by us,
                // as the user already gets the opportunity to block when
                // rejecting an incoming pairing request.
                // This case may be for someone requesting to access a service
                // which requires authentication by us, but not by the remote device.
                iConnection->LaunchBlockingQueryL();
                // For incoming pairing, blocking is asked after rejecting the 
                // pairing request. This is done in CompleteAcceptPairingQueryL
                }
            CompleteBondingL( err );    // Notify the client if there was a bonding request.
            }
        }
    iNotifierUid = 0;   // Clean up notifier data
    delete iParams;
    iParams = NULL;
    if( err )
        {
        iOperation = EIdle; // We are done now.
        }
    // Complete the message with the result, and result data if any.
    iConnection->CompleteClientRequest( err, resultData );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Completes a bonding operation.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CompleteBondingL( TInt aError )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    if( iDedicatedBonding )
        {
        if( iBondingActive )
            {
            iBondingActive->Cancel();   // Will close subsession;
            }
        iBondingSession.Close();    // In case not active
        iBondingSocket.Close();
        }
    // Determine if we try another time.
    if( ( iOperation == EAutoPairing && aError == KHCIErrorBase - EAuthenticationFailure ) ||
        ( iDedicatedBonding && iOperation == EAwaitingPairingResult &&
          aError == KErrRemoteDeviceIndicatedNoBonding ) ||
        aError == KErrServerTerminated )
        {
        // The cases are: 2) autopairing with a headset that has a non-default passkey
        // 2) SSP dedicated bonding with a device that does not allow that.
        // 3) the pairing server is unavailable (unlikely)
        // Then we try another time, requesting authentication on a 
        // RBTPhysicialLinkAdapter
        BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST,  
                "[BTNOTIF]\t CBTNotifPairingHelper::CompleteBondingL: trying another time." );
        TInt err = iBondingSocket.Open( iTracker->SocketServerSession(), iConnection->Address() );
        TUint32 linkState = 0;
        if( !err )
            {
            err = iBondingSocket.PhysicalLinkState( linkState );
            }
        if( !err && linkState & ENotifyPhysicalLinkUp )
            {
            err = iBondingSocket.Authenticate();
            // Now wait for the dialog and then the link state notification
            }
        else
            {
            // We need to wait for the link to come up. We wait until our
            // parent calls us again.
            iBondingSocket.Close();
            }
        if( err )
            {
            // Cannot continue, show the result to the user.
            BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST,  
                    "[BTNOTIF]\t CBTNotifPairingHelper::HandleAuthenticationCompleteL: pairing failed, complete message." );
            iOperation = EShowPairingFailure;
            PrepareNotificationL( TBluetoothDialogParams::ENote, EPairingFailure );
            }
        }
    if( iDedicatedBonding && iOperation != EAutoPairing )
        {
        BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST,      
                "[BTNOTIF]\t CBTNotifPairingHelper::CompleteBondingL: complete message." );
        TInt err = iTracker->Server()->CompleteMessage( iDedicatedBonding, aError, KNullDesC8 );
        NOTIF_NOTHANDLED( !err )
        iDedicatedBonding = 0;
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CompleteJustWorksProcessingL( TInt aError )
    {
    (void) aError;
    }


// ---------------------------------------------------------------------------
// Ask the user to allow incoming pairing.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::StartAcceptPairingQueryL()
    {
    iOperation = EAcceptPairing;
    PrepareNotificationL( TBluetoothDialogParams::EQuery, EIncomingPairing );
    // if rejected, the client message is completed in CompleteAcceptPairingQueryL
    }


// ---------------------------------------------------------------------------
// The user was asked to accept an incoming pairing. Process and proceed. 
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CompleteAcceptPairingQueryL( TInt aError, TBool aResult )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // Set our state to idle for now. This may get changed if the user just chose 
    // to block, or if we have a pending request.
    iOperation = EIdle;
    TInt err = aError;
    if( !err )
        {
        TBool proceed = iTracker->UpdateBlockingHistoryL( iDevice, aResult );
        if( aResult )
            {
            // User accepted, continue to show pairing query.
            // Minimum lenght does not apply, should only be set on outgoing pairing
            TBool locallyInitiated = EFalse;
            TBuf<8> numVal;
            TBluetoothDialogParams::TBTDialogType dialog = TBluetoothDialogParams::EInvalidDialog;
            TBTDialogResourceId resource = ENoResource;
            // Read the notifier parameters
            ParseNotifierReqParamsL( locallyInitiated, numVal, dialog, resource );
            __ASSERT_ALWAYS( resource != ENoResource, PanicServer( EBTNotifPanicBadState ) );
            PrepareNotificationL( dialog, resource );
            if( numVal.Length() )
                {
                TInt err = iNotification->SetData( TBluetoothDeviceDialog::EAdditionalDesc, numVal );
                NOTIF_NOTHANDLED( !err )
                }
            }
        else
            {
            err = KErrCancel;
            if( proceed )
                {
                // The user denied the connection, ask to block the device.
                iConnection->LaunchBlockingQueryL();
                }
            }
        }
    if( err )
        {
        // The user denied the connection, or something else prevented completion.
        CompletePairingNotifierL( err, EFalse, KNullDesC8 );
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Launch a dialog for setting the device as trusted.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::StartTrustedQueryL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // Assume that the registry update has come through by now.
    iOperation = EQueryTrust;
    PrepareNotificationL( TBluetoothDialogParams::EQuery, ESetTrusted );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Process the user input for setting the device as trusted.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CompleteTrustedQueryL( TInt aError, TBool aResult )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    BOstraceExt2( TRACE_DEBUG, DUMMY_DEVLIST, 
            "CBTNotifPairingHelper::CompleteTrustedQueryL() err=%d result=%d", aError, aResult );
    iOperation = EIdle; // We are done with pairing now.
    if( !aError && aResult )
        {
        // need to update pairing info from registry before writing trusted status
        iConnection->UpdateRegistryEntryL(true);
        }
    CompleteBondingL( KErrNone );   // Notify the client if there was a bonding request.
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Parse the parameters of a request for pairing.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::ParseNotifierReqParamsL( TBool& aLocallyInitiated,
    TDes& aNumVal, TBluetoothDialogParams::TBTDialogType& aDialogType,
    TBTDialogResourceId& aResourceId )
    {
    // Determine the notifier type by the length of the parameter buffer
    if( iNotifierUid == KBTPinCodeEntryNotifierUid.iUid )
        {
        aNumVal.Zero();
        TUint minLen = 0;
        ParsePinCodeReqParamsL( aLocallyInitiated, minLen );
        if( minLen )
            {
            // Don't set zero to this buffer, the buffer length serves for this.
            aNumVal.Num( minLen );
            }
        aDialogType = TBluetoothDialogParams::EInput;
        aResourceId = EPinInput;
        if( iOperation != EAutoPairing )
            {
            iOperation = ELegacyPairing;
            }
        }
    else if( iNotifierUid == KBTNumericComparisonNotifierUid.iUid )
        {
        ParseNumericCompReqParamsL( aLocallyInitiated, aNumVal );
        aDialogType = TBluetoothDialogParams::EQuery;
        aResourceId = ENumericComparison;
        iOperation = ESspPairing;
        }
    else if( iNotifierUid == KBTPasskeyDisplayNotifierUid.iUid )
        {
        ParsePasskeyDisplayReqParamsL( aLocallyInitiated, aNumVal );
        aDialogType = TBluetoothDialogParams::EQuery;
        aResourceId = EPasskeyDisplay;
        iOperation = ESspPairing;
        }
    }


// ---------------------------------------------------------------------------
// Parse the parameters of a request for pairing using pin query.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::ParsePinCodeReqParamsL( TBool& aLocallyInitiated,
    TUint& aNumVal )
    {
    TBTPinCodeEntryNotifierParams params;
    TPckgC<TBTPinCodeEntryNotifierParams> paramsPckg( params );
    paramsPckg.Set( *iParams );
    aLocallyInitiated = paramsPckg().LocallyInitiated();
    aNumVal = paramsPckg().PinCodeMinLength();
    }


// ---------------------------------------------------------------------------
// Parse the parameters of a request for pairing using numeric comparison.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::ParseNumericCompReqParamsL( TBool& aLocallyInitiated,
    TDes& aNumVal )
    {
    TBTNumericComparisonParams params;
    TPckgC<TBTNumericComparisonParams> paramsPckg( params );
    paramsPckg.Set( *iParams );
    aLocallyInitiated = paramsPckg().LocallyInitiated();
    TBTNumericComparisonParams::TComparisonScenario scenario =
                paramsPckg().ComparisonScenario();
    aNumVal.Format( KNumCompFormat, paramsPckg().NumericalValue() );
    }


// ---------------------------------------------------------------------------
// Parse the parameters of a request for pairing using passkey display.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::ParsePasskeyDisplayReqParamsL( TBool& aLocallyInitiated,
    TDes& aNumVal )
    {
    TBTPasskeyDisplayParams params;
    TPckgC<TBTPasskeyDisplayParams> paramsPckg( params );
    paramsPckg.Set( *iParams );
    aLocallyInitiated = paramsPckg().LocallyInitiated();
    aNumVal.Format( KPassKeyFormat, paramsPckg().NumericalValue() );
    }


// ---------------------------------------------------------------------------
// Check if we can guess the PIN and complete the notifier without user interaction.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::CheckAutoPairingL( TBool aLocallyInitiated, const TDesC& aNumVal )
    {
    TUint minLen = 0;
    if( aNumVal.Length() )
        {
        ParsePinCodeReqParamsL( aLocallyInitiated, minLen );
        }
    // ToDo: Add support for NFC OOB pairing
    if( iDedicatedBonding && iOperation == ELegacyPairing &&
        iDevice->DeviceClass().MajorDeviceClass() == EMajorDeviceAV &&
        iDevice->DeviceClass().MinorDeviceClass() != EMinorDeviceAVHandsfree &&
        minLen <= KDefaultPinLength )
        {
        // Outgoing bonding with headset and no passkey requirement => AutomatedPairing
        // Complete message with 0000 and return.
        iOperation = EAutoPairing;
        TBuf8<KDefaultPinLength + sizeof( TPckgBuf<TBool> )> defaultPin( KDefaultPinLength );
        for( TInt i = 0; i < KDefaultPinLength; i++ )
            {
            defaultPin[i] = KDefaultPinValue;
            }
        // Complete the pairing through the function dedicated to that.
        CompletePairingNotifierL( KErrNone, ETrue, defaultPin );
        }
    else if( iOperation == EAutoPairing )
        {
        iOperation = ELegacyPairing;    // Reset the autopairing status
        }
    }


// ---------------------------------------------------------------------------
// Get and configure a notification.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::PrepareNotificationL( TBluetoothDialogParams::TBTDialogType aType,
    TBTDialogResourceId aResourceId )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    __ASSERT_ALWAYS( iOperation != EIdle || aType == TBluetoothDialogParams::ENote, PanicServer( EBTNotifPanicBadState ) );
    iNotification = iTracker->NotificationManager()->GetNotification();
    User::LeaveIfNull( iNotification ); // For OOM exception, leaves with KErrNoMemory
    iNotification->SetObserver( this );
    iNotification->SetNotificationType( aType, aResourceId );
    TBTDeviceName name;
    GetDeviceNameL( name, *iDevice );
    TInt err = iNotification->SetData( TBluetoothDeviceDialog::EDeviceName, name );
    NOTIF_NOTHANDLED( !err )
    // Re-use name buffer for 16-bit descriptor representation of remote address.
    iConnection->Address().GetReadable( name );
    err = iNotification->SetData( TBluetoothDialogParams::EAddress, name );
    NOTIF_NOTHANDLED( !err )
    err = iNotification->SetData( (TInt) TBluetoothDeviceDialog::EDeviceClass,
                iDevice->DeviceClass().DeviceClass() );
    err = iTracker->NotificationManager()->QueueNotification( iNotification );
    NOTIF_NOTHANDLED( !err )
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// The notification is finished, handle the result.
// ---------------------------------------------------------------------------
//
void CBTNotifPairingHelper::NotificationClosedL( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    // Read the result.
    TPckgC<TBool> result( EFalse );
    result.Set( aData.Ptr(), result.Length() ); // Read the part containing the result
    // Set a pointer descriptor to capture the remaining data, if any.
    TPtrC8 dataPtr( aData.Mid( result.Length() ) );
    switch( iOperation )
        {
        case EAcceptPairing:
            CompleteAcceptPairingQueryL( aError, result() );
            break;
        case ELegacyPairing:
        case ESspPairing:
            CompletePairingNotifierL( aError, result(), dataPtr );
            break;
        case EQueryTrust:
            CompleteTrustedQueryL( aError, result() );
            break;
        case EShowPairingSuccess:
            StartTrustedQueryL();
            break;
        case EShowPairingFailure:
            // Pairing failure, we are done.
            iOperation = EIdle;
            break;
        default:
            NOTIF_NOTIMPL
            break;
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }
