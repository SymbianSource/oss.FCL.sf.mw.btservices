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

#include "btcpuideviceview.h"
#include "btuiviewutil.h"
#include <QtGui/QGraphicsLinearLayout>
#include <hbmainwindow.h>
#include <hbdocumentloader.h>
#include <hbcombobox.h>
#include <hbgroupbox.h>
#include <hbpushbutton.h>
#include <hblabel.h>
#include <hbicon.h>
#include <hblineedit.h>
#include <hblistview.h>
#include <hbmenu.h>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <bluetoothuitrace.h>
#include <btabstractdelegate.h>
#include <btdelegatefactory.h>
#include "btuiiconutil.h"
#include "btuidevtypemap.h"
#include "btqtconstants.h"
#include "btcpuidevicedetail.h"
#include "btcpuiviewmgr.h"

// docml to load
const char* BTUI_DEVICEVIEW_DOCML = ":/docml/bt-device-view.docml";


// ToDo:  should use base class delegate instead of mAbstractDelegate, also base class createDelegate()
// and createExecuteDelegate()

BtcpuiDeviceView::BtcpuiDeviceView(BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel, 
        QGraphicsItem *parent) :
    BtcpuiBaseView(settingModel,deviceModel,parent),
    mComboboxIndex(-1), mAbstractDelegate(0), mDeviceDetail(0)
{
    mDeviceIndex = QModelIndex(); // initialize to zero

    // read view info from docml file

    // Create view for the application.
    // Set the name for the view. The name should be same as the view's
    // name in docml.
    setObjectName("bt_device_view");

    mLoader = new HbDocumentLoader();
    // Pass the view to documentloader. Document loader uses this view
    // when docml is parsed, instead of creating new view.
    QObjectList objectList;
    objectList.append(this);
    mLoader->setObjectTree(objectList);
    
    bool ret = false;

    mLoader->load( BTUI_DEVICEVIEW_DOCML, &ret );
    // Exit if the file format is invalid
    BTUI_ASSERT_X( ret, "bt-device-view", "Invalid docml file" );
    // listen for orientation changes
    ret = connect(mMainWindow, SIGNAL(orientationChanged(Qt::Orientation)),
            this, SLOT(changeOrientation(Qt::Orientation)));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceView::BtCpUiDeviceView()", "connect orientationChanged() failed");
    
    // assign automatically created widgets to local variables
    
    mDeviceIcon=0;
    mDeviceIcon = qobject_cast<HbLabel *>( mLoader->findWidget( "deviceIcon" ) );  
    BTUI_ASSERT_X( mDeviceIcon != 0, "bt-device-view", "Device Icon not found" );
    
    mDeviceName=0;
    mDeviceName = qobject_cast<HbLineEdit *>( mLoader->findWidget( "deviceName" ) );
    BTUI_ASSERT_X( mDeviceName != 0, "bt-device-view", "Device Name not found" );
    ret = connect(mDeviceName, SIGNAL(editingFinished ()), this, SLOT(changeBtDeviceName()));
    
    mDeviceCategory=0;
    mDeviceCategory = qobject_cast<HbLabel *>( mLoader->findWidget( "deviceCategory" ) );  
    BTUI_ASSERT_X( mDeviceCategory != 0, "bt-device-view", "Device Category not found" );
    
    mDeviceStatus=0;
    mDeviceStatus = qobject_cast<HbLabel *>( mLoader->findWidget( "deviceStatus" ) );  
    BTUI_ASSERT_X( mDeviceStatus != 0, "bt-device-view", "Device status not found" );
    
    
    mCombobox = 0;
    mCombobox = qobject_cast<HbComboBox *>( mLoader->findWidget( "connectionCombobox" ) );
    BTUI_ASSERT_X( mCombobox != 0, "bt-device-view", "connection combobox not found" );
    
    ret = connect(mCombobox, SIGNAL(currentIndexChanged (int)), 
                this, SLOT(connectionPreferenceChanged(int)));
    BTUI_ASSERT_X( ret, "Btui, BtcpuiDeviceView::BtcpuiDeviceView", "currentIndexChanged() connect failed");
        
    mPair_Unpair=0;
    mPair_Unpair = qobject_cast<HbPushButton *>( mLoader->findWidget( "devicePairUnpair" ) );
    BTUI_ASSERT_X( mPair_Unpair != 0, "bt-device-view", "pair/unpair button not found" );
 
    mConnect_Disconnect=0;
    mConnect_Disconnect = qobject_cast<HbPushButton *>( mLoader->findWidget( "deviceConnectDisconnect" ) );
    BTUI_ASSERT_X( mConnect_Disconnect != 0, "bt-device-view", "connect/disconnect button not found" );
      
    mDeviceSetting = 0;
    mDeviceSetting = qobject_cast<HbPushButton *>( mLoader->findWidget( "deviceSettings" ) );
    BTUI_ASSERT_X( mDeviceSetting != 0, "bt-device-view", "settings button not found" );
    ret = connect(mDeviceSetting, SIGNAL(clicked()), this,
            SLOT(handleDeviceSetting()));
    BTUI_ASSERT_X( ret, "Btui, BtcpuiDeviceView::BtcpuiDeviceView", "clicked() connect failed");


}

