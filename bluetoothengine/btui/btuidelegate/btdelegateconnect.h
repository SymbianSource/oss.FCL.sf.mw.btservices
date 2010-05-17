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

#ifndef BTDELEGATECONNECT_H_
#define BTDELEGATECONNECT_H_

#include <e32base.h>
#include <btengconnman.h>
#include "btabstractdelegate.h"

class BtuiModel;

/*!
    \class BtDelegateConnect
    \brief the base class for handling Bluetooth Connection.
 */
class BtDelegateConnect : public BtAbstractDelegate,
        public MBTEngConnObserver
{
    Q_OBJECT

public:
    explicit BtDelegateConnect( 
            BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, 
            QObject *parent = 0 );
    
    virtual ~BtDelegateConnect();

    virtual void exec( const QVariant &params );
    
    virtual void cancel();
    
public slots:

protected:
    //From MBTEngConnObserver
    virtual void ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts );
    virtual void DisconnectComplete( TBTDevAddr& aAddr, TInt aErr );
    virtual void PairingComplete( TBTDevAddr& aAddr, TInt aErr );

    void emitCommandComplete(int error);
    
private:

    CBTEngConnMan *mBtengConnMan;

    TBTDevAddr mBtEngddr;
    
    QString mdeviceName;
    
    Q_DISABLE_COPY(BtDelegateConnect)

};


#endif /* BTDELEGATECONNECT_H_ */
