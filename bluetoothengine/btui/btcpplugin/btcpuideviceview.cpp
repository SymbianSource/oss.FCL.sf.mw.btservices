/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "btcpuideviceview.h"
#include "btuiviewutil.h"
#include <QtGui/QGraphicsLinearLayout>
#include <HbInstance>
#include <hbdocumentloader.h>
#include <hbdataform.h>
#include <hbgroupbox.h>
#include <hbpushbutton.h>
#include <hblabel.h>
#include <hbtextedit.h>
#include <hblistview.h>
#include <hbmenu.h>
#include <qstring>
#include <qstringlist>
#include <qdebug>
#include <bluetoothuitrace.h>
#include "btcpuimainview.h"
#include <btabstractdelegate.h>
#include <btdelegatefactory.h>
#include <QModelIndex>

// docml to load
const char* BTUI_DEVICEVIEW_DOCML = ":/docml/bt-device-view.docml";


BtCpUiDeviceView::BtCpUiDeviceView(
        BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel, 
        QGraphicsItem *parent) :
    BtCpUiBaseView(settingModel,deviceModel,parent),
    mPairStatus(false), mConnectStatus(false), mConnectable(false), mAbstractDelegate(0)   
{
    mDeviceIndex = QModelIndex();//is it needed to initialize mIndex???
    
    mMainView = (BtCpUiMainView *) parent;
    
    mMainWindow = hbInstance->allMainWindows().first();
    
    mSoftKeyBackAction = new HbAction(Hb::BackNaviAction, this);
    BTUI_ASSERT_X(mSoftKeyBackAction, "BtCpUiBaseView::BtCpUiBaseView", "can't create back action");

    // read view info from docml file

    // Create view for the application.
    // Set the name for the view. The name should be same as the view's
    // name in docml.
    setObjectName("bt_device_view");

    QObjectList objectList;
    objectList.append(this);
    // Pass the view to documentloader. Document loader uses this view
    // when docml is parsed, instead of creating new view.
    mLoader = new HbDocumentLoader();
    mLoader->setObjectTree(objectList);
    
    bool ret = false;

    bool ok = false;
    mLoader->load( BTUI_DEVICEVIEW_DOCML, &ok );
    // Exit if the file format is invalid
    BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file" );
    
    // Set title for the control panel
    // ToDo:  check if deprecated API
    setTitle("Control Panel");

    // assign automatically created widgets to local variables
    
    mGroupBox = 0;
    mGroupBox = qobject_cast<HbGroupBox *>( mLoader->findWidget( "groupBox_deviceView" ) );
    BTUI_ASSERT_X( mGroupBox != 0, "bt-device-view", "Device groupbox not found" );
    
    mDeviceIcon=0;
    //can't use qobject_cast since HbIcon is not derived from QObject!
    mDeviceIcon = qobject_cast<HbLabel *>( mLoader->findWidget( "deviceIcon" ) );  
    BTUI_ASSERT_X( mDeviceIcon != 0, "bt-device-view", "Device Icon not found" );
    
    mDeviceName=0;
    mDeviceName = qobject_cast<HbTextEdit *>( mLoader->findWidget( "deviceName" ) );
    BTUI_ASSERT_X( mDeviceName != 0, "bt-device-view", "Device Name not found" );
    ret = connect(mDeviceName, SIGNAL(editingFinished ()), this, SLOT(changeBtDeviceName()));
    
    mDeviceCategory=0;
    mDeviceCategory = qobject_cast<HbLabel *>( mLoader->findWidget( "deviceCategory" ) );  
    BTUI_ASSERT_X( mDeviceCategory != 0, "bt-device-view", "Device Category not found" );
    
    mDeviceStatus=0;
    mDeviceStatus = qobject_cast<HbLabel *>( mLoader->findWidget( "deviceStatus" ) );  
    BTUI_ASSERT_X( mDeviceStatus != 0, "bt-device-view", "Device status not found" );
    
    mPair_Unpair=0;
    mPair_Unpair = qobject_cast<HbPushButton *>( mLoader->findWidget( "pushButton_0" ) );
    BTUI_ASSERT_X( mPair_Unpair != 0, "bt-device-view", "pair/unpair button not found" );
    ret =  connect(mPair_Unpair, SIGNAL(clicked()), this, SLOT(pairUnpair()));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceView::BtCpUiDeviceView", "can't connect pair button" );
 
    mConnect_Disconnect=0;
    mConnect_Disconnect = qobject_cast<HbPushButton *>( mLoader->findWidget( "pushButton_1" ) );
    BTUI_ASSERT_X( mConnect_Disconnect != 0, "bt-device-view", "connect/disconnect button not found" );
    ret =  connect(mConnect_Disconnect, SIGNAL(clicked()), this, SLOT(connectDisconnect()));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceView::BtCpUiDeviceView", "can't connect disconnect button" );
      
    mDeviceSetting = 0;
    mDeviceSetting = qobject_cast<HbPushButton *>( mLoader->findWidget( "pushButton_2" ) );
    BTUI_ASSERT_X( mDeviceSetting != 0, "bt-device-view", "settings button not found" );
        
    // read landscape orientation section from docml file if needed
    // mOrientation = ((BTUIViewManager*)parent)->orientation();
    mOrientation = Qt::Vertical;
    if (mOrientation == Qt::Horizontal) {
        mLoader->load(BTUI_DEVICEVIEW_DOCML, "landscape", &ok);
        BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file: landscape section problem" );
    }
 
}

