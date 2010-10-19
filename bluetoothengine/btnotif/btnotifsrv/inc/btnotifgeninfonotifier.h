/*
* ============================================================================
*  Name        : btnotifgeninfonotifier.h
*  Part of     : BTProximity / BTProximity
*  Description : Class for the generic info notifier.
*
*  Copyright © 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef BTNOTIFGENINFONOTIFIER_H
#define BTNOTIFGENINFONOTIFIER_H


#include "bluetoothnotification.h"
#include "btnotifserver.h"
#include <btservices/btdevextension.h>
#include <hb/hbcore/hbsymbianvariant.h>

/**
 *  CBTNotifGenInfoNotifier for generic info notifier
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifGenInfoNotifier ) : 
        public CBase,
        public MBTNotificationResult
    {

public:

    /**
     * Two-phased constructor.
     * @param aServer Pointer to our parent
     */
    static CBTNotifGenInfoNotifier* NewL( CBTNotifServer* aServer );

    /**
    * Destructor.
    */
    virtual ~CBTNotifGenInfoNotifier();

    /**
     * Processes a message from a notifier client related to settings.
     *
     * @since Symbian^4
     * @param aMessage The message containing the details of the client request.
     */
    void HandleNotifierRequestL( const RMessage2& aMessage );

private:
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
    
    // From MDevDiscoveryObserver
    
private:

    CBTNotifGenInfoNotifier( CBTNotifServer* aServer );

    void ConstructL();
    
    void ShowNotificationL(const RMessage2& aMessage);
			
private: // data    

    /**
     * Reference to our parent the server class.
     * Not own.
     */
    CBTNotifServer* iServer;
    
    /**
     * Pointer to an outstanding user interaction.
     * Not own.
     */
    CBluetoothNotification* iNotification;

    /**
     * Pointer to the entering SAP notification.
     * 
     */    
    CBluetoothNotification* iEnterSAPNotification;
  
    };

#endif // BTNOTIFDEVICESELECTOR_H
