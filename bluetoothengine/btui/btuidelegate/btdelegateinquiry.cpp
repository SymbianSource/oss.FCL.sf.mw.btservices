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
#include "btdelegateinquiry.h"
#include "btdelegatepower.h"
#include "btqtconstants.h"
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <bluetoothuitrace.h>

BtDelegateInquiry::BtDelegateInquiry(
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject* parent )
    :BtAbstractDelegate( settingModel, deviceModel, parent )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
}

BtDelegateInquiry::~BtDelegateInquiry()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (isExecuting()) {
        deviceModel()->cancelSearchDevice();
    }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
}

/*!
    Returns the supported editor types.
    \return the sum of supported editor types
 */
int BtDelegateInquiry::supportedEditorTypes() const
{
    return BtDelegate::Inquiry;
}

void BtDelegateInquiry::exec( const QVariant& params )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    Q_UNUSED(params);
    BTUI_ASSERT_X(!isExecuting(), "BtDelegateInquiry::exec", "operation ongoing!");
    bool ok(false);
    
    setExecuting(true);
    // Inquiry needs BT to be on.
    if(!isBtPowerOn()) {
        mPowerDelegate = new BtDelegatePower(settingModel(), deviceModel(), this);
        ok = connect(
                mPowerDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), 
            this, SLOT(handleManagePowerCompleted(int)));
        if (ok ) {
            mPowerDelegate->exec(QVariant(BtPowerOn));
        }
    } else {
        // No power management is required. Start inquiry immediately
        ok = startInquiry();
    }
    if (!ok) {
        completeDelegateExecution(KErrGeneral);
    }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
}

void BtDelegateInquiry::cancel()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (isExecuting()) {
        deviceModel()->cancelSearchDevice();
        setExecuting(false);
    }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
}

void BtDelegateInquiry::handleManagePowerCompleted(int error)
{
    if (isExecuting()) {
        delete mPowerDelegate;
        bool ok = (error == KErrNone);
        if (ok) {
            ok = startInquiry();
        }
        if (!ok) {
            completeDelegateExecution(KErrGeneral);
        }
    }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
}

void BtDelegateInquiry::handleSearchCompleted(int error)
{
    completeDelegateExecution(error? KErrGeneral : KErrNone);
    BOstraceFunctionExit0( DUMMY_DEVLIST );
}

bool BtDelegateInquiry::startInquiry()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    // Make sure this connects the signal only once:
    disconnect(deviceModel(), 0, this, 0);
    bool ok = connect(deviceModel(), SIGNAL(deviceSearchCompleted(int)),
            this, SLOT(handleSearchCompleted(int)));
    if (ok ) {
        ok = deviceModel()->searchDevice();
    }
    if (ok) {
        // Inform the client that inquiry really starts now. With
        // this signal, the client can thus prepare for showing inquiry results:
        emit beginUiEditMode();
    }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    return ok;
}