BtCpUiDeviceView::~BtCpUiDeviceView()
{
    setNavigationAction(0);
    delete mSoftKeyBackAction;
    if(mAbstractDelegate)
    {
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
}


void BtCpUiDeviceView::setSoftkeyBack()
{
    if (navigationAction() != mSoftKeyBackAction) {
        setNavigationAction(mSoftKeyBackAction);
        connect( mSoftKeyBackAction, SIGNAL(triggered()), this, SLOT(switchToPreviousView()) );
    }
}

void BtCpUiDeviceView::switchToPreviousView()
{
    BTUI_ASSERT_X(mMainView, "BtCpUiSearchView::switchToPreviousView", "invalid mMainView");
    mMainView->switchToPreviousView();
}

void BtCpUiDeviceView::activateView( const QVariant& value, int cmdId )
{
    Q_UNUSED(cmdId);  
    
    setSoftkeyBack();
    
    QModelIndex index = value.value<QModelIndex>();
    mDeviceBdAddr = (mDeviceModel->data(index, BtDeviceModel::ReadableBdaddrRole));
    
    //activate view is called when device is selected
    clearViewData();
    updateDeviceData();
    
    bool ret(false);
    ret=connect(mDeviceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           this, SLOT(updateDeviceData()));
    BTUI_ASSERT_X( ret, "Btui, BtCpUiDeviceView::activateView", "dataChanged() connect failed");
}

void BtCpUiDeviceView::deactivateView()
{
}

void BtCpUiDeviceView::clearViewData()
{
    mDeviceIcon->clear();
    mDeviceCategory->clear();
    mDeviceStatus->clear();
    
    mPairStatus = false;
    mConnectStatus = false;
    mConnectable = false;
}
    
void BtCpUiDeviceView::updateDeviceData()
{
    QModelIndex localIndex = mSettingModel->index( BtSettingModel::LocalBtNameRow, 0);
    QString localName = (mSettingModel->data(localIndex,BtSettingModel::settingDisplayRole)).toString();
    QString groupBoxTitle (tr("Bluetooth-"));
    mGroupBox->setHeading(groupBoxTitle.append(localName));
    //Get the QModelIndex of the device using the device BDAddres
    QModelIndex start = mDeviceModel->index(0,0);
    QModelIndexList indexList = mDeviceModel->match(start,BtDeviceModel::ReadableBdaddrRole, mDeviceBdAddr);
    mDeviceIndex = indexList.at(0);
    
    //populate device view with device data fetched from UiModel
    QString deviceName = (mDeviceModel->data(mDeviceIndex, 
             BtDeviceModel::NameAliasRole)).toString(); 
    mDeviceName->setPlainText(deviceName);
     
    int cod = (mDeviceModel->data(mDeviceIndex,BtDeviceModel::CoDRole)).toInt();
     
    int majorRole = (mDeviceModel->data(mDeviceIndex,BtDeviceModel::MajorPropertyRole)).toInt();
    int minorRole = (mDeviceModel->data(mDeviceIndex,BtDeviceModel::MinorPropertyRole)).toInt();
    
	setDeviceCategory(cod, majorRole, minorRole);
    setDeviceStatus(majorRole);
    setTextAndVisibilityOfButtons();
}

void BtCpUiDeviceView::setDeviceCategory(int cod,int majorRole, int minorRole)
{
    //TODO: change the hardcoded numeric value to enumerations
    if (cod)
    {
        if (majorRole & 0x00020000)
        {
            //this is a phone
            mDeviceCategory->setPlainText(tr("Phone"));
        }
        else if (majorRole & 0x00010000)
        {
            //this is a computer
            mDeviceCategory->setPlainText(tr("Computer"));
        
        }
        else if (majorRole & 0x00080000)
        {  
            //this is a A/V device
            //int minorRole = (mDeviceModel->data(mIndex,BtDeviceModel::MinorPropertyRole)).toInt();
            if ( minorRole & 0x00000002)
            {
                //this is a Headset, it is possible to connect
                mConnectable = true;
                mDeviceCategory->setPlainText(tr("Headset"));
            }  
        }
        else 
        {
            mDeviceCategory->setPlainText(tr("Uncategorized Dev"));
        }
    
    }
    
    
}
void BtCpUiDeviceView::setDeviceStatus(int majorRole)
{
    //TODO: change the hardcoded numeric value to enumerations
    QString deviceStatus;
    if (majorRole & 0x00000001)
    {
        deviceStatus = deviceStatus.append(tr("Paired"));
        mPairStatus = true;
    }
    else
    {
        mPairStatus = false;
    }
    
    if ((majorRole & 0x00000020)&& (mConnectable))
    {
        //if the device is connected and it is a headset NOTE! two phone can be paired but not be connected
        deviceStatus = deviceStatus.append(tr(", connected"));
        mConnectStatus = true;
    }
    else 
    {
        mConnectStatus = false;
    }
    mDeviceStatus->setPlainText(deviceStatus);
    
    
    
}

void BtCpUiDeviceView::setTextAndVisibilityOfButtons()
{
    if (mPairStatus)
    {
        mPair_Unpair->setText(tr("Unpair"));
    }
    else
    {
        mPair_Unpair->setText(tr("Pair"));
    }
    
    if (mConnectable)
    {
        if (mConnectStatus)
        {
            mConnect_Disconnect->setText(tr("Disconnect"));
        }
        else
        {
            mConnect_Disconnect->setText(tr("Connect"));
        }
        
    }
    else
    {
        //it is not possible to connect, set the button invisible
        mConnect_Disconnect->setVisible(false);
    }
    
    mDeviceSetting->setVisible(false);

}

void BtCpUiDeviceView::changeBtDeviceName()
{
    /*
    if (!mAbstractDelegate) 
    {
        mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::DeviceName, mModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int,QVariant)), this, SLOT(btNameDelegateCompleted(int,QVariant)) );
        mAbstractDelegate->exec(mDeviceNameEdit->text ());
    }
    */
    
}

void BtCpUiDeviceView::pairUnpair()
{
    if (mPairStatus)
    {
        //if the device is paired, call unpairDevice() when the button is tabbed
        unpairDevice();
    }
    else
    {
        //if the device is unpaired, call pairDevice() when the button is tabbed
        pairDevice();

    }
    
    
}

void BtCpUiDeviceView::connectDisconnect()
{
    if (mConnectStatus)
    {
        //if the device is connected, call disconnectDevice() when the button is tabbed
        disconnectDevice();
    }
    else
    {
        //if the device is disconnected, call connectDevice() when the button is tabbed
        connectDevice();

    }
}

void BtCpUiDeviceView::pairDevice()
{
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        QVariant params;
        params.setValue(mDeviceIndex);
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::Pair, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(pairDelegateCompleted(int)) );
        mAbstractDelegate->exec(params);
    }
    
}

