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

#ifndef BTDELEGATEDEVSECURITY_H
#define BTDELEGATEDEVSECURITY_H

#include <e32base.h>
#include <btengconnman.h>
#include <btengdevman.h>
#include "btabstractdelegate.h"

class BtuiModel;

/*!
    \class BtDelegateDisconnect
    \brief the base class for Disconnecting Bluetooth Connection.
 */
class BtDelegateDevSecurity : public BtAbstractDelegate,
        public MBTEngDevManObserver
{
    Q_OBJECT

public:
    explicit BtDelegateDevSecurity( 
            BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, 
            QObject *parent = 0 );
    
    virtual ~BtDelegateDevSecurity();

    virtual void exec( const QVariant &params );
    
    virtual void cancel();
    
public slots:

protected:
    //From MBTEngDevManObserver
    virtual void HandleDevManComplete( TInt aErr );
    virtual void HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray );
    
    void emitCommandComplete(int error);
    
private:

    CBTEngDevMan *mBtEngDevMan;
    
    QString mdeviceName;
    
    Q_DISABLE_COPY(BtDelegateDevSecurity)

};


#endif /* BTDELEGATEDEVSECURITY_H */
