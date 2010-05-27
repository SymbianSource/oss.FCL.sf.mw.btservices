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

#include "btnotifincomingpairinghandler.h"
#include "btnotifpairingmanager.h"
#include "btnotifoutgoingpairinghandler.h"
#include <btengconstants.h>

const TInt KBTNotifWaitingForPairingOkDelay = 500000; // 0.5s

enum TPairingStageId
    {
    /**
     * is monitoring physical link status
     */
    EPhysicalLinkNotify = 100,
    EWaitingForPairingOk,
    };

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifIncomingPairingHandler::CBTNotifIncomingPairingHandler( CBTNotifPairingManager& aParent, 
    const TBTDevAddr& aAddr) : CBTNotifBasePairingHandler( aParent, aAddr )
    {
    }

// ---------------------------------------------------------------------------
// 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::ConstructL()
    {
    BaseConstructL();
    iActivePairingOk = CBtSimpleActive::NewL(*this, EWaitingForPairingOk );
    User::LeaveIfError( iPairingOkTimer.CreateLocal() );
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTNotifBasePairingHandler* CBTNotifIncomingPairingHandler::NewL( CBTNotifPairingManager& aParent, 
    const TBTDevAddr& aAddr)
    {
    CBTNotifIncomingPairingHandler* self = new (ELeave) CBTNotifIncomingPairingHandler(aParent, aAddr);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifIncomingPairingHandler::~CBTNotifIncomingPairingHandler()
    {
    // TRACE_FUNC_ENTRY
    // Cancel all outstanding requests
    CancelPlaNotification();
    iPla.Close();
    delete iActivePairingOk;
    iPairingOkTimer.Close();
    // TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Accept this message only if the specified device is the same as this is
// dealing with.
// ---------------------------------------------------------------------------
//
TInt CBTNotifIncomingPairingHandler::ObserveIncomingPair( const TBTDevAddr& aAddr )
    {
    TInt err( KErrServerBusy );
    if ( iAddr == aAddr )
        {
        err = KErrNone;
        iUserAwarePairing = ETrue; // This function is called by a notifier, which means the UI has been involved
        // Therefore we can display it in the paired devices list
        if ( !iActive->IsActive() && !OpenPhysicalLinkAdaptor() )
            {
            // If we are observing physical link, or showing user a note,
            // we won't interrupt it.
            UnSetPairResult();
            MonitorPhysicalLink();
            }
        }
    return err;
    }

// ---------------------------------------------------------------------------
// Assign the responsibility of outgoing pair handling to CBTEngOtgPair
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::HandleOutgoingPairL( const TBTDevAddr& aAddr, TUint aCod )
    {
    // TRACE_FUNC_ENTRY
    // Outgoing pairing always takes highest priority:
    CBTNotifBasePairingHandler* pairinghandler = CBTNotifOutgoingPairingHandler::NewL( iParent, aAddr );
    pairinghandler->HandleOutgoingPairL( aAddr, aCod );
    iParent.RenewPairingHandler( pairinghandler );
    // TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Accept this message only if the specified device is the same as this is
// dealing with.
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::StopPairHandling( const TBTDevAddr& aAddr )
    {
    if ( aAddr == iAddr )
        {
        // TRACE_FUNC_ENTRY
        iParent.RenewPairingHandler( NULL );
        // TRACE_FUNC_EXIT
        }
    }

// ---------------------------------------------------------------------------
// Notify user if pairing failed.
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::DoHandlePairServerResult( TInt aResult )
    {
    CancelPlaNotification();
    // For a successful pairing, we need wait for registry table change.
    if( aResult != KErrNone && aResult != KHCIErrorBase )
        {
        // Pair failure situation.
        SetPairResult( aResult );
        // todo: show pairing failure note.
        }
    }

// ---------------------------------------------------------------------------
// Kill this if the linkkey type indicates OBEX authentication.
// Otherwise notify user the pair result.
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::DoHandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev )
    {
    // TRACE_FUNC_ENTRY
    
    // First of all cancel the iPairingOkTimer timer, if active
    if (iActivePairingOk->IsActive())
        {
        iActivePairingOk->Cancel();
        UnSetPairResult();  // we might have set it before (if the link went down) so we want to reset it.   
        }
    if (aDev.LinkKeyType() == ELinkKeyUnauthenticatedNonUpgradable && !iUserAwarePairing)
		{
		// If an application uses btengconnman API to connect a service of 
		// this device and JW pairing occurred as part of security enforcement,
		// it shall be a user aware pairing, and we shall add this device in paired
		// view. In this way, user is able to disconnect the device from our UI.
		// Otherwise the link key has been created by a device without IO requesting 
		// a service connection with phone. We won't take any action (e.g. remove 
		// link key) in this case. As the result, this device can't be seen in our UI, 
		// however other applications are still freely to use its services.
		// TRACE_INFO(_L("[BTEng]: CBTEngIncPair: JW pairing with no IO device" ) )
		TBTEngConnectionStatus status = iParent.ConnectStatus( aDev.Address() );
		if ( status == EBTEngConnecting || status == EBTEngConnected )
			{
			// the return error is ingore as we can not have other proper 
			// exception handling option:
			(void) iParent.AddUiCookieJustWorksPaired( aDev );
			}
		iParent.RenewPairingHandler( NULL );
		}
    else if (aDev.LinkKeyType() == ELinkKeyUnauthenticatedUpgradable && !iUserAwarePairing)
		{
		// The linkkey has been created  by an incoming OBEX service request
		// which resulted a pairing event received from pair server.
		// TRACE_INFO(_L("[BTEng]: CBTEngIncPair: JW pairing with IO device" ) )
		  iParent.RenewPairingHandler( NULL );
		}
    else
		{
		if (aDev.LinkKeyType() == ELinkKeyUnauthenticatedNonUpgradable || aDev.LinkKeyType() == ELinkKeyUnauthenticatedUpgradable)
			{
			// The user was involved in the pairing, so display in the paired devices list
			(void) iParent.AddUiCookieJustWorksPaired(aDev);
			}
		// TRACE_INFO(_L("[BTEng]: CBTEngIncPair: Non-JW pairing"))
		// Other pairing model than Just Works:
		CancelPlaNotification();
		SetPairResult( KErrNone );
		iParent.RenewPairingHandler( NULL );
		}
    // TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTNotifPairingAOObserver.
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus )
    {
    // TRACE_FUNC_ARG( ( _L( "aId: %d, aStatus: %d"), aId, aStatus ) )
        // Check which request completed.
    switch( aActive->RequestId() )
        {
        case EPhysicalLinkNotify:
            {
                // Check if the link has disconnected.
            HandlePhysicalLinkResultL( aStatus );
            break;
            }
        case EWaitingForPairingOk:
            {
            // pairing failed, inform user:
            if (iPairResult == KErrNone)
                {
                // iPairResult must have been set as an error. if it's not it means somewhere else
                // it has been reset. But we need to have it set to an error as we are notifying 
                // the "unable to pair" message.
                SetPairResult(KErrGeneral);
                }
            iParent.RenewPairingHandler( NULL );
            break;
            } 
        default:
                // Should not be possible, but no need for handling.
            break;
        }
    // TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// cancels an outstanding request according to the given id.
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::CancelRequest( TInt aRequestId )
    {
    switch ( aRequestId ) 
        {
        case EPhysicalLinkNotify:
            {
            iPla.CancelNextBasebandChangeEventNotifier();
            }
        case EWaitingForPairingOk:
            {
            iPairingOkTimer.Cancel();
            }
        }
    }

// ---------------------------------------------------------------------------
// From class MBTNotifPairingAOObserver.
// Handles a leave in RequestCompleted by simply self-destructing.
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::HandleError( CBtSimpleActive* aActive, TInt aError )
    {
    // TRACE_FUNC_ARG( ( _L( "request id: %d, error: %d" ), aId, aError ) )
    (void) aActive;
    (void) aError;
        // Our error handling is to just stop observing. 
        // Nothing critical to be preserved here, the user 
        // just won't get any notification of pairing result.
    iParent.RenewPairingHandler( NULL );
    }

// ---------------------------------------------------------------------------
// Subscribe to physical link notifications. 
// physical link must exist when calling this function.
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::MonitorPhysicalLink()
    {
    // TRACE_FUNC_ENTRY
    iActive->SetRequestId( EPhysicalLinkNotify );
        // Subscribe to disconnect and error events.
    iPla.NotifyNextBasebandChangeEvent( iBbEvent, 
                            iActive->RequestStatus(), 
                            ENotifyPhysicalLinkDown | ENotifyPhysicalLinkError );
    iActive->GoActive();
    // TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Opens the adaptor if physical link exists.
// ---------------------------------------------------------------------------
//
TInt CBTNotifIncomingPairingHandler::OpenPhysicalLinkAdaptor()
    {
    // TRACE_FUNC_ENTRY
    TInt err ( KErrNone );
    if( !iPla.IsOpen() )
        {
            // Try to open the adapter in case it failed earlier.
            // This can happen for outgoing dedicated bonding with 
            // non-SSP device, as the PIN dialog can be kept open even 
            // though the link has dropped because of a time-out.
        err = iPla.Open( iParent.SocketServ(), iAddr );
        }
    // TRACE_INFO( (_L("[BTEng]: CBTEngIncPair::HasPhysicalLink ? %d"), iPla.IsOpen() ) )
    return err;
    }

// ---------------------------------------------------------------------------
// Cancel outstanding physical link notification
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::CancelPlaNotification()
    {
    // TRACE_FUNC_ENTRY
    if( iActive && iActive->RequestId() == EPhysicalLinkNotify )
        {
        // cancel Baseband monitor
        iActive->Cancel();
        }
    // TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Handle a physical link event. Notify pair failed if physical link is down.
// ---------------------------------------------------------------------------
//
void CBTNotifIncomingPairingHandler::HandlePhysicalLinkResultL( TInt aResult )
    {
    // TRACE_FUNC_ARG( ( _L( " BBEvent 0x%08X, code %d"), 
    //                        iBbEvent().EventType(), iBbEvent().SymbianErrorCode() ) )
        // Check if the connection is still alive.
    TBool physicalLinkDown = 
        ( iBbEvent().EventType() == ENotifyPhysicalLinkDown | ENotifyPhysicalLinkError );

    if( aResult || physicalLinkDown )
        {
        // link went down. It might be because of pairing failed or the remote device disconnected the
        // physical link after a successful pairing.
        // we wait for 0.5 secs before notifying the "unable to pair" message as, if the pair is 
        // successful, we manage it to show the right confirmation message.
        SetPairResult( (aResult == 0) ? KErrGeneral : aResult );
        iPairingOkTimer.After(iActivePairingOk->iStatus, KBTNotifWaitingForPairingOkDelay);
        iActivePairingOk->GoActive();
        }
    else
        {
        // Uninteresting event, re-subscribe.
        MonitorPhysicalLink();
        }
    // TRACE_FUNC_EXIT
    }



