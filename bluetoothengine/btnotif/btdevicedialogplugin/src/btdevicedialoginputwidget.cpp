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
* Description:  BtDeviceDialogWidget class implementation.
*
*/


#include "btdevicedialoginputwidget.h"
#include "btdevicedialogplugintrace.h"
#include <bluetoothdevicedialogs.h>
#include <hbaction.h>
#include <hbdialog.h>
#include <hblabel.h>
#include <hbvalidator.h>
#include <hbparameterlengthlimiter.h>
#include "btdevicedialogpluginerrors.h"

const int PASSCODE_MAX_LEN = 16; // from BT specs

/*!
    class Constructor
 */
BtDeviceDialogInputWidget::BtDeviceDialogInputWidget(
        const QVariantMap &parameters)
{
    TRACE_ENTRY
    // set properties
    mLastError = NoError;
    mShowEventReceived = false;
    mInputDialog = new HbInputDialog();
    
    resetProperties();
    constructInputDialog(parameters);
    TRACE_EXIT
}

/*!
    Set parameters, implementation of interface
    Invoked when HbDeviceDialog::update calls.
 */
bool BtDeviceDialogInputWidget::setDeviceDialogParameters(
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
int BtDeviceDialogInputWidget::deviceDialogError() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mLastError;
}

/*!
    Close notification, implementation of interface
 */ 
void BtDeviceDialogInputWidget::closeDeviceDialog(bool byClient)
{
    TRACE_ENTRY
    Q_UNUSED(byClient);
    // Closed by client or internally by server -> no action to be transmitted.
    mSendAction = false;
    mInputDialog->close();
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
HbDialog *BtDeviceDialogInputWidget::deviceDialogWidget() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mInputDialog;
}

QObject *BtDeviceDialogInputWidget::signalSender() const
{
    return const_cast<BtDeviceDialogInputWidget*>(this);
}     

/*!
    Construct display widget
 */
bool BtDeviceDialogInputWidget::constructInputDialog(const QVariantMap &parameters)
{
    TRACE_ENTRY
    // analyze the parameters to compose the properties of the widget
    processParam(parameters);
    connect(mInputDialog, SIGNAL(finished(HbAction*)), this, SLOT(inputClosed(HbAction*)));
    
    TRACE_EXIT
    return true;
}

/*!
    Take parameter values and generate relevant property of this widget
 */
void BtDeviceDialogInputWidget::processParam(const QVariantMap &parameters)
{
    TRACE_ENTRY

    QString keyStr, prompt,title,regExp;
    QVariant name;
    bool minLenRequired = false;
    
    keyStr.setNum( TBluetoothDialogParams::EResource );
    // Validate if the resource item exists.
    QVariantMap::const_iterator i = parameters.constFind( keyStr );
    // item of ResourceId is not found, can't continue.
    if ( i == parameters.constEnd() ) {
        mLastError = UnknownDeviceDialogError;
        return;
    }

    QVariant param = parameters.value( keyStr );
  
    if(param.toInt() != EPinInput)
        {
        mLastError = ParameterError;
        return;
        }
    mInputDialog->setInputMode(HbInputDialog::RealInput);
    name = parameters.value( QString::number( TBluetoothDeviceDialog::EDeviceName ) );
    title = HbParameterLengthLimiter(hbTrId("txt_bt_title_pairing_with_1")).arg(name.toString());
    
    // Set minimum input length requirements
    keyStr.setNum( TBluetoothDeviceDialog::EAdditionalDesc );
    i = parameters.constFind( keyStr );
    // Min Length required
    if ( i != parameters.constEnd() ) {
        minLenRequired = true;
        param = parameters.value( keyStr );
        regExp = "^\\d{%1,}$";
        regExp.arg(param.toString());
        HbValidator* validator = new HbValidator(mInputDialog->lineEdit());
        validator->addField(
                new QRegExpValidator(
                        QRegExp(regExp, Qt::CaseInsensitive), validator ),"");
        mInputDialog->setValidator(validator);
    }else{
        // Minimum requirement is to have at least 1 digit
        regExp = "^\\d{1,}$";
        HbValidator* validator = new HbValidator(mInputDialog->lineEdit());
        validator->addField(
                new QRegExpValidator(
                        QRegExp(regExp, Qt::CaseInsensitive), validator ),"");
        mInputDialog->setValidator(validator);    
    }
    if(minLenRequired)
        {
        // Todo : The string ID for the localization is not available yet
        // for string : "Enter %1 digit passcode for device %2:"
        // I'm using the "Enter the passcode for device %1" instead
        prompt = HbParameterLengthLimiter(hbTrId("txt_bt_dialog_please_enter_the_passcode_for_1" )).arg(name.toString());
        }
    else
        {
        prompt = HbParameterLengthLimiter(hbTrId("txt_bt_dialog_please_enter_the_passcode_for_1" )).arg(name.toString());
        }
    mInputDialog->setHeadingWidget(new HbLabel(title));
    mInputDialog->lineEdit(0)->setMaxLength(PASSCODE_MAX_LEN);
    mInputDialog->lineEdit(0)->setText(tr("")); // clear the input field
    mInputDialog->setPromptText(prompt);
    TRACE_EXIT
}

/*!
    Reset properties to default values
 */
void BtDeviceDialogInputWidget::resetProperties()
{
    TRACE_ENTRY
    // set to default values
    mInputDialog->setModal(true);
    mInputDialog->setTimeout(HbDialog::NoTimeout);
    mInputDialog->setDismissPolicy(HbDialog::NoDismiss);
    mSendAction = true;
    // Todo: clean the Validator
    TRACE_EXIT
    return;
}

void BtDeviceDialogInputWidget::inputClosed(HbAction *action)
{
    QVariantMap data;
    
    HbInputDialog *dlg=static_cast<HbInputDialog*>(sender());
    if(dlg->actions().first() == action) {
        //Ok
        QVariant result( dlg->value().toString().toUtf8() );
        data.insert( QString( "result" ), QVariant(true));
        data.insert( QString( "input" ), result );
   } 
    else if(dlg->actions().at(1) == action) {
        //Cancel
        data.insert( QString( "result" ), QVariant(false));
    }

    emit deviceDialogData(data);
    emit deviceDialogClosed();
    mSendAction = false;
}