BtcpuiDeviceView::~BtcpuiDeviceView()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    delete mLoader; // Also deletes all widgets that it constructed.
    delete mAbstractDelegate;
    unloadDeviceDetails();
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::backToPreviousView()
{
    BtcpuiBaseView::backToPreviousView();
    unloadDeviceDetails();
}

void BtcpuiDeviceView::loadDeviceDetails()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    bool ret(false);
    unloadDeviceDetails();  // ToDo: is it necessary to delete the device view every time there is a change?
    mDeviceDetail = new BtCpUiDeviceDetail();
    ret=connect(mDeviceDetail, SIGNAL(deviceSettingsChanged(bool)),
           this, SLOT(handleDeviceSettingsChange(bool)));
    BTUI_ASSERT_X( ret, "Btui, BtcpuiDeviceView::loadDeviceDetails", "deviceSettingsChanged() connect failed");

    QString btAddr = mDeviceBdAddr.toString();
    QString deviceName = mDeviceName->text();
    mDeviceDetail->loadDeviceDetailPlugins(btAddr, deviceName);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::unloadDeviceDetails()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if(mDeviceDetail) {
        mDeviceDetail->sendCloseEvent();
        delete mDeviceDetail;
        mDeviceDetail = 0;
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::activateView( const QVariant& value, bool backNavi)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    Q_UNUSED( backNavi ); 
    // get bluetooth address of device in question
    QModelIndex index = value.value<QModelIndex>();
    mDeviceBdAddr = (mDeviceModel->data(index, BtDeviceModel::ReadableBdaddrRole));
    QModelIndex start = mDeviceModel->index(0,0);
    QModelIndexList indexList = mDeviceModel->match(start,BtDeviceModel::ReadableBdaddrRole, mDeviceBdAddr);
    mDeviceIndex = indexList.at(0);
    
    bool ret = false;
    if (mMainWindow->orientation() == Qt::Horizontal) {
        //mLoader->load(BTUI_DEVICEVIEW_DOCML, "landscape", &ret);
        //BTUI_ASSERT_X( ret, "bt-device-view", "Invalid docml file: landscape section problem" );
        int majorRole = (mDeviceModel->data(mDeviceIndex,BtDeviceModel::MajorPropertyRole)).toInt();
        if (majorRole & BtuiDevProperty::Connectable ) {
            mLoader->load( BTUI_DEVICEVIEW_DOCML, "landscape", &ret );
            BTUI_ASSERT_X( ret, "bt-device-view", "Invalid docml file: landscape section problem" );  
        }
        else{
            mLoader->load( BTUI_DEVICEVIEW_DOCML, "landscape2", &ret );
            BTUI_ASSERT_X( ret, "bt-device-view", "Invalid docml file: landscape2 section problem" );  
        }
    }
    else {
        mLoader->load(BTUI_DEVICEVIEW_DOCML, "portrait", &ret);
        BTUI_ASSERT_X( ret, "bt-device-view", "Invalid docml file: portrait section problem" );
            
              
    }
    
    updateDeviceData();
    
    mDeviceSetting->setVisible(false);
    loadDeviceDetails();    
    ret = false;
    ret=connect(mDeviceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           this, SLOT(updateDeviceData()));
    BTUI_ASSERT_X( ret, "Btui, BtCpUiDeviceView::activateView", "dataChanged() connect failed");
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::deactivateView()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    disconnect(mDeviceModel, 0, this, 0);
    unloadDeviceDetails();
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::handleDeviceSettingsChange(bool status)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mDeviceSetting->setVisible(status);
    mDeviceSetting->setStretched(true);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::handleDeviceSetting()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if(mDeviceDetail) {
        mDeviceDetail->loadDeviceDetailsView();
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}


// called due to real orientation change event coming from main window
void BtcpuiDeviceView::changeOrientation( Qt::Orientation orientation )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    bool ok = false;
    int majorRole = (mDeviceModel->data(mDeviceIndex,BtDeviceModel::MajorPropertyRole)).toInt();
    if( orientation == Qt::Vertical ) {
        // load "portrait" section
        mLoader->load( BTUI_DEVICEVIEW_DOCML, "portrait", &ok );
        BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file: portrait section problem" );
    } else {
        // load "landscape" section
        //mLoader->load( BTUI_DEVICEVIEW_DOCML, "landscape", &ok );
        //BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file: landscape section problem" );
        if (majorRole & BtuiDevProperty::Connectable ) {
            mLoader->load( BTUI_DEVICEVIEW_DOCML, "landscape", &ok );
            BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file: landscape section problem" );  
        }
        else{
            mLoader->load( BTUI_DEVICEVIEW_DOCML, "landscape2", &ok );
            BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file: landscape2 section problem" );  
        }
    }
    setTextAndVisibilityOfButtons(majorRole);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::clearViewData()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mDeviceIcon->clear();
    mDeviceCategory->clear();
    mDeviceStatus->clear();
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::updateDeviceData()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    clearViewData();
    //Get the QModelIndex of the device using the device BDAddress
    QModelIndex start = mDeviceModel->index(0,0);
    QModelIndexList indexList = mDeviceModel->match(start,BtDeviceModel::ReadableBdaddrRole, mDeviceBdAddr);
    mDeviceIndex = indexList.at(0);
    
    //populate device view with device data fetched from UiModel
    QString deviceName = (mDeviceModel->data(mDeviceIndex, 
             BtDeviceModel::NameAliasRole)).toString(); 
    mDeviceName->setText(deviceName);
     
    int cod = (mDeviceModel->data(mDeviceIndex,BtDeviceModel::CoDRole)).toInt();
     
    int majorRole = (mDeviceModel->data(mDeviceIndex,BtDeviceModel::MajorPropertyRole)).toInt();
    
	setDeviceCategory(cod, majorRole);
    setDeviceStatus(majorRole);
    setTextAndVisibilityOfButtons(majorRole);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::setDeviceCategory(int cod,int majorRole)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mDeviceCategory->setPlainText( getDeviceTypeString( cod ));
    HbIcon icon =
    getBadgedDeviceTypeIcon(cod, majorRole,
                            BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight );
    mDeviceIcon->setIcon(icon);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::setDeviceStatus(int majorRole)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QString deviceStatus;
    bool ret = false;
    
    // Avoid unnecessary actions:
    // The device model has been modified and need to be reflected
    // in the UI however if we don't disable the signal before the UI update
    // the connectionPreferenceChanged function will be called and trigger
    // unnecessary update of the model by the device view.
    ret = disconnect(mCombobox, SIGNAL(currentIndexChanged (int)), 
            this, SLOT(connectionPreferenceChanged(int)));
    BTUI_ASSERT_X( ret, "Btui, BtcpuiDeviceView::setDeviceStatus", "currentIndexChanged() disconnect failed");
    
    if (majorRole & BtuiDevProperty::Bonded && 
        majorRole & BtuiDevProperty::Trusted &&
        majorRole & BtuiDevProperty::Connected ) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_paired_trused_connected"));
        mCombobox->setCurrentIndex(ConnectionAutomatic );
        mComboboxIndex = ConnectionAutomatic;
    } 
    else if (majorRole & BtuiDevProperty::Bonded && 
             majorRole & BtuiDevProperty::Connected ) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_paired_connected"));
        mCombobox->setCurrentIndex(ConnectionAlwaysAsk );
        mComboboxIndex = ConnectionAlwaysAsk;
    }
    else if (majorRole & BtuiDevProperty::Bonded && 
             majorRole & BtuiDevProperty::Trusted ) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_paired_trusted"));
        mCombobox->setCurrentIndex(ConnectionAutomatic );
        mComboboxIndex = ConnectionAutomatic;
    } 
    else if (majorRole & BtuiDevProperty::Connected && 
             majorRole & BtuiDevProperty::Trusted ) {
        mDeviceStatus->setPlainText(hbTrId("Trusted, Connected"));  // ToDo: missing textId!
        mCombobox->setCurrentIndex(ConnectionAutomatic );
        mComboboxIndex = ConnectionAutomatic;
    } 
    else if (majorRole & BtuiDevProperty::Trusted ) {
        mDeviceStatus->setPlainText(hbTrId("Trusted"));  // ToDo: missing textId!
        mCombobox->setCurrentIndex(ConnectionAutomatic );
        mComboboxIndex = ConnectionAutomatic;
    } 
    else if (majorRole & BtuiDevProperty::Bonded) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_paired"));
        mCombobox->setCurrentIndex(ConnectionAlwaysAsk );
        mComboboxIndex = ConnectionAlwaysAsk;
    }
    else if (majorRole & BtuiDevProperty::Connected) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_connected"));
        mCombobox->setCurrentIndex(ConnectionAlwaysAsk );
        mComboboxIndex = ConnectionAlwaysAsk;
    }
    else if (majorRole & BtuiDevProperty::Blocked) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_blocked"));
        mCombobox->setCurrentIndex(ConnectionBlocked );
        mComboboxIndex = ConnectionBlocked;
    }
    else {
        // device not paired, connected, trusted or blocked.  
        mDeviceStatus->setPlainText(" ");
        mCombobox->setCurrentIndex(ConnectionAlwaysAsk );
        mComboboxIndex = ConnectionAlwaysAsk;
    }
    ret = connect(mCombobox, SIGNAL(currentIndexChanged (int)), 
            this, SLOT(connectionPreferenceChanged(int)));
    BTUI_ASSERT_X( ret, "Btui, BtcpuiDeviceView::setDeviceStatus", "currentIndexChanged() connect failed");
    
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::setTextAndVisibilityOfButtons(int majorProperty)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    disconnect(mPair_Unpair, 0, 0, 0);
    disconnect(mConnect_Disconnect, 0, 0, 0);
    
    if (majorProperty & BtuiDevProperty::Blocked) {
        mPair_Unpair->setVisible(false);
        mConnect_Disconnect->setVisible(false);
        mDeviceSetting->setVisible(false);
        return;
    }
    bool ret;
    mPair_Unpair->setVisible(true);
    mPair_Unpair->setStretched(true);
    if (majorProperty & BtuiDevProperty::Bonded) {
        updateButton(mPair_Unpair, "qtg_mono_bt_unpair", hbTrId("txt_bt_button_unpair"));
        ret =  connect(mPair_Unpair, SIGNAL(clicked()), this, SLOT(unpairDevice()));
        BTUI_ASSERT_X( ret, "BtcpuiDeviceView::BtcpuiDeviceView", "can't connect unpair button" );
    }
    else {
        updateButton(mPair_Unpair, "qtg_mono_bt_pair", hbTrId("txt_bt_button_pair"));
        ret =  connect(mPair_Unpair, SIGNAL(clicked()), this, SLOT(pairDevice()));
        BTUI_ASSERT_X( ret, "BtcpuiDeviceView::BtcpuiDeviceView", "can't connect pair button" );        
    }
    
    if (majorProperty & BtuiDevProperty::Connectable ) {
        mConnect_Disconnect->setVisible(true);
        mConnect_Disconnect->setStretched(true);
        if (majorProperty & BtuiDevProperty::Connected) {
            updateButton(mConnect_Disconnect, "qtg_mono_speaker_off", hbTrId("txt_bt_button_disconnect"));
            ret =  connect(mConnect_Disconnect, SIGNAL(clicked()), this, SLOT(disconnectDevice()));
            BTUI_ASSERT_X( ret, "BtcpuiDeviceView::BtcpuiDeviceView", "can't connect disconnect button" );
        }
        else {
            updateButton(mConnect_Disconnect, "qtg_mono_speaker", hbTrId("txt_bt_button_connect"));
            ret =  connect(mConnect_Disconnect, SIGNAL(clicked()), this, SLOT(connectDevice()));
            BTUI_ASSERT_X( ret, "BtcpuiDeviceView::BtcpuiDeviceView", "can't connect connect button" );
        }
    }
    else {
        //it is not possible to connect, set the button invisible
        mConnect_Disconnect->setVisible(false);
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::connectionPreferenceChanged(int index)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if ((index >= ConnectionAutomatic)&&(index != mComboboxIndex)) {
        switch ( index )
        {
        case ConnectionAutomatic:
            setDeviceAuthorised();
            break;
        case ConnectionAlwaysAsk:
            setDeviceAlwaysAsk();
            break;
        case ConnectionBlocked:
            setDeviceBlocked();
            break;
        default: 
            BTUI_ASSERT_X( 0, "BtCpUiDeviceView::connectionPreferenceChanged()", "wrong index" );
        }
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::setDeviceAuthorised()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (!mAbstractDelegate){
        //if there is no other delegate running
        QList<QVariant>list;
        list.append(mDeviceBdAddr);
        
        if (mComboboxIndex != ConnectionAutomatic) { // transition: (always ask|blocked) --> automatic
            // unblock the device
            list.append(QVariant(BtAuthorize));
        }
        else {
            // do nothing, state is already correct
            return;
        }
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::TrustDevice, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), 
                this, SLOT(unpairDelegateCompleted(int)) );
        mAbstractDelegate->exec(QVariant(list));
    }   
    BOstraceFunctionExit0(DUMMY_DEVLIST);  
}

