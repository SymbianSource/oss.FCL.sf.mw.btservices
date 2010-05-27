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

    virtual void exec( const QVariant &params );
    
    virtual void PowerStateChanged( TBTPowerStateValue aState );

    virtual void VisibilityModeChanged( TBTVisibilityMode aState );
    
public slots:
    void btOnQuestionClose(HbAction *action);
    
    void btOnWarningClose();
    
    void btOffDialogClose(HbAction *action);
    
private:
    void switchBTOn();
    
    void switchBTOff();
    
    bool checkOfflineMode(TBTEnabledInOfflineMode& aEnabledInOffline);
    
public slots:

private:
    CBTEngSettings* mBtengSettings;

private:

    Q_DISABLE_COPY(BtDelegatePower)

};

#endif // BTDELEGATEPOWER_H