void BtCpUiDeviceView::pairDelegateCompleted(int status)
{
    Q_UNUSED(status);
    //TODO: handle the error here
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
}

void BtCpUiDeviceView::unpairDevice()
{
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        QVariant params;
        params.setValue(mDeviceIndex);
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::Unpair, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(unpairDelegateCompleted(int)) );
        mAbstractDelegate->exec(params);
    }
        
    
}

void BtCpUiDeviceView::unpairDelegateCompleted(int status)
{
    Q_UNUSED(status);
    //TODO: handle the error here 
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
}

void BtCpUiDeviceView::connectDevice()
{
    
    
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        QVariant params;
        params.setValue(mDeviceIndex);
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::Connect, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(connectDelegateCompleted(int)) );
        mAbstractDelegate->exec(params);
    }
    
    
}

void BtCpUiDeviceView::connectDelegateCompleted(int status)
{
    Q_UNUSED(status);
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }   
    
    
}

void BtCpUiDeviceView::disconnectDevice()
{
    if (!mAbstractDelegate)//if there is no other delegate running
        { 
            QVariant params;
            params.setValue(mDeviceIndex);
            mAbstractDelegate = BtDelegateFactory::newDelegate(
                    BtDelegate::Disconnect, mSettingModel, mDeviceModel); 
            connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(disconnectDelegateCompleted(int)) );
            mAbstractDelegate->exec(params);
        }
    
}

void BtCpUiDeviceView::disconnectDelegateCompleted(int status)
{
    Q_UNUSED(status);
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }   
    
    
}

