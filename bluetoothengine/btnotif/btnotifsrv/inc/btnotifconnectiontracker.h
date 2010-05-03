/*
* ============================================================================
*  Name        : btnotifconnectiontracker.h
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
* Template version: 4.2
*/

#ifndef BTNOTIFCONNECTIONTRACKER_H
#define BTNOTIFCONNECTIONTRACKER_H


#include <e32property.h>
#include <btmanclient.h>
#include <bluetooth/pairing.h>
#include <btengconnman.h>
#include <btservices/btsimpleactive.h>
#include "btnotifserver.h"

class CBTNotifConnection;
class CBTNotificationManager;
class CbtnotifConnectionTrackerTest;



/**
 *  CBTNotifConnectionTracker keeps track of remote device connections
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifConnectionTracker ) : public CBase,
                                                 public MBluetoothPhysicalLinksNotifier,
                                                 public MBTEngConnObserver,
                                                 public MBtSimpleActiveObserver
    {

public:

    /**
     * Two-phased constructor.
     * @param aServer Pointer to our parent
     */
    static CBTNotifConnectionTracker* NewL( CBTNotifServer* aServer );

    /**
     * Destructor.
     */
    virtual ~CBTNotifConnectionTracker();

    /**
     * Get a pointer to the btnotif server object.
     *
     * @since Symbian^4
     * @return The server.
     */
    inline CBTNotifServer* Server() const
        { return iServer; }

    /**
     * Get a pointer to the notification manager.
     * This handle can be used for queueing notifications.
     *
     * @since Symbian^4
     * @return The notification manager.
     */
    inline CBTNotificationManager* NotificationManager() const
        { return iServer->NotificationManager(); }

    /**
     * Get the shared handle to BT registry server.
     * This handle can be used for creating subsessions.
     *
     * @since Symbian^4
     * @return Session with BT registry server.
     */
    inline RBTRegServ& RegistryServerSession()
        { return iBTRegistrySession; }

    /**
     * Get the handle to the socket server.
     * This handle can be used for creating subsessions.
     *
     * @since Symbian^4
     * @return Session with the socket server.
     */
    inline RSocketServ& SocketServerSession()
        { return iSockServ; }

    /**
     * Get the handle to the Bluetooth pairing server.
     * This handle can be used for creating subsessions.
     *
     * @since Symbian^4
     * @return Session with the socket server or NULL.
     */
    inline RBluetoothPairingServer* PairingServerSession()
        { return iPairingServ; }

    /**
     * Processes a message from a notifier client related to connections.
     *
     * @since Symbian^4
     * @param aMessage The message containing the details of the client request.
     */
    void DispatchNotifierMessageL( const RMessage2& aMessage );

    /**
     * Handle a request related to pairing.
     *
     * @since Symbian^4
     * @param aUid The UID of the notification request.
     */
    void HandleBondingRequestL( const RMessage2& aMessage );

    /**
     * Handle a change in the number of physical connections.
     * This also handles the case where a connection monitor has finished
     * its processing, and is now ready for removal.
     *
     * @since Symbian^4
     */
    void HandleLinkCountChangeL();

    /**
     * Check repeated connection attempts, and record rejected/accepted queries.
     *
     * @since Symbian^4
     * @param aDevice The details of the remote device for this query.
     * @param aAccepted ETrue if the user accepted the request, EFalse if rejected.
     * @return ETrue if the user should be queried for blocking this device,
     *         EFalse if no query should be launched by the caller.
     */
    TBool UpdateBlockingHistoryL( const CBTDevice* aDevice, TBool aAccepted );

// from base class MBluetoothPhysicalLinksNotifier
    /** Notification of a requested connection coming up.
     * If no error is reported, then that connection is ready for use.
     * 
     * @since Symbian^4
     * @param aErr the returned error
    */
    virtual void HandleCreateConnectionCompleteL( TInt aErr );

    /** Notification of a requested disconnection having taken place.
     * If no error is reported, then that connection has been closed.
     * 
     * @since Symbian^4
     * @param aErr the returned error
    */
    virtual void HandleDisconnectCompleteL( TInt aErr );

    /** Notification that all existing connections have been torn down.
     * If no error is reported, then there are no Bluetooth connections existing.
     * 
     * @since Symbian^4
     * @param aErr the returned error
    */
    virtual void HandleDisconnectAllCompleteL( TInt aErr );

