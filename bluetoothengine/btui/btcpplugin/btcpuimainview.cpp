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
* Description:  BtcpuiMainView implementation
*
*/

#include "btcpuimainview.h"
#include "btuiviewutil.h"
#include <QtGlobal>
#include <QGraphicsLinearLayout>
#include <HbInstance>
#include <hbdocumentloader.h>
#include <hbnotificationdialog.h>
#include <hbgridview.h>
#include <hblistview.h>
#include <hbpushbutton.h>
#include <hblabel.h>
#include <hbicon.h>
#include <hblineedit.h>
#include <hbtooltip.h>
#include <btengconstants.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbcombobox.h>
#include <hbgroupbox.h>
#include <hbdataform.h>
#include <HbStyleLoader>
#include <bluetoothuitrace.h>
#include <HbMessageBox>
#include <btabstractdelegate.h>
#include "btqtconstants.h"
#include "btcpuimaingridviewitem.h"
#include "btuidevtypemap.h"
#include "btcpuiviewmgr.h"
#include "btcpuisearchview.h"
#include "btcpuideviceview.h"

// docml to load
const char* BTUI_MAINVIEW_DOCML = ":/docml/bt-main-view.docml";

/*!
    Constructs a new BtUiMainView using HBDocumentLoader.  Docml (basically xml) file
    has been generated using Application Designer.
 */
BtcpuiMainView::BtcpuiMainView(QGraphicsItem *parent )
    : BtcpuiBaseView(parent), mSearchView(0), mDeviceView(0)
{
    loadDocument();
    setViewMgr(this);
}

/*!
    Constructs a new BtUiMainView using HBDocumentLoader.  Docml (basically xml) file
    has been generated using Application Designer.
 */
BtcpuiMainView::BtcpuiMainView(BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel, 
        QGraphicsItem *parent )
    : BtcpuiBaseView(settingModel, deviceModel, parent ), mSearchView(0), mDeviceView(0)
{
    loadDocument();
    setViewMgr(this);    
}

/*!
    Destructs the BtcpuiMainView.
 */
BtcpuiMainView::~BtcpuiMainView()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    delete mSearchView;
    delete mDeviceView;
    delete mLoader; // Also deletes all widgets that it constructed.
	HbStyleLoader::unregisterFilePath(":/docml/btcpuimaingridviewitem.widgetml");
    HbStyleLoader::unregisterFilePath(":/docml/btcpuimaingridviewitem.css");
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

/*! 
    from base class, initialize the view
 */
void BtcpuiMainView::activateView(const QVariant& value, bool backNavi)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    Q_UNUSED(value);
    Q_UNUSED(backNavi);
    QModelIndex top = mSettingModel->index( BtSettingModel::LocalBtNameRow, 0 );
    QModelIndex bottom = mSettingModel->index( BtSettingModel::AllowedInOfflineRow, 0 );
    // update name, power and visibility rows
    updateSettingItems( top, bottom );
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

/*!
    From base class. Handle resource before the current view is deactivated.
 */
