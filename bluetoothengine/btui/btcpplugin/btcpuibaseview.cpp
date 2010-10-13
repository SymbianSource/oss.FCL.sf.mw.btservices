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

#include "btcpuibaseview.h"
#include <hbaction.h>
#include <HbInstance>
#include <bluetoothuitrace.h>
#include "btcpuiviewmgr.h"
#include <HbSelectionDialog>
#include <HbLabel>
#include "btuidevtypemap.h"
#include <btabstractdelegate.h>
#include <btdelegatefactory.h>
#include "btqtconstants.h"


/*!
    This constructor constructs new setting and device models.
 */
BtcpuiBaseView::BtcpuiBaseView(QGraphicsItem *parent) :
    CpBaseSettingView(0, parent), mViewMgr(0), mDelegate(0), mPreviousView(0),
            mBack(0), mQuery(0), mContextMenu(0), mBtuiModelSortFilter(0)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mSettingModel = new BtSettingModel(this);
    mDeviceModel = new BtDeviceModel(this);
    initialise();
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

/*!
    This constructor constructs models from the given setting and device models.
    This implies the model impl and data structure are shared.
 */
BtcpuiBaseView::BtcpuiBaseView(BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel,
        QGraphicsItem *parent) :
    CpBaseSettingView(0, parent), mViewMgr(0), mDelegate(0), mPreviousView(0),
            mBack(0), mQuery(0), mContextMenu(0), mBtuiModelSortFilter(0)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mSettingModel = new BtSettingModel(settingModel, this);
    mDeviceModel = new BtDeviceModel(deviceModel, this);
    initialise();
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

/*!
    Destructor.
 */
BtcpuiBaseView::~BtcpuiBaseView()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    delete mDelegate;
    delete mQuery;
    
    if(mContextMenu) {
        mContextMenu->clearActions();
        delete mContextMenu;
    }

    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

void BtcpuiBaseView::initialise()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    bool ret(false); 
    mMainWindow = hbInstance->allMainWindows().first();
    mContextMenu = new HbMenu();
    ret = connect(mContextMenu, SIGNAL(triggered(HbAction *)), this, SLOT(contextMenuTriggered(HbAction *)));
    BTUI_ASSERT_X( ret, "BtcpuiBaseView::initialise()", "Context Menu can't connect" );
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

void BtcpuiBaseView::setPreviousView(BtcpuiBaseView *view)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mPreviousView = view;
    // when a non-null previous view is set, it means this view is navigated from an existing
    // view. And the back-action should navigate to the previous view.
    if (mPreviousView) {
        // Back action is created on demand.
        if (!mBack) {
            mBack = new HbAction(Hb::BackNaviAction, this);
            BTUI_ASSERT_X(mBack, "BtcpuiBaseView::setPreviousView", "can't create back action");
            connect(mBack, SIGNAL(triggered()), this, SLOT(backToPreviousView()));
        }
        if (navigationAction() != mBack) {
            setNavigationAction(mBack);
        }
    }
    else {
        setNavigationAction(0);
    }
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

void BtcpuiBaseView::setViewMgr(BtcpuiViewMgr *mgr)
{
    mViewMgr = mgr;
}

void BtcpuiBaseView::backToPreviousView()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if ( mPreviousView ) {
        viewMgr()->switchView(this, mPreviousView, QVariant(), true);
    }
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

BtSettingModel *BtcpuiBaseView::settingModel()
{
    return mSettingModel;
}

BtDeviceModel *BtcpuiBaseView::deviceModel()
{
    return mDeviceModel;
}

BtcpuiViewMgr *BtcpuiBaseView::viewMgr()
{
    return mViewMgr;
}

bool BtcpuiBaseView::createDelegate(BtDelegate::EditorType type,
        QObject *receiver, const char *member)
{
    bool ok(false);
    if(!mDelegate) {
        mDelegate = BtDelegateFactory::newDelegate(type, mSettingModel, mDeviceModel); 
        ok = connect(mDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)),
                receiver, member);
        BOstraceExt1(TRACE_DEBUG, DUMMY_DEVLIST, "BtcpuiBaseView::createDelegate new: signal connect %d", ok);
        if (!ok) {
            delete mDelegate;
            mDelegate = 0;
        }
    }
    BOstraceExt2(TRACE_DEBUG, DUMMY_DEVLIST, "BtcpuiBaseView::createDelegate(): mDe: 0x%08X, ret %d", mDelegate, ok);
    return ok;
}

bool BtcpuiBaseView::createExecuteDelegate(BtDelegate::EditorType type,
        QObject *receiver, const char *member, const QVariant &param)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    bool ok = createDelegate(type, receiver, member);
    if (ok) {
        mDelegate->exec(param);
    }
    BOstraceFunctionExitExt(DUMMY_DEVLIST, this, ok);
    return ok;
}

void BtcpuiBaseView::viewByDeviceTypeDialog()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if ( !mQuery ) {
        mQuery = new HbSelectionDialog();
        QStringList devTypeList;
        devTypeList << hbTrId("txt_bt_list_audio_devices")
                << hbTrId("txt_bt_list_computers") 
                << hbTrId("txt_bt_list_input_devices") 
                << hbTrId("txt_bt_list_phones") 
                << hbTrId("txt_bt_list_other_devices");

        
        mQuery->setStringItems(devTypeList, 0);
        mQuery->setSelectionMode(HbAbstractItemView::MultiSelection);
    
        QList<QVariant> current;
        current.append(QVariant(0));
        mQuery->setSelectedItems(current);
    
        // Set the heading for the dialog.
        HbLabel *headingLabel = new HbLabel(hbTrId("txt_bt_title_show"), mQuery);
        mQuery->setHeadingWidget(headingLabel);
    }
    mQuery->open(this,SLOT(viewByDialogClosed(HbAction*)));
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