// from base class MBTEngConnObserver

    /**
     * From MBTEngConnObserver.
     * Indicates to the caller that a service-level connection has completed.
     * This function is called for both incoming and outgoing connections. 
     * This function is also called when an outgoing connection request fails, 
     * e.g. with error code KErrCouldNotConnect.
     * When this function is called, new commands can be issued to the 
     * CBTEngConnMan API immediately.
     *
     * @since  S60 v3.2
     * @param  aAddr The address of the remote device.
     * @param  aErr Status information of the connection. KErrNone if the
     *              connection succeeded, otherwise the error code with 
     *              which the outgoing connection failed. KErrAlreadyExists 
     *              is returned if there already is an existing connection 
     *              for the selected profile(s), or otherwise e.g. 
     *              KErrCouldNotConnect or KErrDisconnected for indicating 
     *              connection problems.
     * @param  aConflicts If there already is a connection for the selected 
     *                    profile(s) of an outgoing connection request (the 
     *                    selection is performed by BTEng), then this array 
     *                    contains the bluetooth device addresses of the 
     *                    remote devices for those connections.
     */
    virtual void ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts = NULL );

    /**
     * From MBTEngConnObserver.
     * Indicates to the caller that a service-level connection has disconnected.
     * When this function is called, new commands can be issued to the 
     * CBTEngConnMan API immediately.
     *
     * @since  S60 v3.2
     * @param  aAddr The address of the remote device.
     * @param  aErr The error code with which the disconnection occured. 
     *              KErrNone for a normal disconnection, 
     *              or e.g. KErrDisconnected if the connection was lost.
     */
    virtual void DisconnectComplete( TBTDevAddr& aAddr, TInt aErr );

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
    
private:

    CBTNotifConnectionTracker( CBTNotifServer* aServer );

    void ConstructL();

    /**
     * Parse a client message and find the correct connection handler
     * for  dispatching a request.
     * The parsed client message may be a request that is being served,
     * in case a cancel request has come in. The opcode identifies the
     * client message that is currently being dispatched.
     *
     * @since Symbian^4
     * @param aOpcode The opcode of the original request.
     * @param aMessage The client message to parse.
     * @param aBuffer On return this contains the parameters read from the message.
     * @return The connection that is identified by the address in the
     *         message, or NULL if none found.
     */
    CBTNotifConnection* FindConnectionFromMessageL( TInt aOpcode, 
                const RMessage2& aMessage, TDes8& aBuffer );

    /**
     * Parse the Bluetooth address from a request for some notification.
     *
     * @since Symbian^4
     * @param aUid The UID of the notification request.
     * @param aParamsBuf Descriptor containing the parameter read from the
     *        client message.
     * @return Bluetooth address parsed from the descriptor.
     */
    TBTDevAddr ParseAddressL( TInt aUid, const TDesC8& aParamsBuf ) const;

    /**
     * Find the handler of a specific connection.
     *
     * @since Symbian^4
     * @param aAddr The remote device address identifying the connection.
     * @return Connnection handler or NULL;
     */
    CBTNotifConnection* FindConnectionHandler( const TBTDevAddr& aAddr ) const;

    /**
     * Record and check the time between connection attempts.
     *
     * @since Symbian^4
     * @param aAccepted ETrue if the user accepted the request, EFalse if rejected.
     * @return EFalse if the attempt followed the previous attempt too fast,
     *         otherwise ETrue.
     */
    TBool RecordConnectionAttempts( TBool aAccepted );

private: // data

    /**
     * PubSub key for tracking the number of connections.
     */
    RProperty iLinkCount;

// ToDo: remove this when registry notifications API is available!!
    /**
     * PubSub key for tracking registry changes.
     */
    RProperty iRegistryChange;

    /**
     * Helper active object for observing the link count.
     * Own.
     */
    CBtSimpleActive* iRegistryActive;
// End ToDo

    /**
     * Array of pointers to observers of individual connections.
     */
    RPointerArray<CBTNotifConnection> iConnArray;
//    RHashMap<TBTDevAddr,CBTNotifConnection*> iConnArray;

    /**
     * Time of the last denied connection attempt.
     */
    TInt64 iLastReject;

    /**
     * Array of device addresses that the user has denied access.
     */
    RArray<TBTDevAddr> iDeniedRequests;

    /**
     * Single session with BTRegistry, to be used for subsessions.
     */
    RBTRegServ iBTRegistrySession;

    /**
     * Single session with the socket server, to be used for subsessions.
     */
    RSocketServ iSockServ;

    /**
     * Address placeholder for receiving secure simple pairing results.
     */
    TBTDevAddr iSspResultAddr;

    /**
     * Session with pairing server for receiving secure simple pairing results.
     */
    RBluetoothPairingResult iSspResultSession;

    /**
     * Single session with the pairing server, to be used for subsessions.
     * Own.
     */
    RBluetoothPairingServer* iPairingServ;

    /**
     * Object for managing the piconet.
     * Own.
     */
    CBTEngConnMan* iConnMan;

    /**
     * Object for managing the piconet.
     * Own.
     */
    CBluetoothPhysicalLinks* iPhyLinks;

    /**
     * Helper active object for observing the link count.
     * Own.
     */
    CBtSimpleActive* iLinkCountActive;

    /**
     * Helper active object for receiving secure simple pairing results.
     * Own.
     */
    CBtSimpleActive* iSspResultActive;

    /**
     * Reference to our parent the server class.
     * Not own.
     */
    CBTNotifServer* iServer;

    BTUNITTESTHOOK

    };

#endif // BTNOTIFCONNECTIONTRACKER_H
