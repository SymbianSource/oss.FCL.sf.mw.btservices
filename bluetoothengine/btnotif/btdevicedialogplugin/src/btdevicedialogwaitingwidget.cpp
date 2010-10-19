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
* Description:  BtDeviceDialogWaitingWidget class declaration.
*
*/


#include "btdevicedialogwaitingwidget.h"
#include "bluetoothdevicedialogs.h"
#include "btdevicedialogpluginerrors.h"
#include <hblabel.h>
#include <hbprogressbar.h>

const char* DOCML_BT_WAITING_DIALOG = ":/docml/bt-waiting-dialog.docml";


BtDeviceDialogWaitingWidget::BtDeviceDialogWaitingWidget(const QVariantMap &parameters)
:mLoader(0),mDialog(0),mLastError(NoError)
{
    constructDialog(parameters);
}

BtDeviceDialogWaitingWidget::~BtDeviceDialogWaitingWidget()
{
    delete mLoader;
}

bool BtDeviceDialogWaitingWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    Q_UNUSED(parameters);
    return true;
}

int BtDeviceDialogWaitingWidget::deviceDialogError() const
{
    return mLastError;
}

void BtDeviceDialogWaitingWidget::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    mDialog->close();
    emit deviceDialogClosed();
}

HbPopup* BtDeviceDialogWaitingWidget::deviceDialogWidget() const
{
    return mDialog;
}

QObject* BtDeviceDialogWaitingWidget::signalSender() const
{
    return const_cast<BtDeviceDialogWaitingWidget*>(this);
}

bool BtDeviceDialogWaitingWidget::constructDialog(const QVariantMap &parameters)
{
    Q_UNUSED(parameters);
    HbLabel *heading;
    HbProgressBar* progressBar;
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_WAITING_DIALOG, &ok);
    if(ok)
    {
        mDialog = qobject_cast<HbDialog *>( mLoader->findWidget( "dialog" ) );
        if(mDialog == 0)
        {
            mLastError = UnknownDeviceDialogError; 
            return false;
        }
        heading = qobject_cast<HbLabel *>( mLoader->findWidget( "heading" ) );
        if(heading == 0)
        {
            mLastError = UnknownDeviceDialogError; 
            return false;
        }    
        progressBar = qobject_cast<HbProgressBar *>( mLoader->findWidget( "progressBar" ) );
        if(progressBar == 0)
        {
            mLastError = UnknownDeviceDialogError; 
            return false;
        }
    
        progressBar->setRange(0,0);
        heading->setPlainText(hbTrId("txt_bt_info_entering_sim_access_profile"));
        
        mDialog->setBackgroundFaded(false);
        mDialog->setDismissPolicy(HbPopup::NoDismiss);
        mDialog->setTimeout(HbPopup::NoTimeout);
        return true;
    }
    else
    {
	mLastError = DocMLLoadingError;
        return false;
    }
}




