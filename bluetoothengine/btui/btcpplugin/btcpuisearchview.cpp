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

#include "btcpuisearchview.h"
#include <QtGui/QGraphicsLinearLayout>
#include <HbInstance>
#include <hbdocumentloader.h>
#include <hbdataform.h>
#include <hbpushbutton.h>
#include <hblabel.h>
#include <hblistview.h>
#include <hbmenu.h>
#include <qstringlist>
#include <qdebug>
#include <bluetoothuitrace.h>
#include "btcpuimainview.h"



// docml to load
const char* BTUI_SEARCHVIEW_DOCML = ":/docml/bt-search-view.docml";

BtCpUiSearchView::BtCpUiSearchView(BtuiModel &model, QGraphicsItem *parent) :
    BtCpUiBaseView(model,parent)
/*{
    mSoftKeyBackAction = new HbAction(Hb::BackNaviAction, this);
    BTUI_ASSERT_X(mSoftKeyBackAction, "BtCpUiBaseView::BtCpUiBaseView", "can't create back action");

    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout( Qt::Vertical, this );
    // create button
    HbPushButton *button = new HbPushButton(this);
    button->setText("Press Me");
    button->setMaximumSize(150,50);
    mainLayout->addItem(button);  

//    (void) connect(button, SIGNAL(clicked()), this, SLOT(changePowerState()));  
    
    setLayout(mainLayout);
    
    // create dummy options menu
    HbMenu *optionsMenu = new HbMenu();
    HbAction *openGadgetGallery = optionsMenu->addAction("Open Device Gallery");
//    connect(openGadgetGallery, SIGNAL(triggered()), this, SLOT(openGadgetGalleryView()));
    HbAction *openNewMainView = optionsMenu->addAction("Open new Main View");   
//    connect(openNewMainView, SIGNAL(triggered()), this, SLOT(openNewMainView()));   
    setMenu(optionsMenu);

}*/
    
{
    //bool ret(false);
    
    mSearchView = this;
    mMainView = (BtCpUiMainView *) parent;
    
    mMainWindow = hbInstance->allMainWindows().first();
    
    mSoftKeyBackAction = new HbAction(Hb::BackNaviAction, this);
    BTUI_ASSERT_X(mSoftKeyBackAction, "BtCpUiBaseView::BtCpUiBaseView", "can't create back action");
    

    // read view info from docml file


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
    
    mDeviceName=0;
    mDeviceName = qobject_cast<HbLabel *>( mLoader->findWidget( "label_found_devices" ) );
    BTUI_ASSERT_X( mDeviceName != 0, "bt-search-view", "Device Name not found" );
        
    mDeviceList=0;
    mDeviceList = qobject_cast<HbListView *>( mLoader->findWidget( "deviceList" ) );
    BTUI_ASSERT_X( mDeviceList != 0, "bt-search-view", "Device List not found" );   
    
    
//    // set model to list view
//    mDeviceList->setModel( mFilterProxy );
//    // define settings for list view
//    mDeviceList->setSelectionMode(HbAbstractItemView::SingleSelection);
//    // set prototype item for list view
//    BtUiDevListGridItem *item = new BtUiDevListGridItem( mDeviceList ); 
//    mDeviceList->setItemPrototype(item);
//    // connect to list view pressed signal
//    ret = connect( mDeviceList, SIGNAL(pressed(QModelIndex)),this, SLOT(itemActivated(QModelIndex)) );
//    BTUI_ASSERT_X( ret, "bt-search-view", "device list can't connect" ); 
    
    // read landscape orientation section from docml file if needed
//    mOrientation = ((BTUIViewManager*)parent)->orientation();
    mOrientation = Qt::Vertical;
    if (mOrientation == Qt::Horizontal) {
        mLoader->load(BTUI_SEARCHVIEW_DOCML, "landscape", &ok);
        BTUI_ASSERT_X( ok, "bt-search-view", "Invalid docml file: landscape section problem" );
    }

    
    // load tool bar actions
    HbAction *viewByAction = static_cast<HbAction*>( mLoader->findObject( "viewByAction" ) );
    BTUI_ASSERT_X( viewByAction, "bt-search-view", "view by action missing" ); 
//    ret = connect(viewByAction, SIGNAL(triggered()), this, SLOT(noOp()));
//    BTUI_ASSERT_X( ret, "bt-search-view", "viewByAction can't connect" ); 

    HbAction *stopAction = static_cast<HbAction*>( mLoader->findObject( "stopAction" ) );
    BTUI_ASSERT_X( stopAction, "bt-search-view", "view by action missing" ); 
//    ret = connect(stopAction, SIGNAL(triggered()), this, SLOT(noOp()));
//    BTUI_ASSERT_X( ret, "bt-search-view", "stopAction can't connect" ); 

    HbAction *retryAction = static_cast<HbAction*>( mLoader->findObject( "retryAction" ) );
    BTUI_ASSERT_X( retryAction, "bt-search-view", "view by action missing" ); 
//    ret = connect(retryAction, SIGNAL(triggered()), this, SLOT(noOp()));
//    BTUI_ASSERT_X( ret, "bt-search-view", "retryAction can't connect" ); 

    
    // load menu
    HbMenu *optionsMenu = qobject_cast<HbMenu *>(mLoader->findWidget("viewMenu"));
    BTUI_ASSERT_X( optionsMenu != 0, "bt-search-view", "Options menu not found" );   
    this->setMenu(optionsMenu);
        
    
    // update display when setting data changed
//    ret = connect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), 
//            this, SLOT(updateSettingItems(QModelIndex,QModelIndex)));
//    BTUI_ASSERT_X( ret, "BtCpUiSearchView::BtCpUiSearchView", "can't connect dataChanged" );
    
//    QModelIndex top = mModel->index( Btuim::LocalDeviceName, KDefaultColumn );
//    QModelIndex bottom = mModel->index( Btuim::Visibility, KDefaultColumn );
//    // update name, power and visibility rows
//    updateSettingItems( top, bottom );

 
}

BtCpUiSearchView::~BtCpUiSearchView()
{
    setNavigationAction(0);
    delete mSoftKeyBackAction;
}

void BtCpUiSearchView::deviceSelected(const QModelIndex& modelIndex)
{
    int row = modelIndex.row();
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
    // jump to previous view of current view.
    mSearchView->deactivateView();
    
    // set the new current view 
    mMainWindow->setCurrentView( mMainView );

    // do preparation or some actions when new view is activated 
    mMainView->activateView( 0, 0 );

}

void BtCpUiSearchView::activateView( const QVariant& value, int cmdId )
{
    Q_UNUSED(value);
    Q_UNUSED(cmdId);  
    
    setSoftkeyBack();
}

void BtCpUiSearchView::deactivateView()
{
}