void BtcpuiMainView::deactivateView()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::launchDeviceDiscovery()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    // Construct the search view if it does not exist yet.
    (void) searchView();
    bool ok = createDelegate(BtDelegate::Inquiry, 
            this, SLOT(handleDelegateCompleted(int,BtAbstractDelegate*)));
    if (ok) {
        // Connect to the signal from BtDelegateInquiry for switching to 
        // search view when BT inquiry operation is really started.
        ok = connect(mDelegate, SIGNAL(beginUiEditMode()), this, SLOT(goToSearchView()));
        BTUI_ASSERT_X( ok, "BtcpuiMainView", "launchDeviceDiscovery() can't connect" );
    }
    if (ok) {
        mDelegate->exec(QVariant());
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::goToSearchView()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    BtcpuiBaseView *search = searchView();
    // For navigating back to this view
    search->setPreviousView( this );
    search->take(mDelegate);
    mDelegate = 0;
    switchView(this, search, QVariant(), false);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::changeBtLocalName()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    bool ok = createExecuteDelegate(BtDelegate::ChangeLocalName, 
            this, SLOT(btNameDelegateCompleted(int)), mDeviceNameEdit->text());
    if (!ok) {
        setPrevBtLocalName();
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::setPrevBtLocalName()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QModelIndex index = mSettingModel->index( BtSettingModel::LocalBtNameRow,0 );
    
    mDeviceNameEdit->setText( mSettingModel->data(
            index,BtSettingModel::settingDisplayRole).toString() );
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::btNameDelegateCompleted(int status)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if(KErrNone != status) {
        setPrevBtLocalName();
    }
    handleDelegateCompleted(status, mDelegate);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::removeRegistryDevices(BtDelegate::EditorType type)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    
    if (type != BtDelegate::RemoveBlockedDevices) {
        //Before removing devices from the list call disconnect delegate
        //to disconnect all active connections, this is done if all/paired
        //devices are removed.   
        QList<QVariant> list;
        list.append(QVariant( AllOngoingConnections ));
        
        (void) createExecuteDelegate(BtDelegate::DisconnectAllConnections, 
                    this, SLOT(disconnectAllDelegateCompleted(int)), list);
    }
    else {
        //In case of Blocked devices, just remove devices from list.
        (void) createExecuteDelegate(type, 
                this, SLOT(handleDelegateCompleted(int,BtAbstractDelegate*)), type);
    }
    
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::disconnectAllDelegateCompleted(int status)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    handleDelegateCompleted(status, mDelegate);
    
    //Remove Device list from registry after disconnect is done.
    (void) createExecuteDelegate(mRemoveDevDelegateType, 
            this, SLOT(handleDelegateCompleted(int,BtAbstractDelegate*)), mRemoveDevDelegateType);

    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::changeVisibility(int index)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QList<QVariant> list;
    
    VisibilityMode mode = indexToVisibilityMode(index);
    list.append(QVariant((int)mode));
    if( BtTemporary == VisibilityMode(mode) ) {
        //Right now hardcoded to 5 Mins.
        list.append(QVariant(5));
    }
    bool ok = createExecuteDelegate(BtDelegate::ChangeVisibility, 
            this, SLOT(visibilityDelegateCompleted(int)), list);
    //Error handling has to be done.    
    if ( !ok ) {
        setPrevVisibilityMode();
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::setPrevVisibilityMode()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QModelIndex index = mSettingModel->index( BtSettingModel::VisibilityRow, 0 );
    
    mVisibilityMode->setCurrentIndex ( visibilityModeToIndex((VisibilityMode)
                mSettingModel->data(index,BtSettingModel::SettingValueRole).toInt()) );
    BOstraceFunctionExit0(DUMMY_DEVLIST);   
}


void BtcpuiMainView::allActionTriggered()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
   
    updateDeviceListFilter(BtuiAll);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::pairActionTriggered()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );

    updateDeviceListFilter(BtuiPaired);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::aboutToShowOptionsMenu()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    
    updateOptionsMenu();
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::updateOptionsMenu()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    
    if(mPairAction->isEnabled()) {
        //All Toolbar Action is active
        mRemovePairedDevices->setVisible(false);
        
        int devCount = mDeviceModel->deviceCount(BtuiDevProperty::InRegistry);      
        bool visibility = false;

        HbAction *subMenuAction = static_cast<HbAction*> (mLoader->findObject(
                "showAction"));

        if(!devCount) {
            //No Registry devices then return.
            mSubMenu->menuAction()->setVisible(false);
            subMenuAction->setVisible(false);
            return;
        }
        
        visibility = true;           
        mSubMenu->menuAction()->setVisible(visibility);
        subMenuAction->setVisible(visibility);
        
        subMenuAction = static_cast<HbAction*> (mLoader->findObject(
                "removeAllAction"));
        subMenuAction->setVisible(visibility);
        
        devCount = mDeviceModel->deviceCount(BtuiDevProperty::Bonded);
        
        visibility = false;   
        if(devCount) {
            visibility = true;           
        }
        subMenuAction = static_cast<HbAction*> (mLoader->findObject(
                "removePairedAction"));
        subMenuAction->setVisible(visibility);
        
        devCount = mDeviceModel->deviceCount(BtuiDevProperty::Blocked);
        
        visibility = false;   
        if(devCount) {
            visibility = true;           
        }
        subMenuAction = static_cast<HbAction*> (mLoader->findObject(
                "removeBlockedAction"));
        subMenuAction->setVisible(visibility);

    }
    else {
        //Pair Toolbar Action is active
        mSubMenu->menuAction()->setVisible(false);
        
        int devCount = mDeviceModel->deviceCount(BtuiDevProperty::Bonded);
        bool visibility = false;
        
        HbAction *subMenuAction = static_cast<HbAction*> (mLoader->findObject(
                "showAction"));

        if(devCount) {
            visibility = true;           
        }
        mRemovePairedDevices->setVisible(visibility);
        subMenuAction->setVisible(visibility);
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::menuActionTriggered(HbAction *action)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );

    if(!(action->text().compare(hbTrId("txt_bt_opt_advanced_settings")))) {
        //Advance Settings is not implemented yet.
        return;
    }
    
    if(!(action->text().compare(hbTrId("txt_bt_opt_show")))) {
        viewByDeviceTypeDialog();
    }
    else {
        QString dialogtext;
        
        if(!(action->text().compare(hbTrId("txt_bt_opt_remove_sub_all_devices")))) {
            mRemoveDevDelegateType = BtDelegate::RemoveAllRegistryDevices;
            dialogtext = hbTrId("txt_bt_info_delete_all_devices_some_devices_may_b");
        }
        else if(!(action->text().compare(hbTrId("txt_bt_opt_remove_sub_blocked_devices")))) {
            mRemoveDevDelegateType = BtDelegate::RemoveBlockedDevices;
            dialogtext = hbTrId("txt_bt_info_delete_all_blocked_devices");
        }
        else {
            mRemoveDevDelegateType = BtDelegate::RemovePairedDevices;
            dialogtext = hbTrId("txt_bt_info_delete_all_paired_devices_some_device");
        }

        HbMessageBox* dialog = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
        dialog->setText(dialogtext);
        dialog->setStandardButtons(HbMessageBox::Yes | HbMessageBox::No);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        
        dialog->open(this,SLOT(onRemoveQuestionDialogClosed(int)));

    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::onRemoveQuestionDialogClosed(int action)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    
    if (action == HbMessageBox::Yes) {
        removeRegistryDevices(mRemoveDevDelegateType);
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

/*!
   Callback for HbSelectionDialog closing
   ReImplemented Slot from Base class

 */
void BtcpuiMainView::viewByDialogClosed(HbAction* action)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    
    int devTypesWanted = 0;
    
    devTypesWanted = selectedDeviceTypes(action);
    
    if (devTypesWanted) {
        mBtuiModelSortFilter->clearDeviceMajorFilters();
        if(mAllAction->isEnabled()) {
            mBtuiModelSortFilter->addDeviceMajorFilter(
                    BtuiDevProperty::InRegistry | BtuiDevProperty::Bonded, 
                    BtuiModelSortFilter::AtLeastMatch);
        }
        else {
            mBtuiModelSortFilter->addDeviceMajorFilter(
                    BtuiDevProperty::InRegistry, 
                    BtuiModelSortFilter::AtLeastMatch);
        }
        mBtuiModelSortFilter->addDeviceMajorFilter(devTypesWanted,
                BtuiModelSortFilter::RoughMatch);             // device can be any one of selected ones
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

BtcpuiBaseView *BtcpuiMainView::searchView()
{
    if (!mSearchView) {
        mSearchView = new BtcpuiSearchView(*mSettingModel, *mDeviceModel);
        mSearchView->setViewMgr(this);
        mMainWindow->addView(mSearchView);
    }
    return mSearchView;
}

BtcpuiBaseView *BtcpuiMainView::deviceView()
{
    if (!mDeviceView) {
        mDeviceView = new BtcpuiDeviceView(*mSettingModel, *mDeviceModel);
        mDeviceView->setViewMgr(this);
        mMainWindow->addView(mDeviceView);
    }
    return mDeviceView;
}

void BtcpuiMainView::switchView(
        BtcpuiBaseView *from, BtcpuiBaseView *destination,
        const QVariant &init, bool backNavi)
{
    BTUI_ASSERT_X( from && destination, "BtcpuiSettingItem::switchView", "null view" );
    from->deactivateView();
    mMainWindow->setCurrentView( destination );
    destination->activateView( init, backNavi);
}

void BtcpuiMainView::updateDeviceListFilter(BtcpuiMainView::filterType filter)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mBtuiModelSortFilter->clearDeviceMajorFilters();
    
    switch (filter) {
        case BtuiAll:
            mDataForm->setHeading(hbTrId("txt_bt_subhead_bluetooth_all_devices"));
            mPairAction->setEnabled(true);
            mAllAction->setEnabled(false);
            mBtuiModelSortFilter->addDeviceMajorFilter(
                    BtuiDevProperty::InRegistry, 
                    BtuiModelSortFilter::AtLeastMatch);

            break;
        case BtuiPaired:
            mDataForm->setHeading(hbTrId("txt_bt_subhead_bluetooth_paired_devices"));
            mPairAction->setEnabled(false);
            mAllAction->setEnabled(true);
            mBtuiModelSortFilter->addDeviceMajorFilter(
                    BtuiDevProperty::InRegistry | BtuiDevProperty::Bonded, 
                    BtuiModelSortFilter::AtLeastMatch);

            break;
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiMainView::visibilityDelegateCompleted(int status)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    //This should be mapped to Qt error
    if(KErrNone != status) {
        setPrevVisibilityMode();
    }
    handleDelegateCompleted(status, mDelegate);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}


// called due to real orientation change event coming from main window
void BtcpuiMainView::changeOrientation( Qt::Orientation orientation )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    bool ok = false;
    mOrientation = orientation;
    if( orientation == Qt::Vertical ) {
        // load "portrait" section
        mLoader->load( BTUI_MAINVIEW_DOCML, "portrait", &ok );
        BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file: portrait section problem" );
        
        mDeviceList->setColumnCount(1);
        mDeviceList->setRowCount(4);
        mDeviceList->setScrollDirections(Qt::Vertical);
        mDeviceList->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded );
    } else {
        // load "landscape" section
        mLoader->load( BTUI_MAINVIEW_DOCML, "landscape", &ok );
        BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file: landscape section problem" );
        
        mDeviceList->setRowCount(1);
        mDeviceList->setColumnCount(6);
        mDeviceList->setScrollDirections(Qt::Horizontal);
        mDeviceList->setHorizontalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded );
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

/*!
    Slot for receiving notification of local setting changes from the model.
    Identify the setting changed and update the corresponding UI item.
 */
void BtcpuiMainView::updateSettingItems(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{   
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    bool val(false);
    
    // update only the part of the view specified by the model's row(s)
    for (int i=topLeft.row(); i <= bottomRight.row(); i++) {
        QModelIndex index = mSettingModel->index( i, 0);
        // Distinguish which setting value is changed.
        switch ( i ) {
        case BtSettingModel::LocalBtNameRow :
            mDeviceNameEdit->setText( 
                    mSettingModel->data(index,BtSettingModel::settingDisplayRole).toString() );
            break;
        case BtSettingModel::PowerStateRow:
            val = mSettingModel->data(index, BtSettingModel::SettingValueRole).toBool();
            if (val) {
                HbIcon icon("qtg_mono_bluetooth");
                icon.setIconName("qtg_mono_bluetooth");
                mPowerButton->setIcon(icon);  
            }
            else {
                HbIcon icon("qtg_mono_bluetooth_off");
                icon.setIconName("qtg_mono_bluetooth_off");
                mPowerButton->setIcon(icon);
            }
            break;
        case BtSettingModel::VisibilityRow:
            mVisibilityMode->setCurrentIndex ( visibilityModeToIndex((VisibilityMode)
                    mSettingModel->data(index,BtSettingModel::SettingValueRole).toInt()) );
            break;
        }
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

/*!
    Slot for receiving notification for user interaction on power state.
    Manually update model data since HbPushButton is not linked to model directly.
 */
void BtcpuiMainView::changePowerState()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QModelIndex index = mSettingModel->index(BtSettingModel::PowerStateRow, 0);
    PowerStateQtValue powerState = (PowerStateQtValue)mSettingModel->data(index, Qt::EditRole).toInt();
    BTUI_ASSERT_X(((powerState == BtPowerOn) || (powerState == BtPowerOff)), 
            "BtcpuiMainView::changePowerState()", "incorrect qt power state");

    if (powerState == BtPowerOff) {
        powerState = BtPowerOn;
    }
    else {
        powerState = BtPowerOff;
    } 
    (void) createExecuteDelegate(BtDelegate::ManagePower, 
            this, SLOT(handleDelegateCompleted(int,BtAbstractDelegate*)), 
            QVariant((int)powerState));
    BOstraceFunctionExit0(DUMMY_DEVLIST); 
}

void BtcpuiMainView::loadDocument()
{
    bool ret(false);
    // Create view for the application.
    // Set the name for the view. The name should be same as the view's
    // name in docml.
    setObjectName("bt_main_view");

    mLoader = new HbDocumentLoader();
    // Pass the view to documentloader. Document loader uses this view
    // when docml is parsed, instead of creating new view.
    QObjectList objectList;
    objectList.append(this);
    mLoader->setObjectTree(objectList);

    bool ok = false;
    mLoader->load( BTUI_MAINVIEW_DOCML, &ok );
    // Exit if the file format is invalid
    BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file" );
    
    mDeviceList=0;
    mDeviceList = qobject_cast<HbGridView *>( mLoader->findWidget( "deviceList" ) );
    BTUI_ASSERT_X( mDeviceList != 0, "bt-main-view", "Device List (grid view) not found" );   
    
    ret = connect(mDeviceList, SIGNAL(activated(QModelIndex)), this, SLOT(openDeviceView(QModelIndex)));
    BTUI_ASSERT_X( ret, "bt-main-view", "openDeviceView can't connect" ); 

    ret = connect(mDeviceList,
            SIGNAL(longPressed(HbAbstractViewItem*, QPointF)), this,
            SLOT(showContextMenu(HbAbstractViewItem*, QPointF)));
    BTUI_ASSERT_X( ret, "bt-main-view", "showContextMenu can't connect" ); 

    mOrientation = mMainWindow->orientation();
    
    mDeviceList->setSwapDimensionsOnOrientationChange(false);
    
    if (mOrientation == Qt::Horizontal) {
        mLoader->load(BTUI_MAINVIEW_DOCML, "landscape", &ok);
        BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file: landscape section problem" );
        mDeviceList->setRowCount(1);
        mDeviceList->setColumnCount(6);
        mDeviceList->setScrollDirections(Qt::Horizontal);
        mDeviceList->setHorizontalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded );
    }
    else {
        mLoader->load(BTUI_MAINVIEW_DOCML, "portrait", &ok);
        BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file: landscape section problem" );
        mDeviceList->setColumnCount(1);
        mDeviceList->setRowCount(4);
        mDeviceList->setScrollDirections(Qt::Vertical);
        mDeviceList->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded );
    }

    mDeviceNameEdit=0;
    mDeviceNameEdit = qobject_cast<HbLineEdit *>( mLoader->findWidget( "deviceNameEdit" ) );
    BTUI_ASSERT_X( mDeviceNameEdit != 0, "bt-main-view", "Device Name not found" );
    ret =  connect(mDeviceNameEdit, SIGNAL(editingFinished ()), this, SLOT(changeBtLocalName()));
    
    mPowerButton=0;
    mPowerButton = qobject_cast<HbPushButton *>( mLoader->findWidget( "toggleBluetoothPower" ) );
    BTUI_ASSERT_X( mPowerButton != 0, "bt-main-view", "power button not found" );
    ret =  connect(mPowerButton, SIGNAL(clicked()), this, SLOT(changePowerState()));
    BTUI_ASSERT_X( ret, "BtcpuiMainView::BtcpuiMainView", "can't connect power button" );
    
    mVisibilityMode=0;
    mVisibilityMode = qobject_cast<HbComboBox *>( mLoader->findWidget( "visibilityMode" ) );
    BTUI_ASSERT_X( mVisibilityMode != 0, "bt-main-view", "visibility combobox not found" );
    // add new item for temporary visibility
    // ToDo:  change this to use translation once it starts working
    QString tempVis(hbTrId("txt_bt_setlabel_visibility_val_visible_for_l1_min", 5));  
    //QString tempVis(hbTrId("Visible for 5 min"));  
    mVisibilityMode->addItem(tempVis, Qt::DisplayRole);
        
    // listen for orientation changes
    ret = connect(mMainWindow, SIGNAL(orientationChanged(Qt::Orientation)),
            this, SLOT(changeOrientation(Qt::Orientation)));
    BTUI_ASSERT_X( ret, "BtcpuiMainView::BtcpuiMainView()", "connect orientationChanged() failed");

    // load tool bar actions
    HbAction *discoverAction = static_cast<HbAction*>( mLoader->findObject( "discoverAction" ) );
    BTUI_ASSERT_X( discoverAction, "bt-main-view", "discover action missing" ); 
    ret = connect(discoverAction, SIGNAL(triggered()), this, SLOT(launchDeviceDiscovery()));
    BTUI_ASSERT_X( ret, "bt-main-view", "discover action can't connect" ); 

    // load tool bar actions
    mAllAction = static_cast<HbAction*>( mLoader->findObject( "allAction" ) );
    BTUI_ASSERT_X( mAllAction, "bt-main-view", "All action missing" ); 
    ret = connect(mAllAction, SIGNAL(triggered()), this, SLOT(allActionTriggered()));
    BTUI_ASSERT_X( ret, "bt-main-view", "all action can't connect" ); 

    // load tool bar actions
    mPairAction = static_cast<HbAction*>( mLoader->findObject( "pairedAction" ) );
    BTUI_ASSERT_X( mPairAction, "bt-main-view", "Pair action missing" ); 
    ret = connect(mPairAction, SIGNAL(triggered()), this, SLOT(pairActionTriggered()));
    BTUI_ASSERT_X( ret, "bt-main-view", "pair action can't connect" ); 

    mDataForm = qobject_cast<HbDataForm *>( mLoader->findWidget( "bluetoothHeading" ) );
    BTUI_ASSERT_X( mDataForm != 0, "bt-main-view", "dataForm not found" ); 
    
        
    // load menu
    HbMenu *optionsMenu = qobject_cast<HbMenu *>(mLoader->findWidget("viewMenu"));
    BTUI_ASSERT_X( optionsMenu != 0, "bt-main-view", "Options menu not found" );
    ret = connect(optionsMenu, SIGNAL(triggered(HbAction *)), this, SLOT(menuActionTriggered(HbAction *)));
    BTUI_ASSERT_X( ret, "bt-main-view", "Menu can't connect" );

    ret = connect(optionsMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowOptionsMenu()));
    BTUI_ASSERT_X( ret, "bt-main-view", "Menu aboutToShow() can't connect" );

    mSubMenu = qobject_cast<HbMenu *>(mLoader->findWidget("removeSubMenu"));
    BTUI_ASSERT_X( optionsMenu != 0, "bt-main-view", "Options Sub menu not found" );
    
    mRemovePairedDevices = static_cast<HbAction*>( mLoader->findObject( "viewRemovePairedAction" ) );
            
    // update display when setting data changed
    ret = connect(mSettingModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), 
            this, SLOT(updateSettingItems(QModelIndex,QModelIndex)));
    BTUI_ASSERT_X( ret, "BtcpuiMainView::BtcpuiMainView", "can't connect dataChanged" );
    
    QModelIndex top = mSettingModel->index( BtSettingModel::LocalBtNameRow, 0 );
    QModelIndex bottom = mSettingModel->index( BtSettingModel::AllowedInOfflineRow, 0 );
    // update name, power and visibility rows
    updateSettingItems( top, bottom );

    //Handle Visibility Change User Interaction
    ret = connect(mVisibilityMode, SIGNAL(currentIndexChanged (int)), 
            this, SLOT(changeVisibility(int)));
    
    bool regStatus(false);
    regStatus = HbStyleLoader::registerFilePath(":/docml/btcpuimaingridviewitem.widgetml");
    BTUI_ASSERT_X( regStatus, "BtcpuiMainView::BtcpuiMainView", "registerFilePath Widgetml Failed" );
    regStatus = HbStyleLoader::registerFilePath(":/docml/btcpuimaingridviewitem.css");
    BTUI_ASSERT_X( regStatus, "BtcpuiMainView::BtcpuiMainView", "registerFilePath CSS Failed" );
    
    mBtuiModelSortFilter = new BtuiModelSortFilter(this);
    
    mBtuiModelSortFilter->setSourceModel( mDeviceModel );
    mDeviceList->setModel(mBtuiModelSortFilter);
    mDeviceList->setLayoutName("portrait");
    updateDeviceListFilter(BtuiPaired);
    
    mSubMenu->menuAction()->setVisible(false);
        // Grid view item
    BtCpUiMainGridViewItem *prototype = new BtCpUiMainGridViewItem(mDeviceList);
    prototype->setModelSortFilter(mBtuiModelSortFilter);
    mDeviceList->setItemPrototype(prototype);
}

/*!
 * Mapping from visibility mode UI row to VisibilityMode
 */
VisibilityMode BtcpuiMainView::indexToVisibilityMode(int index)
{
    VisibilityMode mode = BtVisibilityUnknown;
    switch(index) {
    case UiRowBtHidden:  
        mode = BtHidden;
        break;
    case UiRowBtVisible:  
        mode = BtVisible;
        break;
    case UiRowBtTemporary:  
        mode = BtTemporary;
        break;
    default:
        BTUI_ASSERT_X(false, "BtcpuiMainView::indexToVisibilityMode", "invalid mode");
    }
    return mode;
}

/*!
 * Mapping from VisibilityMode to visibility mode UI row  
 */
int BtcpuiMainView::visibilityModeToIndex(VisibilityMode mode)
{
    int uiRow = UiRowBtUnknown;
    switch(mode) {
    case BtHidden:  
        uiRow = UiRowBtHidden;
        break;
    case BtVisible:  
        uiRow = UiRowBtVisible;
        break;
    case BtTemporary:  
        uiRow = UiRowBtTemporary;
        break;
    default:
        BTUI_ASSERT_X(false, "BtcpuiMainView::visibilityModeToIndex", "invalid mode");
    }
    return uiRow;
}

void BtcpuiMainView::createContextMenuActions(int majorRole)
{  
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (majorRole & BtuiDevProperty::Connected) {
        mContextMenu->addAction(hbTrId("txt_bt_menu_disconnect"));
    }
    else {
        mContextMenu->addAction(hbTrId("txt_bt_menu_connect"));
    }  
    BOstraceFunctionExit0(DUMMY_DEVLIST); 
}



