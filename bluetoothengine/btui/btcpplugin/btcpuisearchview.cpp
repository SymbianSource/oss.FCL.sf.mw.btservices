/*
 * Copyright (c) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include "btuiviewutil.h"
#include <QtGui/QGraphicsLinearLayout>
#include <HbInstance>
#include <HbDocumentLoader>
#include <HbDataForm>
#include <HbPushButton>
#include <HbLabel>
#include <HbListView>
#include <HbMenu>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <bluetoothuitrace.h>
#include "btcpuimainview.h"
#include "btdelegatefactory.h"
#include "btabstractdelegate.h"



// docml to load
const char* BTUI_SEARCHVIEW_DOCML = ":/docml/bt-search-view.docml";

BtCpUiSearchView::BtCpUiSearchView(
        BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel, 
        QGraphicsItem *parent) :
    BtCpUiBaseView(settingModel,deviceModel, parent), mAbstractDelegate(0), mBtuiModelSortFilter(0)
{
    bool ret(false);
    
    mMainView = (BtCpUiMainView *) parent;
    
    mMainWindow = hbInstance->allMainWindows().first();
    
    mSoftKeyBackAction = new HbAction(Hb::BackNaviAction, this);
    BTUI_ASSERT_X(mSoftKeyBackAction, "BtCpUiBaseView::BtCpUiBaseView", "can't create back action");
    
    // Create view for the application.
    // Set the name for the view. The name should be same as the view's
    // name in docml.
    setObjectName("bt_search_view");

    QObjectList objectList;
    objectList.append(this);
    // Pass the view to documentloader. Document loader uses this view
    // when docml is parsed, instead of creating new view.
    mLoader = new HbDocumentLoader();
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
    mDeviceIcon = qobject_cast<HbLabel *>( mLoader->findWidget( "icon" ) );  
    BTUI_ASSERT_X( mDeviceIcon != 0, "bt-search-view", "Device Icon not found" );
    
    mLabelFoundDevices=0;
    mLabelFoundDevices = qobject_cast<HbLabel *>( mLoader->findWidget( "label_found_devices" ) );
    BTUI_ASSERT_X( mLabelFoundDevices != 0, "bt-search-view", "Found Devices not found" );
    mLabelFoundDevices->setPlainText(hbTrId("txt_bt_subhead_bluetooth_found_devices"));

    mLabelSearching=0;
    mLabelSearching = qobject_cast<HbLabel *>( mLoader->findWidget( "label_searching" ) );
    BTUI_ASSERT_X( mLabelSearching != 0, "bt-search-view", "Searching not found" );
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_searching"));
    
    mDeviceList=0;
    mDeviceList = qobject_cast<HbListView *>( mLoader->findWidget( "deviceList" ) );
    BTUI_ASSERT_X( mDeviceList != 0, "bt-search-view", "Device List not found" );   
    
    
    mDeviceList->setSelectionMode( HbAbstractItemView::SingleSelection );
    
    // read landscape orientation section from docml file if needed
    mOrientation = mMainWindow->orientation();
    
    if (mOrientation == Qt::Horizontal) {
        mLoader->load(BTUI_SEARCHVIEW_DOCML, "landscape_ui", &ok);
        BTUI_ASSERT_X( ok, "bt-search-view", "Invalid docml file: landscape section problem" );
    } else {
        mLoader->load(BTUI_SEARCHVIEW_DOCML, "portrait_ui", &ok);
        BTUI_ASSERT_X( ok, "bt-search-view", "Invalid docml file: portrait section problem" );        
    }

    // listen for orientation changes
    ret = connect(mMainWindow, SIGNAL(orientationChanged(Qt::Orientation)),
            this, SLOT(changeOrientation(Qt::Orientation)));
    BTUI_ASSERT_X( ret, "BtCpUiSearchView::BtCpUiSearchView()", "connect orientationChanged() failed");
    
    // load tool bar actions
    mViewBy = static_cast<HbAction*>( mLoader->findObject( "viewByAction" ) );
    BTUI_ASSERT_X( mViewBy, "bt-search-view", "view by action missing" ); 
//    TODO - Need to implement the View by
//    ret = connect(viewByAction, SIGNAL(triggered()), this, SLOT(noOp()));
//    BTUI_ASSERT_X( ret, "bt-search-view", "viewByAction can't connect" ); 

    mStop = static_cast<HbAction*>( mLoader->findObject( "stopAction" ) );
    BTUI_ASSERT_X( mStop, "bt-search-view", "stopAction missing" ); 
    ret = connect(mStop, SIGNAL(triggered()), this, SLOT(stopSearching()));
    BTUI_ASSERT_X( ret, "bt-search-view", "stopAction can't connect" ); 
    mStop->setEnabled(true);
    
    mRetry = static_cast<HbAction*>( mLoader->findObject( "retryAction" ) );
    BTUI_ASSERT_X( mRetry, "bt-search-view", "retryAction missing" ); 
    ret = connect(mRetry, SIGNAL(triggered()), this, SLOT(retrySearch()));
    BTUI_ASSERT_X( ret, "bt-search-view", "retryAction can't connect" ); 
    // Disable for initial search
    mRetry->setEnabled(false);
    
    // load menu
    HbMenu *optionsMenu = qobject_cast<HbMenu *>(mLoader->findWidget("viewMenu"));
    BTUI_ASSERT_X( optionsMenu != 0, "bt-search-view", "Options menu not found" );   
    this->setMenu(optionsMenu);      
    
    mExit = static_cast<HbAction*>( mLoader->findObject( "exitAction" ) );
    BTUI_ASSERT_X( mExit, "bt-search-view", "exitAction missing" ); 
    mExit->setText(hbTrId("txt_common_opt_exit"));
    
    mConnect = static_cast<HbAction*>( mLoader->findObject( "connectAction" ) );
    BTUI_ASSERT_X( mConnect, "bt-search-view", "connectAction missing" ); 
    mConnect->setText(hbTrId("txt_bt_menu_connect"));
    
    ret = connect(mDeviceList, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
    BTUI_ASSERT_X( ret, "bt-search-view", "deviceSelected can't connect" ); 
}

BtCpUiSearchView::~BtCpUiSearchView()
{
    setNavigationAction(0);
    delete mSoftKeyBackAction;
    
    if(mAbstractDelegate) {
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    
    if(mBtuiModelSortFilter) {
        delete mBtuiModelSortFilter;
        mBtuiModelSortFilter = 0; 
    }
}

void BtCpUiSearchView::changeOrientation( Qt::Orientation orientation )
{
    bool ok = false;
    mOrientation = orientation;
    if( orientation == Qt::Vertical ) {
        // load "portrait" section
        mLoader->load( BTUI_SEARCHVIEW_DOCML, "portrait_ui", &ok );
        BTUI_ASSERT_X( ok, "bt-search-view", "Invalid docml file: portrait section problem" );
    } else {
        // load "landscape" section
        mLoader->load( BTUI_SEARCHVIEW_DOCML, "landscape_ui", &ok );
        BTUI_ASSERT_X( ok, "bt-search-view", "Invalid docml file: landscape section problem" );
    }
}

void BtCpUiSearchView::stopSearching()
{
    // Stop delegate
    
    // Change label and buttons to reflect status
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_search_done"));
    mRetry->setEnabled(true);
    mStop->setEnabled(false);    

    // Stop delegate
    mAbstractDelegate->cancel();
}

void BtCpUiSearchView::retrySearch()
{
    // Retry delegate

    // Change label and buttons to reflect status
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_searching"));
    mRetry->setEnabled(false);
    mStop->setEnabled(true);
    
    // Make use of the delegate and start the search.
    mAbstractDelegate->cancel();
    mDeviceList->setModel(mBtuiModelSortFilter);
    mAbstractDelegate->exec(QVariant());
}

void BtCpUiSearchView::setSoftkeyBack()
{
    if (navigationAction() != mSoftKeyBackAction) {
        setNavigationAction(mSoftKeyBackAction);
        connect( mSoftKeyBackAction, SIGNAL(triggered()), this, SLOT(switchToPreviousView()) );
    }
}

void BtCpUiSearchView::switchToPreviousView()
{
    BTUI_ASSERT_X(mMainView, "BtCpUiSearchView::switchToPreviousView", "invalid mMainView");
    mMainView->switchToPreviousView();
}

void BtCpUiSearchView::activateView( const QVariant& value, int cmdId )
{
    Q_UNUSED(value);
    Q_UNUSED(cmdId);  
    
    setSoftkeyBack();
    
    bool ret(false);
    
    if(!mAbstractDelegate) {
        // Create the inquiry delegate.
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::Inquiry, mSettingModel, mDeviceModel );
    }
    
    // Connect to the signal from the BtDelegateInquiry for completion of the search request
    ret = connect(mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(searchDelegateCompleted(int)));
    BTUI_ASSERT_X( ret, "bt-search-view", "searchDelegateCompleted can't connect" );
    
    // Connect to the signal from the BtuiModel when the search has been completed.
    ret = connect(mDeviceModel, SIGNAL(deviceSearchCompleted(int)), this, SLOT(deviceSearchCompleted(int)));
    BTUI_ASSERT_X( ret, "bt-search-view", "deviceSearchCompleted can't connect" ); 
    
    if(!mBtuiModelSortFilter) {
        mBtuiModelSortFilter = new BtuiModelSortFilter(this);
    }
    mBtuiModelSortFilter->setSourceModel(mDeviceModel);
    mBtuiModelSortFilter->addDeviceMajorFilter(BtDeviceModel::InRange, BtuiModelSortFilter::AtLeastMatch);
    mDeviceList->setModel(mBtuiModelSortFilter);

    // Make use of the delegate and start the search.
    mAbstractDelegate->exec(QVariant());
}

void BtCpUiSearchView::deactivateView()
{
}

void BtCpUiSearchView::searchDelegateCompleted(int error)
{
    //TODO - handle error.
    Q_UNUSED(error);
}

void BtCpUiSearchView::deviceSearchCompleted(int error)
{
    //TODO - handle error.
    Q_UNUSED(error);
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_search_done"));
    mRetry->setEnabled(true);
    mStop->setEnabled(false);    
}

void BtCpUiSearchView::deviceSelected(const QModelIndex& modelIndex)
{
    QModelIndex index = mBtuiModelSortFilter->mapToSource(modelIndex);
    static_cast<BtCpUiMainView*>(mMainView)->goToDeviceView(index);
}
