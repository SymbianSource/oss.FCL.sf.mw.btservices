/*
* ============================================================================
*  Name        : btnotifpairinghelper.h
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
* Template version: 4.2
*/

#ifndef BTNOTIFPAIRINGHELPER_H
#define BTNOTIFPAIRINGHELPER_H

#include <e32base.h>
#include <btmanclient.h>
#include <bluetooth/pairing.h>
#include <btservices/btsimpleactive.h>

#include "bluetoothnotification.h"
#include "bluetoothtrace.h"

class CBTNotifConnection;
class CBTNotifConnectionTracker;
class CBTDevice;


/**
 *  Helper class for performing pairing-related operations.
 *  
 *  This class is constructed only when there is a pairing-related event,
 *  and destructed after completion. Pairing-related events are bonding
 *  (outgoing bonding request from an application), pairing notifiers
 *  (PIN input i.e. legacy pairing, numeric comparison and passkey display),
 *  and Just Works pairing completion.
 *  
 *  The structure of operations is as follows: each event is initiated through
 *  StartXxL() operation, and finished with CompleteXxL(), there is also a
 *  CancelXxL() operation where needed. In addition there are the base class virtual
 *  functions, a few helper functions and HandleAuthenticationCompleteL, which
 *  processes a baseband authentication result. The same structure applies to
 *  suboperations, such as StartTrustedQueryL and CompleteTrustedQueryL.
 *  
 *  For bonding, there is StartBondingL, CompleteBondingL() CancelBondingL();
 *  for pairing notifiers there is StartPairingNotifierL(),
 *  CancelPairingNotifierL(), CompletePairingNotifierL() and also
 *  UpdatePairingNotifierL(). For Just Works processing, there is
 *  StartJustWorksProcessingL(), CancelJustWorksProcessingL() and
 *  CompleteJustWorksProcessingL().
 *  
 *  The design of this class is focussed on structure and maintainability first.
 *  Duplicate (state) information is kept to a minimum. And memory usage comes
 *  before processing. Pairing is an infrequent operation, and this class is
 *  only instantiated when there is pairing-related processing, so extreme
 *  focus on memory or processing efficiency would have relatively little effect.
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifPairingHelper ) : public CBase,
                                             public MBTNotificationResult,
                                             public MBtSimpleActiveObserver
    {

public:

    /**  Enumeration identifying the stage of the pairing operation. */
    enum TPairingOp
        {
        EIdle,
        EAcceptPairing,
        ELegacyPairing,
        ESspPairing,
        EJustWorksPairing,
        EAutoPairing,
        EPostPairingOperations, // Marks the queries and notes which follow
                                // the real pairing input from the user.
        EDedicatedBonding,
        EUnpairing,
        EAwaitingPairingResult,
        EShowPairingSuccess,
        EShowPairingFailure,
        EQueryTrust,
        EBlocking,
        EPairingCancelled
        };

    /**
     * Two-phased constructor.
     * @param aConnection Pointer to the parent.
     * @param aDevice Pointer to information of the remote device.
     * aParam aTracker Pointer to the connection tracker.
     */
    static CBTNotifPairingHelper* NewL( CBTNotifConnection* aConnection, 
                CBTNotifConnectionTracker* aTracker );

    /**
    * Destructor.
    */
    virtual ~CBTNotifPairingHelper();

    /**
     * Get the address of the remote device for this connection.
     *
     * @since Symbian^4
     * @param aProfile The profile identifying the service.
     */
    inline CBTNotifPairingHelper::TPairingOp CurrentOperation() const
        { return iOperation; }

    /**
     * The baseband authentication has completed, handle the result.
     *
     * @since Symbian^4
     * @param aError The result of the authentication procedure.
     */    
    void HandleAuthenticationCompleteL( TInt aError );

    /**
     * Start a bonding operation with the remote device.
     *
     * @since Symbian^4
     * @param aMessage The handle of the message from the client.
     */
    void StartBondingL( TInt aHandle );

    /**
     * Cancel an ongoing bonding operation with the remote device.
     *
     * @since Symbian^4
     */
    void CancelBondingL();

    /**
     * Handle a notifier request for pairing with the remote device
     * of this connection.
     *
     * @since Symbian^4
     * @param aUid The UID of the notifier for this pairing request.
     * @param aParams The parameters for this request from the client.
     */    
    void StartPairingNotifierL( TInt aUid, const TDesC8& aParams );

    /**
     * Update an outstanding request for this connection.
     *  
     * @since Symbian^4
     * @param aUid The UID of the notifier for this update.
     * @param aParams The updated parameters for this request from the client.
     */
    void UpdatePairingNotifierL( TInt aUid, const TDesC8& aParams );

    /**
     * Cancel an outstanding request for this connection.
     *  
     * @since Symbian^4
     * @param aUid The UID of the notifier for this pairing request.
     */
    void CancelPairingNotifierL( TInt aUid );

    /**
     * Start the processing of a completed Just Works pairing.
     * This needs special attention, as this class is not notified
     * other than by observing the registry, and the user is not
     * notified otherwise either (which is the point of this association
     * model). It needs to be verified that the pairing is related to
     * a service access, as dedicated bonding should not be possible
     * (for devices that do not have any IO capabilities).
     *
     * @since Symbian^4
     * @param aMessage The handle of the message from the client.
     */
    void StartJustWorksProcessingL();

    /**
     * Cancel the processing of a completed Just Works pairing.
     *
     * @since Symbian^4
     */
    void CancelJustWorksProcessingL();

