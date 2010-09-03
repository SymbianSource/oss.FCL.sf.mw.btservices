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

#ifndef BTDELEGATEDEVREMOVE_H
#define BTDELEGATEDEVREMOVE_H

#include <e32base.h>
#include <btengdevman.h>
#include "btabstractdelegate.h"

class BtuiModel;

/*!
    \class BtDelegateDevRemove
    \brief This class will remove the specific device from the registry.
 */
class BtDelegateDevRemove : public BtAbstractDelegate,
        public MBTEngDevManObserver 
{
    Q_OBJECT

public:
    explicit BtDelegateDevRemove( 
            BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, 
            QObject *parent = 0 );
    
    virtual ~BtDelegateDevRemove();

    int supportedEditorTypes() const;
    
    virtual void exec( const QVariant &params );
    
    virtual void cancel();

private:
    
public slots:

protected:
    //From MBTEngDevManObserver
    virtual void HandleDevManComplete( TInt aErr );
    virtual void HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray );

    void emitCommandComplete(int error);
    
    int removeBlockedDevice();
    bool isBlockedDeviceExists();
    
private:

    CBTEngDevMan *mBtEngDevMan;
    
    QModelIndexList mBlockedindexList;
    
    bool mOngoingBlockExec;
    
    Q_DISABLE_COPY(BtDelegateDevRemove)

};


#endif /* BTDELEGATEDEVREMOVE_H */