void BtcpuiDeviceView::setDeviceAlwaysAsk()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (!mAbstractDelegate){
        //if there is no other delegate running
        QList<QVariant>list;
        BtDelegate::EditorType type;
        list.append(mDeviceBdAddr);
        if (mComboboxIndex == ConnectionAutomatic) { // transition: automatic --> always ask
            // unauthorize the device
            list.append(QVariant(BtUnauthorize));
            type = BtDelegate::UntrustDevice;
        }
        else if (mComboboxIndex == ConnectionBlocked) { // transition: blocked --> always ask 
            // unblock the device
            list.append(QVariant(BtUnblock));
            type = BtDelegate::UnblockDevice;
        }
        else {
            // do nothing, state is already correct
            return;
        }
        
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                type, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), 
                this, SLOT(unpairDelegateCompleted(int)) );
        mAbstractDelegate->exec(QVariant(list));
    }   
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::setDeviceBlocked()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    //simple version of the delegate
    //TODO: disconnect after blocking
    if (!mAbstractDelegate){
        //if there is no other delegate running
        QList<QVariant> list;
        list.append(mDeviceBdAddr);
        
        if ( mComboboxIndex == ConnectionAutomatic || 
            mComboboxIndex == ConnectionAlwaysAsk ) {  // transition: automatic|always ask --> blocked
            // to unauthorized the device
            list.append(QVariant(BtBlock));
        }
        else{
            // do nothing, state is already correct
            return;
        }
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::BlockDevice, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), 
                this, SLOT(unpairDelegateCompleted(int)) );
        mAbstractDelegate->exec(QVariant(list));
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::pairDevice()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        QVariant params;
        params.setValue(mDeviceIndex);
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::PairDevice, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), this, SLOT(pairDelegateCompleted(int)) );
        mAbstractDelegate->exec(params);
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::pairDelegateCompleted(int status)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, status );
    Q_UNUSED(status);
    //TODO: handle the error here
    if (mAbstractDelegate)
    {
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::unpairDevice()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        QList<QVariant>list;
        list.append(mDeviceBdAddr);
        list.append(QVariant(BtUnpair));
        
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::UnpairDevice, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), this, 
                SLOT(unpairDelegateCompleted(int)) );
        mAbstractDelegate->exec(QVariant(list));
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST); 
}

