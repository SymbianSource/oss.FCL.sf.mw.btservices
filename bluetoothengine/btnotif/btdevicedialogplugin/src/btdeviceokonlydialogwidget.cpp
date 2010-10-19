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
* Description:  BtDeviceOkOnlyDialogWidget class implementation.
*
*/


#include "btdeviceokonlydialogwidget.h"
#include "btdevicedialogplugintrace.h"
#include <bluetoothdevicedialogs.h>
#include <hbaction.h>
#include <hbdialog.h>
#include <hblabel.h>
#include "btdevicedialogpluginerrors.h"

/*!
    class Constructor
 */
BtDeviceOkOnlyDialogWidget::BtDeviceOkOnlyDialogWidget(
        HbMessageBox::MessageBoxType type, const QVariantMap &parameters)
{
    TRACE_ENTRY
    // set properties
    mLastError = NoError;
    mShowEventReceived = false;
    mMessageBox = new HbMessageBox(type);
    resetProperties();
    constructQueryDialog(parameters);
    TRACE_EXIT
}

/*!
    Set parameters, implementation of interface
    Invoked when HbDeviceDialog::update calls.
 */
bool BtDeviceOkOnlyDialogWidget::setDeviceDialogParameters(
    const QVariantMap &parameters)
{
    TRACE_ENTRY
    mLastError = NoError;
    processParam(parameters);
    TRACE_EXIT
    return true;
}

/*!
    Get error, implementation of interface
 */
int BtDeviceOkOnlyDialogWidget::deviceDialogError() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mLastError;
}

/*!
    Close notification, implementation of interface
 */ 
void BtDeviceOkOnlyDialogWidget::closeDeviceDialog(bool byClient)
{
    TRACE_ENTRY
    Q_UNUSED(byClient);
    // Closed by client or internally by server -> no action to be transmitted.
    mMessageBox->close();
    // If show event has been received, close is signalled from hide event. If not,
    // hide event does not come and close is signalled from here.
    if (!mShowEventReceived) {
        emit deviceDialogClosed();
    }
    TRACE_EXIT
}

/*!
    Return display widget, implementation of interface
 */
HbDialog *BtDeviceOkOnlyDialogWidget::deviceDialogWidget() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mMessageBox;
}

QObject *BtDeviceOkOnlyDialogWidget::signalSender() const
{
    return const_cast<BtDeviceOkOnlyDialogWidget*>(this);
}        

/*!
    Construct display widget
 */
bool BtDeviceOkOnlyDialogWidget::constructQueryDialog(const QVariantMap &parameters)
{
    TRACE_ENTRY
    // analyze the parameters to compose the properties of the message box widget 
    processParam(parameters);
 
    connect(mMessageBox, SIGNAL(finished(HbAction*)), this, SLOT(messageBoxClosed(HbAction*)));
    
    TRACE_EXIT
    return true;
}


/*!
    Take parameter values and generate relevant property of this widget
 */
void BtDeviceOkOnlyDialogWidget::processParam(const QVariantMap &parameters)
{
    TRACE_ENTRY
    QString keyStr, prompt;
    QVariant param;
    keyStr.setNum( TBluetoothDialogParams::EDialogType );
    // Validate if the resource item exists.
    QVariantMap::const_iterator i = parameters.constFind( keyStr );
    // item of ResourceId is not found, can't continue.
    if ( i == parameters.constEnd() ) {
        mLastError = UnknownDeviceDialogError;
        return;
    }
    
    param = parameters.value( keyStr );
    int key = param.toInt();
    switch ( key ) {
        case TBluetoothDialogParams::bt_053_d_unable_to_use_no_sim:
            // Todo : Update this string ID when we have it
            prompt = QString( hbTrId( "txt_bt_title_unable_to_enter_sim_access_profile" ) );
            break;
        case TBluetoothDialogParams::bt_053_d_unable_to_use:
            prompt = QString( hbTrId( "txt_bt_title_unable_to_enter_sim_access_profile" ) );
            break;
        default:
            mLastError = ParameterError;
            break;
    }
    mMessageBox->setStandardButtons( HbMessageBox::Ok);
    mMessageBox->setText( prompt );
    TRACE_EXIT
}

/*!
    Reset properties to default values
 */
void BtDeviceOkOnlyDialogWidget::resetProperties()
{
    TRACE_ENTRY
    // set to default values
    mMessageBox->setModal(true);
    mMessageBox->setTimeout(HbDialog::StandardTimeout);
    mMessageBox->setDismissPolicy(HbDialog::NoDismiss);
    TRACE_EXIT
    return;
}


void BtDeviceOkOnlyDialogWidget::messageBoxClosed(HbAction* action)
{
    TRACE_ENTRY
    Q_UNUSED(action);
    QVariantMap data;
    data.insert( QString( "result" ), QVariant(true));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
    TRACE_EXIT
}
