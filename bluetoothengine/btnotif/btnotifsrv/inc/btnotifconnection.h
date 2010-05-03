/*
* ============================================================================
*  Name        : btnotifconnection.h
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
* Template version: 4.2
*/

#ifndef BTNOTIFCONNECTION_H
#define BTNOTIFCONNECTION_H

#include <bt_sock.h>
#include <btservices/btsimpleactive.h>
#include <btengdevman.h>
#include <btengconstants.h>

#include "bluetoothnotification.h"

#include "bluetoothtrace.h"

class CBTNotifConnectionTracker;
class CBTNotifPairingHelper;

/**
 * Utility function for getting the name of a device to display.
 *
 * @since Symbian^4
 * @param aName On return, will hold the device name to display.
 * @param aDevice Data dtructure holding the device record.
 */
void GetDeviceNameL( TBTDeviceName& aName, const CBTDevice& aDevice );


/**
 *  CBTNotifConnection handles the connection information and operations 
 *  related to remote devices.
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifConnection ) : public CBase,
                                          public MBTNotificationResult,
                                          public MBtSimpleActiveObserver,
                                          public MBTEngDevManObserver
    {

public:

    /**  Enumeration for the current active operation. */
    enum TOperation
        {
        EIdle,
        EAuthorizing,
        EPairing,
        EBonding,
        EAdditionalNotes,   // Marks the queries and notes which follow
                            // notifier requests, but are done after completing
                            // the actual notifier message
        EBlocking,
        EInternalOperations,    // Marks internal operations such as registry update.
        EReadingRegistry,
        EUpdatingRegistry
        };

    /**  Array of BT profiles. */
    typedef RArray<TBTProfile> RBTProfileArray;

    /**
     * Two-phased constructor.
     * @param aAddr Address of the remote device for this connection.
     * @param aTracker Pointer to our parent
     */
    static CBTNotifConnection* NewLC( const TBTDevAddr& aAddr, 
                CBTNotifConnectionTracker* aTracker );

    /**
    * Destructor.
    */
    virtual ~CBTNotifConnection();

    /**
     * Get the address of the remote device for this connection.
     *
     * @since Symbian^4
     * @return The BD_ADDR.
     */
    inline const TBTDevAddr& Address() const
        { return iAddr; }

    /**
     * Get the device record of the remote device for this connection.
     *
     * @since Symbian^4
     * @return The CBTDevice device record.
     */
    inline CBTDevice* BTDevice() const
        { return iDevice; }

    /**
     * Get the current operation for this connection.
     *
     * @since Symbian^4
     * @param aProfile The profile identifying the service.
     */
    inline CBTNotifConnection::TOperation CurrentOperation() const
        { return iCurrentOp; }

    /**
     * Checks if we have any outstanding request, and handle the next
     * in line. Also checks the link state, and informs the tracker
     * if we have finished processing and the link is down.
     *  
     * @since Symbian^4
     */
    void CheckNextOperationL();

    /**
     * Completes the first outstanding client request and removes 
     * it from the queue.
     *  
     * @since Symbian^4
     * @param aReason The reason code to complete the message with.
     * @param aReply Data to write back to the client.
     */
    void CompleteClientRequest( TInt aReason, const TDesC8& aReply );

    /**
     * Distinguish the type request of this connection and queue it
     * or handle it.
     *  
     * @since Symbian^4
     * @param aParams The parameters for this request from the client.
     * @param aMessage The message from the client.
     */
    void HandleNotifierRequestL( const TDesC8& aParams, const RMessage2& aMessage );

    /**
     * Update an outstanding request for this connection.
     *  
     * @since Symbian^4
     * @param aParams The parameters of the original request from the client.
     * @param aMessage The update message from the client.
     */
    void HandleNotifierUpdateL( const TDesC8& aParams, const RMessage2& aMessage );

    /**
     * Cancel an outstanding request for this connection.
     *  
     * @since Symbian^4
     * @param aMessage The message from the client. (Temp! find better way!)
     */
    void CancelNotifierRequestL( const RMessage2& aMessage );

    /**
     * Start a bonding operation with the remote device.
     *
     * @since Symbian^4
     * @param aMessage The message from the client.
     */
    void StartBondingL( const RMessage2& aMessage );

    /**
     * Cancel an ongoing bonding operation with the remote device.
     *
     * @since Symbian^4
     */
    void CancelBondingL();

    /**
     * The pairing handler has completed a pairing operation. If this was part 
     * of a bonding procedure then this will complete the client request.
     *
     * @since Symbian^4
     */
    void PairingCompleted();

    /**
     * Process a new pairing result, and determine if we need to show
     * anything to the user.
     *
     * @since Symbian^4
     * @param aError Result of the pairing operation.
     */
    void PairingResult( TInt aError );

    /**
     * A service-level connection has been made with the device 
     * observed by this instance. The appropriate notification 
     * will be shown to the user.
     *
     * @since Symbian^4
     * @param aProfile The profile identifying the service.
     */
    void ServiceConnectedL( TBTProfile aProfile );

    /**
     * A service-level connection has disconnected from the device 
     * observed by this instance. The appropriate notification 
     * will be shown to the user.
     *
     * @since Symbian^4
     * @param aProfile The profile identifying the service.
     */
    void ServiceDisconnectedL( TBTProfile aProfile );

    /**
     * Ask the user if he/she wants to block future connection requests.
     *
     * @since Symbian^4
     */
    void LaunchBlockingQueryL();

    /**
     * Modify the record for the remote device in BTRegistry, with the 
     * changes already made in the local record.
     *
     * @since Symbian^4
     */
    void UpdateRegistryEntryL();
    
    /**
     * Modify the record for the remote device in BTRegistry, if 
     * aTrusted == true, then update trusted status after reading device 
     * info from registry
     *
     * @since Symbian^4
     */
     void UpdateRegistryEntryL(TBool aTrusted);
    
