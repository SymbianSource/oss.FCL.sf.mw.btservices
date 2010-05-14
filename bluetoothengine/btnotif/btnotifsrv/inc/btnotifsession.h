/*
* ============================================================================
*  Name        : btnotifsession.h
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
* Template version: 4.2
*/

#ifndef BTNOTIFSESSION_H
#define BTNOTIFSESSION_H

#include "btnotifserver.h"

/**
 *  CBTNotifSession maintains a session with a client.
 *
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifSession ) : public CSession2
    {

public:

    /**
     * Two-phased constructor.
     */
    static CBTNotifSession* NewL();

    /**
    * Destructor.
    */
    virtual ~CBTNotifSession();

// from base class CSession2

    /**
     * From CSession2.
     * Receives a message from a client.
     *
     * @since Symbian^4
     * @param aMessage The message containing the details of the client request.
     */
    virtual void ServiceL( const RMessage2& aMessage );

    /**
     * From CSession2.
     * Completes construction of the session.
     *
     * @since Symbian^4
     */
    virtual void CreateL();

    /**
     * Complete a client message from a message handle.
     *
     * @since Symbian^4
     * @param aHandle The handle identifying the message.
     * @param aReason The reason code to complete the message with.
     * @param aReply Data to write back to the client.
     * @return KErrNone on success; KErrNotFound if the message is not found.
     */
    TInt CompleteMessage( TInt aHandle, TInt aReason, const TDesC8& aReply );

    /**
     * Find a message identified by an RMessage2 handle.
     *
     * @since Symbian^4
     * @param aHandle The handle identifying the message.
     * @return The message.
     */
    const RMessage2* FindMessageFromHandle( TInt aHandle ) const;

    /**
     * Find a message identified by an RNotifier notifier UID.
     *
     * @since Symbian^4
     * @param aUid The UID identifying the message.
     * @return The message.
     */
    const RMessage2* FindMessageFromUid( TInt aUid ) const;

private:

    CBTNotifSession();

    void ConstructL();

    /**
     * Returns a handle to our server.
     *
     * @since Symbian^4
     * @param Pointer to our server.
     */
    inline CBTNotifServer* Server() const
        { return (CBTNotifServer*) CSession2::Server(); }

    /**
     * Processes a message from a client.
     *
     * @since Symbian^4
     * @param aMessage The message containing the details of the client request.
     */
    void DispatchMessageL( const RMessage2& aMessage );

private: // data

    /**
     * Array of messages currently being processed.
     */
    RArray<RMessage2> iMessageQ;

    BTUNITTESTHOOK

    };

#endif // BTNOTIFSESSION_H