// from base class MBTNotificationResult

    /**
     * From MBTNotificationResult.
     * Handle an intermediate result from a user query.
     * This function is called if the user query passes information
     * back before it has finished i.e. is dismissed. The final acceptance/
     * denial of a query is passed back in MBRNotificationClosed.
     *
     * @since Symbian^4
     * @param aData the returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    virtual void MBRDataReceived( CHbSymbianVariantMap& aData );

    /**
     * From MBTNotificationResult.
     * The notification is finished. The resulting data (e.g. user input or
     * acceptance/denial of the query) is passed back here.
     *
     * @since Symbian^4
     * @param aErr KErrNone or one of the system-wide error codes.
     * @param aData the returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    virtual void MBRNotificationClosed( TInt aError, const TDesC8& aData );

// from base class MBtSimpleActiveObserver

    /**
     * From MBtSimpleActiveObserver.
     * Callback to notify that an outstanding request has completed.
     *
     * @since Symbian^4
     * @param aActive The active object helper that completed this request.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus );

    /**
     * From MBtSimpleActiveObserver.
     * Callback for handling cancelation of an outstanding request.
     *
     * @since Symbian^4
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void CancelRequest( TInt aRequestId );

    /**
     * Callback to notify that an error has occurred in RunL.
     *
     * @param aActive Pointer to the active object that completed.
     * @param aError The error occurred in RunL.
     */
    virtual void HandleError( CBtSimpleActive* aActive, TInt aError );
    
