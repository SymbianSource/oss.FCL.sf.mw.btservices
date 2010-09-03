/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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

#include "btcpuisettingitem.h"
#include <cpitemdatahelper.h>
#include <bluetoothuitrace.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <HbInstance>
#include <HbTranslator>
#include <HbView>
#include "btcpuimainview.h"

BtcpuiSettingItem::BtcpuiSettingItem(CpItemDataHelper &itemDataHelper) :
    CpSettingFormEntryItemData(itemDataHelper), mMainView(0)
{
    bool ret(false);
    loadTranslators();
    mSettingModel = new BtSettingModel(this);
    mDeviceModel = new BtDeviceModel(this);
    mMainWindow = hbInstance->allMainWindows().first();
    ret = connect(mSettingModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), 
            this, SLOT(handleDataChanged(QModelIndex,QModelIndex)));
    BTUI_ASSERT_X( ret, "BtcpuiSettingItem::BtcpuiSettingItem", "can't connect dataChanged" );

    this->setDescription(hbTrId("txt_cp_dblist_bluetooth"));
    updateStatus();
}

BtcpuiSettingItem::~BtcpuiSettingItem()
{
    delete mMainView;
    delete mViewTranslator;
    delete mDialogTranslator;
}

void BtcpuiSettingItem::onLaunchView()
{
    // Main view will be launched when the user clicked the Bluetooth setting item in CP.

    // We need to save the pointer of the view from which the main view is launched, 
    // for navigating back to CP when the main view is closed.
    mCpView = mMainWindow->currentView();
    mMainView = new BtcpuiMainView(*mSettingModel, *mDeviceModel);
    mMainWindow->addView(mMainView);
    connect(mMainView, SIGNAL(aboutToClose()), this, SLOT(handleCloseMainView()));
    mMainWindow->setCurrentView(mMainView);
}

void BtcpuiSettingItem::handleCloseMainView()
{
    if (mMainView) {
        mMainWindow->removeView(mMainView);
        delete mMainView;
        mMainView = 0;
    }
    mMainWindow->setCurrentView(mCpView);
}

/*!
    Slot for receiving notification of local setting changes from the model.
    Identify the setting changed and update the corresponding UI item.
 */
void BtcpuiSettingItem::handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{     
    // update only the part of the view specified by the model's row(s)
    for (int i=topLeft.row(); i <= bottomRight.row(); i++) {
        QModelIndex index = mSettingModel->index( i, 0);
        // Distinguish which setting value is changed.
        switch ( i ) {
        case BtSettingModel::PowerStateRow:
        case BtSettingModel::VisibilityRow:
            updateStatus();
            break;
        }
    }   
}

CpBaseSettingView *BtcpuiSettingItem::createSettingView() const
{
    return 0;
}

void BtcpuiSettingItem::updateStatus()
{
    QString btStatusText;
    HbIcon btStatusIcon;
    
    //todo: Connection status is not updated right now, which is also required as per UI Spec.
    PowerStateQtValue btPower = (PowerStateQtValue)mSettingModel->data(mSettingModel->index(
            BtSettingModel::PowerStateRow, 0),
            BtSettingModel::SettingValueRole).toInt();
    
    if(BtPowerOn == btPower) {
        VisibilityMode visibilityMode = (VisibilityMode) mSettingModel->data(
                mSettingModel->index(BtSettingModel::VisibilityRow, 0),
                BtSettingModel::SettingValueRole).toInt();
        switch(visibilityMode) {
            case BtHidden:
                btStatusText = hbTrId("txt_cp_dblist_bluetooth_val_on_and_hidden");
                btStatusIcon.setIconName("qtg_large_bluetooth_hide");
                break;
            case BtVisible:
            case BtTemporary:
                btStatusText = hbTrId("txt_cp_dblist_bluetooth_val_on_and_visible");
                btStatusIcon.setIconName("qtg_large_bluetooth");
                break;
            default:
                BTUI_ASSERT_X(false, "BtcpuiSettingItem::updateStatus", "invalid visibility mode");
        }
    }
    else {
        //Bt is off.
        btStatusText = hbTrId("txt_cp_dblist_bluetooth_val_off");
        btStatusIcon.setIconName("qtg_large_bluetooth_off");
    }
    this->setDescription(btStatusText);
    this->setEntryItemIcon(btStatusIcon);
}


void BtcpuiSettingItem::loadTranslators()
{
    mViewTranslator = new HbTranslator("btviews");
    mDialogTranslator = new HbTranslator("btdialogs");
}