// from base class MBTNotificationResult

    /**
     * From MBTNotificationResult.
     * Handle an intermediate result from a user query.
     * This ffunction is called if the user query passes information
     * back before it has finished i.e. is dismissed. The final acceptance/
     * denial of a query is passed back in MBRNotificationClosed.
     *
     * @since Symbian^4
     * @param aData the returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    virtual void MBRDataReceived( CHbSymbianVariantMap & aData );

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
     * @param sError The error occured in RunL
     */
    virtual void HandleError( CBtSimpleActive* aActive, 
                                        TInt aError );
    
// from base class MBTEngDevmanObserver

    /**
     * From MBTEngDevManObserver.
     * Indicates to the caller that adding, deleting or modifying a device 
     * has completed.
     * When this function is called, new commands can be issued to the 
     * CBTEngDevMan API immediately.
     *
     * @since  S60 v3.2
     * @param  aErr Status information, if there is an error.
     */
    virtual void HandleDevManComplete( TInt aErr );
    
    /**
     * From MBTEngDevManObserver.
     * Indicates to the caller that getting a device from registry
     * has completed.
     *
     * @param aErr Status information, if there is an error.
     * @param aDeviceArray Array of devices that match the given criteria 
     *                     (the array provided by the caller).
     */
    virtual void HandleGetDevicesComplete( 
            TInt err, CBTDeviceArray* deviceArray );

private:

    /**
     * C++ default constructor.
     */
    CBTNotifConnection( const TBTDevAddr& aAddr,
                CBTNotifConnectionTracker* aTracker );

    /**
     * Symbian 2nd-phase constructor.
     */
    void ConstructL();

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

    /**
     * Handle a request for authorization of this connection.
     *
     * @since Symbian^4
     * @param aParams The parameters for this request from the client.
     */
    void HandleAuthorizationReqL( const TDesC8& aParams );

    /**
     * Process the user input and complete the outstanding authorization request.
     *
     * @since Symbian^4
     * @param aError The result off the notification.
     * @param aData The data returned from the notification dialog.
     */
    void CompleteAuthorizationReqL( TInt aError, const TDesC8& aData );

    /**
     * Process the user input for blocking a device.
     *
     * @since Symbian^4
     * @param aError The result off the notification.
     * @param aData The data returned from the notification dialog.
     */
    void CompleteBlockingReqL( TInt aError, const TDesC8& aData );
    
    /**
     * Fetch device from registry
     *
     * @since Symbian^4
     * @param addr BT address of device to fetch from registry
     */
    void GetDeviceFromRegistry( const TBTDevAddr &addr );

private: // data

    /**
     * The current operation we are carrying out.
     */
    TOperation iCurrentOp;

    /**
     * Address of the remote device, identifying this connection.
     */
    TBTDevAddr iAddr;

    /**
     * Package to receive updates of the physical link state.
     */
    TBTBasebandEvent iBasebandEvent;

    /**
     * Queue of handles (identifier) of client messages we have been requested to work on.
     */
    RArray<TInt> iMsgHandleQ;

    /**
     * Array of accepted profile connections (as known here).
     */
    RBTProfileArray iAcceptedConnections;

    /**
     * Array of rejected profile connections (as known here).
     */
    RBTProfileArray iDeniedConnections;

    /**
     * Handle to observe and control the baseband connection.
     */
    RBTPhysicalLinkAdapter iPhyLink;

    /**
     * Subsession with BT registry.
     */
    RBTRegistry iRegistry;

    /**
     * Details of the remote device.
     * Own.
     */
    CBTDevice* iDevice;

    /**
     * Details of the remote device as retrieved from BT registry.
     * Own.
     */
    CBTRegistryResponse* iRegistryResponse;

    /**
     * helper for modifying registry.
     * Own.
     */
    CBTEngDevMan* iDevMan;

    /**
     * Active object helper for observing physical link changes.
     * Own.
     */
    CBtSimpleActive* iPhyActive;

    /**
     * Active object helper for observing BT registry changes.
     * Own.
     */
    CBtSimpleActive* iRegActive;

    /**
     * Helper class for processing pairing-related operations.
     * Own.
     */
    CBTNotifPairingHelper* iPairingHelper;

    /**
     * Pointer to an outstanding user interaction.
     * Not own.
     */
    CBluetoothNotification* iNotification;

    /**
     * Pointer to our parent.
     * Not own.
     */
    CBTNotifConnectionTracker* iTracker;
    
    CBTDeviceArray* iRegDevArray;  // used for retrieving devices from registry

    BTUNITTESTHOOK

    };

#endif // BTNOTIFCONNECTION_H
