/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BtDeviceDialogWidget class implementation.
*
*/


#include "btdevicedialognotifwidget.h"
#include "btdevicedialogplugintrace.h"
#include <bluetoothdevicedialogs.h>
#include <hbaction.h>
#include <hbdialog.h>
#include "btdevicedialogpluginerrors.h"

/*!
    class Constructor
 */
BtDeviceDialogNotifWidget::BtDeviceDialogNotifWidget( const QVariantMap &parameters )
{
    TRACE_ENTRY
    // set properties
    mLastError = NoError;
    mShowEventReceived = false;
    resetProperties();
    constructQueryDialog(parameters);
    TRACE_EXIT
}

/*!
    Set parameters, implementation of interface
    Invoked when HbDeviceDialog::update calls.
 */
bool BtDeviceDialogNotifWidget::setDeviceDialogParameters(
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
int BtDeviceDialogNotifWidget::deviceDialogError() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mLastError;
}

/*!
    Close notification, implementation of interface
 */ 
void BtDeviceDialogNotifWidget::closeDeviceDialog(bool byClient)
{
    TRACE_ENTRY
    Q_UNUSED(byClient);
    // Closed by client or internally by server -> no action to be transmitted.
    mSendAction = false;
    close();
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
HbDialog *BtDeviceDialogNotifWidget::deviceDialogWidget() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return const_cast<BtDeviceDialogNotifWidget*>(this);
}

/*!
    Construct display widget
 */
bool BtDeviceDialogNotifWidget::constructQueryDialog(const QVariantMap &parameters)
{
    TRACE_ENTRY
    // analyze the parameters to compose the properties of the message box widget 
    processParam(parameters);
 
    TRACE_EXIT
    return true;
}

/*!
    Take parameter values and generate relevant property of this widget
 */
void BtDeviceDialogNotifWidget::processParam(const QVariantMap &parameters)
{
    TRACE_ENTRY
    QString keyStr, prompt;
    keyStr.setNum( TBluetoothDialogParams::EResource );
    // Validate if the resource item exists.
    QVariantMap::const_iterator i = parameters.constFind( keyStr );
    // item of ResourceId is not found, can't continue.
    if ( i == parameters.constEnd() ) {
        mLastError = UnknownDeviceDialogError;
        return;
    }

    QVariant param = parameters.value( keyStr );
    int key = param.toInt();
    switch ( key ) {
        // Note dialogs
        case EPairingSuccess:
            prompt = QString( tr( "Pairing with %1 complete" ) );
            break;
        case EPairingFailure:
            prompt = QString( tr( "Unable to pair with %1" ) );
            break;            
        case EVisibilityTimeout:
            prompt = QString( tr( "Phone is not detectable in searches made by other devices" ) );
            break;
        default:
            mLastError = ParameterError;
            break;
    }
    // Could use QChar with ReplacementCharacter?
    int repls = prompt.count( QString( "%" ) );
    if ( repls > 0 ) {
        QVariant name = parameters.value( QString::number( TBluetoothDeviceDialog::EDeviceName ) );
        prompt = prompt.arg( name.toString() );
    }
    // set property value to this dialog widget
    HbNotificationDialog::setTitle( prompt );
    TRACE_EXIT
}

/*!
    Reset properties to default values
 */
void BtDeviceDialogNotifWidget::resetProperties()
{
    TRACE_ENTRY
    mSendAction = true;
    TRACE_EXIT
    return;
}

/*!
    Widget is about to hide. Closing effect has ended.
 */
void BtDeviceDialogNotifWidget::hideEvent(QHideEvent *event)
{
    HbNotificationDialog::hideEvent(event);
    emit deviceDialogClosed();
}

/*!
    Widget is about to show
 */
void BtDeviceDialogNotifWidget::showEvent(QShowEvent *event)
{
    HbNotificationDialog::showEvent(event);
    mShowEventReceived = true;
}
