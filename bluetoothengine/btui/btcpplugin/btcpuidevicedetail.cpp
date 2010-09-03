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

#include "btcpuidevicedetail.h"
#include "btcpuideviceview.h"
#include "btcpuidevicedetailsview.h"

#include <btdevsettingpluginloader.h>
#include <bluetoothuitrace.h>
#include <HbInstance>


BtCpUiDeviceDetail::BtCpUiDeviceDetail(QObject *parent) :
    QObject(parent), mDeviceDetailView(0)
{
    mMainWindow = hbInstance->allMainWindows().first();
}

BtCpUiDeviceDetail::~BtCpUiDeviceDetail()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    clearDeviceDetailList();
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetail::loadDeviceDetailPlugins(QString& deviceAddress, QString& deviceName)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QList<BtDevSettingInterface*> interfaces;
    BtAbstractDevSetting *devSetting;
    
    clearDeviceDetailList();
    interfaces = BtDevSettingPluginLoader::loadDevSettingInterfaces();
    QList<BtDevSettingInterface*>::const_iterator i;
    
    for (i = interfaces.constBegin(); i != interfaces.constEnd(); ++i) {
        devSetting = 0;
        devSetting = (*i)->createDevSetting( deviceAddress );
        BtTraceQString1( TRACE_DEBUG, DUMMY_DEVLIST, "device addr=", deviceAddress);
        if(devSetting) {
            BOstrace0(TRACE_FLOW, DUMMY_DEVLIST, "Plug-in settings appended");
            //If settings are available add to list and connect to plugin signals.
            appendDeviceToList(devSetting);
        }
    }

    for (int i = interfaces.count() - 1; i >= 0; --i ) {
        delete interfaces.at(i);
    }
    
    createDeviceDetailsView(deviceName);

    //Notify any changes in settings availability to Device view.
    //Based on this Device Settings button will be shown.
    notifyDeviceDetailStatus();
    BOstraceFunctionExit0(DUMMY_DEVLIST);   
}

void BtCpUiDeviceDetail::appendDeviceToList(BtAbstractDevSetting *devSetting)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    bool ret = false;
    BtDeviceDetails devicedetails;
    
    devicedetails.mSetting = devSetting;
    devicedetails.mSettingAvailable = devSetting->isSettingAvailable();
    devicedetails.mSettingForm = 0;
    
    if(devicedetails.mSettingAvailable) {
        devicedetails.mSettingForm = devSetting->getSettingWidget();
    }
    
    mDeviceDetailList.append(devicedetails);
    
    ret = connect(devicedetails.mSetting, SIGNAL(settingAvailabilityChanged(BtAbstractDevSetting *, bool)),
            this, SLOT(handleSettingChange(BtAbstractDevSetting *, bool)));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceDetail::appendDeviceToList", "connect settingAvailabilityChanged() failed");
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetail::handleSettingChange(BtAbstractDevSetting *setting, bool available)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, available );  
    QList<BtDeviceDetails>::iterator i;
       
    for (i = mDeviceDetailList.begin(); i != mDeviceDetailList.end(); ++i) {
        if((setting == (*i).mSetting) && mDeviceDetailView) {
            (*i).mSettingAvailable = available;
            if(available) {
                if(((*i).mSettingForm)) {
                    //If item already exists, remove it first.
                    BOstrace0(TRACE_FLOW, DUMMY_DEVLIST, "Setting removed");
                    mDeviceDetailView->removeItem((*i).mSettingForm);
                }
                (*i).mSettingForm = setting->getSettingWidget();
                BOstrace0(TRACE_FLOW, DUMMY_DEVLIST, "Setting added");
                //add widget
                mDeviceDetailView->addItem((*i).mSettingForm);
            }
            else {
                if((*i).mSettingForm) {
                    BOstrace0(TRACE_FLOW, DUMMY_DEVLIST, "Setting removed");
                    //remove widget
                    mDeviceDetailView->removeItem((*i).mSettingForm);
                    (*i).mSettingForm = 0;
                    checkDeviceDetailSettings();
                }
            }
            //Notify any changes in settings availability to Device view.
            //Based on this Device Settings button will be shown.
            notifyDeviceDetailStatus();
        }
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}


void BtCpUiDeviceDetail::checkDeviceDetailSettings()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QList<BtDeviceDetails>::const_iterator i;
    bool devicedetail = false;
    
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        if((*i).mSettingForm) {
            devicedetail = true;
            break;
        }
    }
    //If no setting is available and current view is device detail 
    //view move to previous view.
    if((!devicedetail) && (mMainWindow->currentView() == mDeviceDetailView)) {
        mMainWindow->removeView(mDeviceDetailView); 
        mMainWindow->setCurrentView( mPreviousView );
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetail::notifyDeviceDetailStatus()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QList<BtDeviceDetails>::const_iterator i;
    bool devicedetail = false;
    
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        if(((*i).mSettingAvailable == true) && ((*i).mSettingForm) ) {
            devicedetail = true;
            break;
        }
    }
    emit deviceSettingsChanged(devicedetail);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetail::clearDeviceDetailList()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QList<BtDeviceDetails>::const_iterator i;
    
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        if((*i).mSetting ) {
            delete (*i).mSetting;
        }
    }
    mDeviceDetailList.clear();
    
    if(mDeviceDetailView) {
        mDeviceDetailView->removeAllItems();
        delete mDeviceDetailView;
        mDeviceDetailView = 0;
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetail::createDeviceDetailsView(QString& deviceName)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    bool ret = false;
    //Launch Device Detail View, this view will be created once device view
    //is launched and will be deleted when device view is closed.
    mDeviceDetailView = new BtCpUiDeviceDetailsView();
    mDeviceDetailView->setDeviceName(deviceName);
    
    ret = connect(mDeviceDetailView, SIGNAL(aboutToClose()), this,
            SLOT(handleDeviceDetailViewClose()));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceDetail::createDeviceDetailsView", "connect deviceDetailViewClosed() failed");

    QList<BtDeviceDetails>::const_iterator i;
    
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        if((*i).mSettingForm) {
            mDeviceDetailView->addItem((*i).mSettingForm);
        }
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetail::loadDeviceDetailsView()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mPreviousView = mMainWindow->currentView();
    mMainWindow->addView(mDeviceDetailView);
    
    //aboutToForeground of plugin is called before launching 
    //device settings view.
    notifyViewStatusToPlugins(AboutToShow);
    
    mMainWindow->setCurrentView( mDeviceDetailView );
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetail::handleDeviceDetailViewClose()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    //aboutToBackground of plugin is called before going 
    //back to device view.
    notifyViewStatusToPlugins(AboutToHide);
    
    mMainWindow->removeView(mDeviceDetailView);
    
    mMainWindow->setCurrentView( mPreviousView );
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetail::notifyViewStatusToPlugins(BtCpUiDeviceDetail::NotifyType type)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QList<BtDeviceDetails>::const_iterator i;
    BtAbstractDevSetting *deviceSetting = 0;
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        deviceSetting = (*i).mSetting;
        if(deviceSetting) {
            switch(type) {
                case AboutToShow:
                    deviceSetting->aboutToForeground();
                    break;
                case AboutToHide:
                    deviceSetting->aboutToBackground();
                    break;
                case AboutToClose:
                    deviceSetting->aboutToClose();
                    break;
            }
        }
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetail::sendCloseEvent()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    //aboutToClose is called before going back from
    //device view.
    notifyViewStatusToPlugins(AboutToClose);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}


