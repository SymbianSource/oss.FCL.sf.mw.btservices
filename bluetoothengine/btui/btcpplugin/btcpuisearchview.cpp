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

#include "btcpuisearchview.h"
#include <HbStyleLoader>
#include <QtGui/QGraphicsLinearLayout>
#include <HbAction>
#include <HbInstance>
#include <HbDocumentLoader>
#include <HbDataForm>
#include <HbPushButton>
#include <HbLabel>
#include <HbListView>
#include <HbMenu>
#include <HbSelectionDialog>
#include <HbGroupBox>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <bluetoothuitrace.h>
#include "btabstractdelegate.h"
#include "btcpuisearchlistviewitem.h"
#include "btuidevtypemap.h"
#include "btuimodelsortfilter.h"
#include "btuiviewutil.h"
#include "btcpuiviewmgr.h"

// docml to load
const char* BTUI_SEARCHVIEW_DOCML = ":/docml/bt-search-view.docml";

BtcpuiSearchView::BtcpuiSearchView(BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel, 
        QGraphicsItem *parent) :
    BtcpuiBaseView(settingModel,deviceModel, parent)
{
    bool ret(false);

    // Create view for the application.
    // Set the name for the view. The name should be same as the view's
    // name in docml.
    setObjectName("bt_search_view");

    mLoader = new HbDocumentLoader();
    BTUI_ASSERT_X( mLoader != 0, "BtcpuiSearchView::BtcpuiSearchView", "can't create mLoader" );
    // Pass the view to documentloader. Document loader uses this view
    // when docml is parsed, instead of creating new view.
    QObjectList objectList;
    objectList.append(this);
    mLoader->setObjectTree(objectList);
    
    // read view info from docml file
    bool ok = false;
    mLoader->load( BTUI_SEARCHVIEW_DOCML, &ok );
    // Exit if the file format is invalid
    BTUI_ASSERT_X( ok, "bt-search-view", "Invalid docml file" );
    
    // Set title for the control panel
    // ToDo:  check if deprecated API
    setTitle("Control Panel");

    // assign automatically created widgets to local variables

    mDeviceIcon=0;
    // can't use qobject_cast since HbIcon is not derived from QObject!
    mDeviceIcon = qobject_cast<HbLabel *>( mLoader->findWidget( "searchIcon" ) );  
    BTUI_ASSERT_X( mDeviceIcon != 0, "BtcpuiSearchView::BtcpuiSearchView", "Device Icon not found" );
    
    mDataForm=0;
    mDataForm = qobject_cast<HbDataForm *>( mLoader->findWidget( "searchHeading" ) );
    BTUI_ASSERT_X( mDataForm != 0, "BtcpuiSearchView::BtcpuiSearchView", "dataForm not found" );
    mDataForm->setHeading(hbTrId("txt_bt_subhead_bluetooth_found_devices"));

    mLabelSearching=0;
    mLabelSearching = qobject_cast<HbLabel *>( mLoader->findWidget( "searchStatus" ) );
    BTUI_ASSERT_X( mLabelSearching != 0, "BtcpuiSearchView::BtcpuiSearchView", "Searching not found" );
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_searching"));
    
    mDeviceList=0;
    mDeviceList = qobject_cast<HbListView *>( mLoader->findWidget( "deviceList" ) );
    BTUI_ASSERT_X( mDeviceList != 0, "BtcpuiSearchView::BtcpuiSearchView", "Device List not found" );   
    
    ret = connect(mDeviceList,
            SIGNAL(longPressed(HbAbstractViewItem*, QPointF)), this,
            SLOT(showContextMenu(HbAbstractViewItem*, QPointF)));
    BTUI_ASSERT_X( ret, "bt-search-view", "deviceSelected can't connect" ); 

    mDeviceList->setSelectionMode( HbAbstractItemView::SingleSelection );
    
    // read landscape orientation section from docml file if needed
    mOrientation = mMainWindow->orientation();
    
    if (mOrientation == Qt::Horizontal) {
        mLoader->load(BTUI_SEARCHVIEW_DOCML, "landscape", &ok);
        BTUI_ASSERT_X( ok, "BtcpuiSearchView::BtcpuiSearchView", "Invalid docml file: landscape section problem" );
    } else {
        mLoader->load(BTUI_SEARCHVIEW_DOCML, "portrait", &ok);
        BTUI_ASSERT_X( ok, "BtcpuiSearchView::BtcpuiSearchView", "Invalid docml file: portrait section problem" );        
    }

    // listen for orientation changes
    ret = connect(mMainWindow, SIGNAL(orientationChanged(Qt::Orientation)),
            this, SLOT(changeOrientation(Qt::Orientation)));
    BTUI_ASSERT_X( ret, "BtcpuiSearchView::BtcpuiSearchView()", "connect orientationChanged() failed");
    
    // load tool bar actions
    mViewBy = static_cast<HbAction*>( mLoader->findObject( "viewByAction" ) );
    BTUI_ASSERT_X( mViewBy, "BtcpuiSearchView::BtcpuiSearchView", "view by action missing" ); 
    ret = connect(mViewBy, SIGNAL(triggered()), this, SLOT(viewByDeviceTypeDialog()));
    BTUI_ASSERT_X( ret, "BtcpuiSearchView::BtcpuiSearchView", "viewByAction can't connect" ); 

    mStop = static_cast<HbAction*>( mLoader->findObject( "stopAction" ) );
    BTUI_ASSERT_X( mStop, "BtcpuiSearchView::BtcpuiSearchView", "stopAction missing" ); 
    ret = connect(mStop, SIGNAL(triggered()), this, SLOT(stopSearching()));
    BTUI_ASSERT_X( ret, "BtcpuiSearchView::BtcpuiSearchView", "stopAction can't connect" ); 
    mStop->setEnabled(true);
    
    mRetry = static_cast<HbAction*>( mLoader->findObject( "retryAction" ) );
    BTUI_ASSERT_X( mRetry, "BtcpuiSearchView::BtcpuiSearchView", "retryAction missing" ); 
    ret = connect(mRetry, SIGNAL(triggered()), this, SLOT(retrySearch()));
    BTUI_ASSERT_X( ret, "BtcpuiSearchView::BtcpuiSearchView", "retryAction can't connect" ); 
    // Disable for initial search
    mRetry->setEnabled(false);
    
    // load menu
    HbMenu *optionsMenu = qobject_cast<HbMenu *>(mLoader->findWidget("viewMenu"));
    BTUI_ASSERT_X( optionsMenu != 0, "BtcpuiSearchView::BtcpuiSearchView", "Options menu not found" );   
    this->setMenu(optionsMenu);      
    
    ret = connect(mDeviceList, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
    BTUI_ASSERT_X( ret, "BtcpuiSearchView::BtcpuiSearchView", "deviceSelected can't connect" ); 
    
    bool regStatus(false);
    regStatus = HbStyleLoader::registerFilePath(":/docml/btcpuisearchlistviewitem.widgetml");
    BTUI_ASSERT_X( regStatus, "BtcpuiSearchView::BtcpuiSearchView", "registerFilePath Widgetml Failed" );
    regStatus = HbStyleLoader::registerFilePath(":/docml/btcpuisearchlistviewitem.css");
    BTUI_ASSERT_X( regStatus, "BtcpuiSearchView::BtcpuiSearchView", "registerFilePath CSS Failed" );

    // initialize sort model & create
    // inquiry delegate
    // Sort is set to dynamic sort filter = true in the class, will sort automatically
    mBtuiModelSortFilter = new BtuiModelSortFilter(this);
    mBtuiModelSortFilter->setSourceModel(mDeviceModel);
    mBtuiModelSortFilter->addDeviceMajorFilter(
            BtuiDevProperty::InRange, BtuiModelSortFilter::AtLeastMatch);
    mDeviceList->setModel(mBtuiModelSortFilter);
    mDeviceList->setLayoutName("custom-list");
    // add sort order to show devices in the order they are found
    mBtuiModelSortFilter->setSortRole(BtDeviceModel::SeqNumRole);
    // Sort according to the first column (and the only column) in the listview - sorted according SeqNumRole
    mBtuiModelSortFilter->sort(0, Qt::AscendingOrder);

    BtCpUiSearchListViewItem *prototype = new BtCpUiSearchListViewItem(mDeviceList);
    prototype->setModelSortFilter(mBtuiModelSortFilter);
    mDeviceList->setItemPrototype(prototype);
}

BtcpuiSearchView::~BtcpuiSearchView()
{
    delete mLoader; // Also deletes all widgets that it constructed.
    delete mBtuiModelSortFilter;
    HbStyleLoader::unregisterFilePath(":/docml/btcpuisearchlistviewitem.widgetml");
    HbStyleLoader::unregisterFilePath(":/docml/btcpuisearchlistviewitem.css");
}

void BtcpuiSearchView::changeOrientation( Qt::Orientation orientation )
{
    // ToDo:  this does not handle redrawing list view items differently for portrait/landscape
    bool ok = false;
    mOrientation = orientation;

    if( orientation == Qt::Vertical ) {
        // load "portrait" section
        mLoader->load( BTUI_SEARCHVIEW_DOCML, "portrait", &ok );
        BTUI_ASSERT_X( ok, "BtcpuiSearchView::changeOrientation", "Invalid docml file: portrait section problem" );
    } else {
        // load "landscape" section
        mLoader->load( BTUI_SEARCHVIEW_DOCML, "landscape", &ok );
        BTUI_ASSERT_X( ok, "BtcpuiSearchView::changeOrientation", "Invalid docml file: landscape section problem" );
    }
}

/*!
   Callback for HbSelectionDialog closing
   ReImplemented Slot from Base class

 */
void BtcpuiSearchView::viewByDialogClosed(HbAction* action)
{
    int devTypesWanted = 0;
    
    devTypesWanted = selectedDeviceTypes(action);
    
    if (devTypesWanted) {
        mBtuiModelSortFilter->clearDeviceMajorFilters();
        mBtuiModelSortFilter->addDeviceMajorFilter(BtuiDevProperty::InRange,
                        BtuiModelSortFilter::AtLeastMatch);   // device must be in range
        mBtuiModelSortFilter->addDeviceMajorFilter(devTypesWanted,
                BtuiModelSortFilter::RoughMatch);             // device can be any one of selected ones
    }
}

void BtcpuiSearchView::stopSearching()
{
    // Stop searching
    if (mDelegate) {
        mDelegate->cancel();
        // update the states of view items 
        deviceSearchCompleted(KErrNone);
    }
}

void BtcpuiSearchView::take(BtAbstractDelegate *delegate)
{
    mDelegate = delegate;
    if (mDelegate) {
        disconnect(mDelegate, 0, 0, 0);
        if (mDelegate->supportedEditorTypes() & BtDelegate::Inquiry) {
            connect(mDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)),
                    this, SLOT(deviceSearchCompleted(int)));
        } else {
            connect(mDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)),
                this, SLOT(handleDelegateCompleted(int)));
        }
    }
}

