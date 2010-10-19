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
* Description:  BtDeviceDialogWidget class declaration.
*
*/


#include "btdevicedialogrecvquerywidget.h"
#include "bluetoothdevicedialogs.h"
#include "btdevicedialogpluginerrors.h"
#include <btuiiconutil.h>

const char* DOCML_BT_RECV_QUERY_DIALOG = ":/docml/bt-receive-auth-dialog.docml";


BTRecvQueryDialogWidget::BTRecvQueryDialogWidget(const QVariantMap &parameters)
:mLoader(0),
mError(NoError)
{
    constructDialog(parameters);
}

BTRecvQueryDialogWidget::~BTRecvQueryDialogWidget()
{
    if(mLoader)
    {
        delete mLoader;
        mLoader = NULL;
    }
}

bool BTRecvQueryDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    Q_UNUSED(parameters);
    if(!mError)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int BTRecvQueryDialogWidget::deviceDialogError() const
{
    return mError;
}

void BTRecvQueryDialogWidget::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    mDialog->close();
    emit deviceDialogClosed();
}

HbPopup* BTRecvQueryDialogWidget::deviceDialogWidget() const
{
    return mDialog;
}

QObject* BTRecvQueryDialogWidget::signalSender() const
{
    return const_cast<BTRecvQueryDialogWidget*>(this);
}

void BTRecvQueryDialogWidget::constructDialog(const QVariantMap &parameters)
{
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_RECV_QUERY_DIALOG, &ok);
    if(ok)
    {
        mDialog = qobject_cast<HbDialog*>(mLoader->findWidget("receiveAuthorizationDialog"));
        mHeading = qobject_cast<HbLabel*>(mLoader->findWidget("receiveAuthorizationHeading"));
        
        mDeviceName = qobject_cast<HbLabel*>(mLoader->findWidget("deviceName"));
        mDeviceType = qobject_cast<HbLabel*>(mLoader->findWidget("deviceType"));
        mDeviceIcon = qobject_cast<HbLabel*>(mLoader->findWidget("deviceIcon"));
        
        int classOfDevice = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass)).toDouble();
        HbIcon icon = getBadgedDeviceTypeIcon(classOfDevice);
        mDeviceIcon->setIcon(icon);
                
        mDeviceName->setPlainText(parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceName)).toString());
        mDeviceType->setPlainText(getDeviceTypeString(classOfDevice));
        
        mYesAction = qobject_cast<HbAction*>(mLoader->findObject("yesAction"));
        mNoAction = qobject_cast<HbAction*>(mLoader->findObject("noAction"));
        
        mAuthorizeUser = qobject_cast<HbCheckBox*>(mLoader->findWidget("authorizeUser"));

        int dialogType = parameters.value(QString::number(TBluetoothDialogParams::EDialogTitle)).toInt();
        switch(dialogType)
        {
            case TBluetoothDialogParams::EReceive:
            {
                mHeading->setPlainText(hbTrId("txt_bt_title_receive_messages_from"));
            }break;
                
            case TBluetoothDialogParams::EReceiveFromPairedDevice:
            {
                mHeading->setPlainText(hbTrId("txt_bt_title_receive_messages_from_paired_device"));
                mAuthorizeUser->setCheckState(Qt::Checked);
            }break;
            
            case TBluetoothDialogParams::EConnect:
            {
                mHeading->setPlainText(hbTrId("txt_bt_title_connect_to"));
                mAuthorizeUser->setCheckState(Qt::Checked);
            }break;
            case TBluetoothDialogParams::EPairingRequest:
                mHeading->setPlainText(hbTrId("txt_bt_title_pair_with"));
                mAuthorizeUser->setCheckState(Qt::Checked);                
                break;
            default:
                break;

        }
        mDialog->setHeadingWidget(mHeading);

        mDialog->setBackgroundFaded(false);
        mDialog->setDismissPolicy(HbPopup::NoDismiss);
        mDialog->setTimeout(HbPopup::NoTimeout);
         
        connect(mYesAction, SIGNAL(triggered()), this, SLOT(yesClicked()));
        connect(mNoAction, SIGNAL(triggered()), this, SLOT(noClicked()));
        connect(mAuthorizeUser, SIGNAL(clicked(bool)), this, SLOT(checkBoxStateChanged(bool)));
    }
    else
    {
        mError = DocMLLoadingError;
    }
}

void BTRecvQueryDialogWidget::yesClicked()
{
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(true));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

void BTRecvQueryDialogWidget::noClicked()
{
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(false));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

void BTRecvQueryDialogWidget::checkBoxStateChanged(bool checked)
{
    QVariantMap data;
    data.insert(QString("checkBoxState"), QVariant(checked));
    emit deviceDialogData(data);
}