void BtcpuiBaseView::viewByDialogClosed(HbAction* action)
{
    Q_UNUSED(action)
    //ReImpemented in derived classes.
}

int BtcpuiBaseView::selectedDeviceTypes(HbAction* action)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    int devTypesWanted = 0;
    
    disconnect( mQuery, 0, this, 0);  // remove signal
    if (action == mQuery->actions().first()) {  // user pressed "Ok"
        // Get selected items.
        QList<QVariant> selections;
        selections = mQuery->selectedItems();
        
        for (int i=0; i < selections.count(); i++) {
            switch (selections.at(i).toInt()) {
            case BtUiDevAudioDevice:
                devTypesWanted |= BtuiDevProperty::AVDev;
                break;
            case BtUiDevComputer:
                devTypesWanted |= BtuiDevProperty::Computer;
                break;
            case BtUiDevInputDevice:
                devTypesWanted |= BtuiDevProperty::Peripheral;
                break;
            case BtUiDevPhone:
                devTypesWanted |= BtuiDevProperty::Phone;
                break;
            case BtUiDevOtherDevice:
                devTypesWanted |= (BtuiDevProperty::LANAccessDev |
                        BtuiDevProperty::Toy |
                        BtuiDevProperty::WearableDev |
                        BtuiDevProperty::ImagingDev |
                        BtuiDevProperty::HealthDev |
                        BtuiDevProperty::UncategorizedDev);
                break;
            default:
                // should never get here
                BTUI_ASSERT_X(false, "BtcpuiSearchView::viewByDialogClosed()", 
                        "wrong device type in viewBy dialog!");
            }
        }
    }
    
    BOstraceFunctionExitExt(DUMMY_DEVLIST, this, devTypesWanted);
    return devTypesWanted;
}

void BtcpuiBaseView::openDeviceView(const QModelIndex& modelIndex)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    BtcpuiBaseView *devView = viewMgr()->deviceView();
    // For navigating back to this view:
    devView->setPreviousView( this );
    QModelIndex index = mBtuiModelSortFilter->mapToSource(modelIndex);
    QVariant params;
    params.setValue(index);
    viewMgr()->switchView(this, devView, params, false);
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

void BtcpuiBaseView::createContextMenuActions(int majorRole)
{
    //Re-Implemented in derived classes.
    Q_UNUSED(majorRole)
}

void BtcpuiBaseView::take(BtAbstractDelegate *delegate)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mDelegate = delegate;
    if (mDelegate) {
        disconnect(mDelegate, 0, 0, 0);
        connect(mDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)),
                this, SLOT(handleDelegateCompleted(int)));
    }
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

//  this could be made virtual if derived classes need different functionality
void BtcpuiBaseView::contextMenuTriggered(HbAction *action)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if(!(action->text().compare(hbTrId("txt_common_menu_open")))) {
        openDeviceView(mLongPressedItem->modelIndex());
    } 
    else if (!(action->text().compare(hbTrId("txt_bt_menu_connect_audio")))
            || !(action->text().compare(hbTrId("txt_bt_menu_connect"))))  {
        connectToDevice(mLongPressedItem->modelIndex());
    }
    else if (!(action->text().compare(hbTrId("txt_bt_menu_disconnect_audio")))
            || !(action->text().compare(hbTrId("txt_bt_menu_disconnect"))))  {
        disconnectFromDevice(mLongPressedItem->modelIndex());
    }
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}


void BtcpuiBaseView::showContextMenu(HbAbstractViewItem *item, const QPointF &coords)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mLongPressedItem = item;
    mContextMenu->clearActions();
    
    mContextMenu->addAction(hbTrId("txt_common_menu_open"));
    
    QModelIndex index = mBtuiModelSortFilter->mapToSource(mLongPressedItem->modelIndex());  
    
    int majorPropRole = (mDeviceModel->data(index,BtDeviceModel::MajorPropertyRole)).toInt();
    if (majorPropRole & BtuiDevProperty::Connectable ) {
        createContextMenuActions(majorPropRole);
    }
    mContextMenu->setPreferredPos(coords);
    mContextMenu->open();
    
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

void BtcpuiBaseView::connectToDevice(const QModelIndex& modelIndex)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QModelIndex index = mBtuiModelSortFilter->mapToSource(modelIndex);
    
    QVariant param;
    param.setValue(index);

    (void) createExecuteDelegate(BtDelegate::ConnectService, 
            this, SLOT(handleDelegateCompleted(int,BtAbstractDelegate*)), param);    
    
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

void BtcpuiBaseView::disconnectFromDevice(const QModelIndex& modelIndex)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QModelIndex index = mBtuiModelSortFilter->mapToSource(modelIndex);   
    QVariant deviceBtAddress = mDeviceModel->data(index, BtDeviceModel::ReadableBdaddrRole); 
            
    QList<QVariant>paramList;
    paramList.append(QVariant(ServiceLevel));
    paramList.append(deviceBtAddress);

    (void) createExecuteDelegate(BtDelegate::DisconnectService, 
            this, SLOT(handleDelegateCompleted(int,BtAbstractDelegate*)), QVariant(paramList));
    
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}

void BtcpuiBaseView::handleDelegateCompleted(int error, BtAbstractDelegate* delegate)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, error );
    Q_UNUSED(delegate);
    Q_UNUSED(error);
    delete mDelegate;
    mDelegate = 0;
    BOstraceFunctionExit1(DUMMY_DEVLIST, this);
}
