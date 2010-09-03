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

#ifndef BTDELEGATEPOWER_H
#define BTDELEGATEPOWER_H

#include <e32base.h>
#include <btengsettings.h>
#include <btengconnman.h>
#include "btabstractdelegate.h"

class BtuiModel;
class HbAction;

/*!
    \class BtDelegatePower
    \brief the base class for handling Bluetooth power management.

    \\sa btuidelegate
 */
class BtDelegatePower : public BtAbstractDelegate, public MBTEngSettingsObserver
{
    Q_OBJECT

public:
    explicit BtDelegatePower(
            BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, QObject *parent = 0 );
    
    virtual ~BtDelegatePower();
    
    int supportedEditorTypes() const;
    
    virtual void exec( const QVariant &params );
    
    //from MBTEngSettingsObserver
    
    virtual void PowerStateChanged( TBTPowerStateValue state );

    virtual void VisibilityModeChanged( TBTVisibilityMode state );
    
public slots:
    void btOnQuestionClose(int action);
    
    void btOnWarningClose();
    
    void disconnectCompleted(int err, BtAbstractDelegate *delegate);
    
private:
    void switchBTOn();
    
    void switchBTOff();
    
    bool checkOfflineMode(bool& btEnabledInOffline);
    
    void disconnectConnections();
    
private:
    CBTEngSettings* mBtengSettings;
    TBTPowerStateValue mReqPowerState; 
    BtAbstractDelegate* mDisconnectDelegate;

private:

    Q_DISABLE_COPY(BtDelegatePower)

};

#endif // BTDELEGATEPOWER_H