private:

    /**
     * C++ default constructor.
     */
    CBTNotifPairingHelper( CBTNotifConnection* aConnection,
                CBTNotifConnectionTracker* aTracker );

    /**
     * Symbian 2nd-phase constructor.
     */
    void ConstructL();

    /**
     * Process the user input and complete the outstanding pairing request.
     *
     * @since Symbian^4
     * @param aError The result off the notification.
     * @param aResult The user response; ETrue if the user accepted the query,
     *                otherwise EFalse.
     * @param aData The data returned from the notification dialog.
     */
    void CompletePairingNotifierL( TInt aError, TBool aResult, const TDesC8& aData );

    /**
     * Completes a bonding operation.
     *
     * @since Symbian^4
     * @param aError The result of the bonding attempt.
     */
    void CompleteBondingL( TInt aError );

    /**
     * Completes a bonding operation.
     *
     * @since Symbian^4
     * @param aError The result of the bonding attempt.
     */
    void CompleteJustWorksProcessingL( TInt aError );

    /**
     * Ask the user to allow incoming pairing.
     *
     * @since Symbian^4
     */
    void StartAcceptPairingQueryL();

    /**
     * Process the user input and for accepting an incoming pairing and
     * continue with the outstanding pairing request.
     *
     * @since Symbian^4
     * @param aError The result of the notification.
     * @param aResult The user response; ETrue if the user accepted the query,
     *                otherwise EFalse.
     */
    void CompleteAcceptPairingQueryL( TInt aError, TBool aResult );

    /**
     * Ask the user to set the device as trusted.
     *
     * @since Symbian^4
     * @param aData The data returned from the notification dialog.
     */
    void StartTrustedQueryL();

    /**
     * Process the user input for setting the device as trusted.
     *
     * @since Symbian^4
     * @param aError The result of the notification.
     * @param aResult The user response; ETrue if the user accepted the query,
     *                otherwise EFalse.
     */
    void CompleteTrustedQueryL( TInt aError, TBool aResult );

    /**
     * Parse the parameters of a request for pairing.
     * This function also returns values to use for dialog config, and sets
     * the operation state member variable (iOperation).
     *
     * @since Symbian^4
     * @param aLocallyInitiated On return, will be set to ETrue if the pairing 
     *                          was initiated by us.
     * @param aNumVal On return, this descriptor will contain a number to use in
     *                the pairing dialog. The meaning depends on the type of pairing.
     * @param aDialogType On return, will contain the dialog type.
     * @param aResourceId On return, will contain the resource id.
     */
    void ParseNotifierReqParamsL( TBool& aLocallyInitiated, TDes& aNumVal,
                TBluetoothDialogParams::TBTDialogType& aDialogType,
                TBTDialogResourceId& aResourceId );

    /**
     * Parse the parameters of a request for pairing using pin query.
     *
     * @since Symbian^4
     * @param aLocallyInitiated On return, will be set to ETrue if the pairing 
     *                          was initiated by us.
     * @param aNumVal On return, this will contain the minimum passcode length.
     */
    void ParsePinCodeReqParamsL( TBool& aLocallyInitiated, TUint& aNumVal );

    /**
     * Parse the parameters of a request for pairing using numeric comparison.
     *
     * @since Symbian^4
     * @param aLocallyInitiated On return, will be set to ETrue if the pairing 
     *                          was initiated by us.
     * @param aNumVal On return, this descriptor will contain the passkey to 
     *                show to the user.
     */
    void ParseNumericCompReqParamsL( TBool& aLocallyInitiated, TDes& aNumVal );

    /**
     * Parse the parameters of a request for pairing using passkey display.
     *
     * @since Symbian^4
     * @param aLocallyInitiated On return, will be set to ETrue if the pairing 
     *                          was initiated by us.
     * @param aNumVal On return, this descriptor will contain the passkey to 
     *                show to the user.
     */
    void ParsePasskeyDisplayReqParamsL( TBool& aLocallyInitiated, TDes& aNumVal );

    /**
     * Check if we can guess the PIN and complete the notifier without 
     * user interaction.
     *
     * @since Symbian^4
     * @param aLocallyInitiated ETrue if we initiated the pairing, otherwise EFalse.
     * @param aNumVal The minimum lenght of the passcode.
     */
    void CheckAutoPairingL( TBool aLocallyInitiated, const TDesC& aNumVal );

    /**
     * Get a notification and configure it according to the current operation.
     *
     * @since Symbian^4
     * @param aType The notification type.
     * @param aResourceId Identifier for the resource to display.
     */
    void PrepareNotificationL( TBluetoothDialogParams::TBTDialogType aType,
                TBTDialogResourceId aResourceId );

    /**
     * Handle the result from a notification that is finished.
     *
     * @since Symbian^4
     * @param aErr KErrNone or one of the system-wide error codes.
     * @param aData The returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    void NotificationClosedL( TInt aError, const TDesC8& aData );

private: // data

    /**
     * Identifier for the current operation.
     */
    TPairingOp iOperation;

    /**
     * UID of the notifier pairing dialog request.
     */
    TInt iNotifierUid;

    /**
     * Handle to the client message for dedicated bonding. Also serves as flag 
     * indicating that we are performing dedicated bonding.
     */
    TInt iDedicatedBonding;

    /**
     * Subsession with pairing server for dedicated bonding.
     */
    RBluetoothDedicatedBondingInitiator iBondingSession;

    /**
     * Handle for general bonding.
     */
    RBTPhysicalLinkAdapter iBondingSocket;

    /**
     * Buffer containing the parameters of the client message.
     * Own.
     */
    HBufC8* iParams;

    /**
     * Active object helper for outgoing bonding.
     * Own.
     */
    CBtSimpleActive* iBondingActive;

    /**
     * Pointer to the data record of the remote device.
     * Not own.
     */
    CBTDevice* iDevice;

    /**
     * Pointer to an outstanding user interaction.
     * Not own.
     */
    CBluetoothNotification* iNotification;

    /**
     * Pointer to the class that we are helping.
     * Not own.
     */
    CBTNotifConnection* iConnection;

    /**
     * Pointer to our grandparent.
     * Not own.
     */
    CBTNotifConnectionTracker* iTracker;

    BTUNITTESTHOOK

    };

#endif // BTNOTIFPAIRINGHELPER_H