void BtcpuiDeviceView::unpairDelegateCompleted(int status)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    Q_UNUSED(status);
    //TODO: handle the error here 
    if (mAbstractDelegate)
    {
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::connectDevice()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );

    if (!mAbstractDelegate) { //if there is no other delegate running
        QVariant params;
        params.setValue(mDeviceIndex);
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::ConnectService, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), this, SLOT(connectDelegateCompleted(int)) );
        mAbstractDelegate->exec(params);
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);       
}

void BtcpuiDeviceView::connectDelegateCompleted(int status)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    Q_UNUSED(status);
    if (mAbstractDelegate) {
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }   
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::disconnectDevice()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (!mAbstractDelegate) { //if there is no other delegate running 
        QList<QVariant>list;
        list.append(QVariant(ServiceLevel));
        list.append(mDeviceBdAddr);
        
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::DisconnectService, mSettingModel, mDeviceModel); 
        // todo: check return value of connect
        connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), this, 
                SLOT(disconnectDelegateCompleted(int)) );
        mAbstractDelegate->exec(QVariant(list));
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::disconnectDelegateCompleted(int status)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    Q_UNUSED(status);
    if (mAbstractDelegate)
    {
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::setPrevBtDeviceName()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QString deviceName = (mDeviceModel->data(mDeviceIndex, 
                BtDeviceModel::NameAliasRole)).toString(); 
   mDeviceName->setText(deviceName);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::changeBtDeviceName()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (!mAbstractDelegate){
        //if there is no other delegate running
        QList<QVariant>list;
        
        QVariant index;
        index.setValue(mDeviceIndex);
        
        QVariant name;
        name.setValue(mDeviceName->text());
        
        list.append(index);
        list.append(name);
        
        QVariant params;
        params.setValue(list);
        
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::ChangeDeviceFriendlyName, mSettingModel, mDeviceModel);
        
        bool ok(false);
        ok =connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), 
                this, SLOT(changeDevNameDelegateCompleted(int)) );
        BTUI_ASSERT_X( ok, "BtcpuiDeviceView", "changeDevName: fail to connect signal" );
        
		mAbstractDelegate->exec(params);
    }
    else {
        setPrevBtDeviceName();
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::changeDevNameDelegateCompleted(int status)
{   
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if(KErrNone != status) {
        setPrevBtDeviceName();
    }
 
    if (mAbstractDelegate){
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }   
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiDeviceView::updateButton(HbPushButton *button, const QString &iconName, const QString &text)
{
    HbIcon icon(iconName);
    icon.setIconName(iconName);
    button->setIcon(icon);
    button->setText(text);
}