void BtcpuiSearchView::startSearchDelegate()
{
    (void) createExecuteDelegate(BtDelegate::Inquiry, 
            this, SLOT(deviceSearchCompleted(int)), QVariant());
}

void BtcpuiSearchView::retrySearch()
{
    // Retry search
    // Change label and buttons to reflect status
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_searching"));
    mRetry->setEnabled(false);
    mStop->setEnabled(true);
    
    // Make use of the delegate and start the search.
    startSearchDelegate();
}

void BtcpuiSearchView::activateView( const QVariant& value, bool backNavi)
{
    Q_UNUSED(value);
    if (!backNavi) {
        retrySearch();
    }
    
    // ToDo:  consider whether orientation should be updated here rather than when orientation really changes;
    // advantage:  if orientation changes temporarily in another view, but returns to previous setting before
    // search is reactived, there is no processing overhead
}

void BtcpuiSearchView::deactivateView()
{
    stopSearching();
}
void BtcpuiSearchView::deviceSearchCompleted(int error)
{
    Q_UNUSED(error);
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_search_done"));
    mRetry->setEnabled(true);
    mStop->setEnabled(false);
    handleDelegateCompleted(error, mDelegate);
}

void BtcpuiSearchView::createContextMenuActions(int majorRole)
{
    if (majorRole & BtuiDevProperty::Connected) {
        if(majorRole & BtuiDevProperty::AVDev) {
            mContextMenu->addAction(hbTrId("txt_bt_menu_disconnect_audio"));
        }
        else {
            mContextMenu->addAction(hbTrId("txt_bt_menu_disconnect"));
        }
    }
    else {
        if(majorRole & BtuiDevProperty::AVDev) {
            mContextMenu->addAction(hbTrId("txt_bt_menu_connect_audio"));
        }
        else {
            mContextMenu->addAction(hbTrId("txt_bt_menu_connect"));
        }
    }  
}
