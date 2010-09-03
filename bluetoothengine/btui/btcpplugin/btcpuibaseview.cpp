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

/*!
    This constructor constructs new setting and device models.
 */
BtcpuiBaseView::BtcpuiBaseView(QGraphicsItem *parent) :
    CpBaseSettingView(0, parent), mViewMgr(0), mDelegate(0), mPreviousView(0),
            mBack(0), mQuery(0), mContextMenu(0), mBtuiModelSortFilter(0)
{
    mSettingModel = new BtSettingModel(this);
    mDeviceModel = new BtDeviceModel(this);
    initialise();
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
    mSettingModel = new BtSettingModel(settingModel, this);
    mDeviceModel = new BtDeviceModel(deviceModel, this);
    initialise();
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

    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiBaseView::initialise()
{
    bool ret(false); 
    mMainWindow = hbInstance->allMainWindows().first();
    mContextMenu = new HbMenu();
    ret = connect(mContextMenu, SIGNAL(triggered(HbAction *)), this, SLOT(contextMenuTriggered(HbAction *)));
    BTUI_ASSERT_X( ret, "bt-main-view", "Context Menu can't connect" );
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
    BOstraceFunctionExit0(DUMMY_DEVLIST);
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
    BOstraceFunctionExit0(DUMMY_DEVLIST);
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
        BOstraceExt1(TRACE_DEBUG, DUMMY_DEVLIST, "BtcpuiBaseView::createDelegate signal connect %d", ok);
        if (!ok) {
            delete mDelegate;
            mDelegate = 0;
        }
    }
    BOstraceExt2(TRACE_DEBUG, DUMMY_DEVLIST, "BtcpuiBaseView::createDelegate: mDe: 0x%08X, ret %d", mDelegate, ok);
    return ok;
}

bool BtcpuiBaseView::createExecuteDelegate(BtDelegate::EditorType type,
        QObject *receiver, const char *member, const QVariant &param)
{
    bool ok = createDelegate(type, receiver, member);
    if (ok) {
        mDelegate->exec(param);
    }
    return ok;
}

void BtcpuiBaseView::viewByDeviceTypeDialog()
{
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
}

void BtcpuiBaseView::viewByDialogClosed(HbAction* action)
{
    Q_UNUSED(action)
    //ReImpemented in derived classes.
}

int BtcpuiBaseView::selectedDeviceTypes(HbAction* action)
{
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
    
    return devTypesWanted;
}

void BtcpuiBaseView::deviceSelected(const QModelIndex& modelIndex)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    BtcpuiBaseView *devView = viewMgr()->deviceView();
    // For navigating back to this view:
    devView->setPreviousView( this );
    QModelIndex index = mBtuiModelSortFilter->mapToSource(modelIndex);
    QVariant params;
    params.setValue(index);
    viewMgr()->switchView(this, devView, params, false);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtcpuiBaseView::createContextMenuActions(int majorRole)
{
    //Re-Implemented in derived classes.
    Q_UNUSED(majorRole)
}

void BtcpuiBaseView::take(BtAbstractDelegate *delegate)
{
    mDelegate = delegate;
    if (mDelegate) {
        disconnect(mDelegate, 0, 0, 0);
        connect(mDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)),
                this, SLOT(handleDelegateCompleted(int)));
    }
}

void BtcpuiBaseView::contextMenuTriggered(HbAction *action)
{
    if(!(action->text().compare(hbTrId("txt_common_menu_open")))) {
            deviceSelected(mLongPressedItem->modelIndex());
        }
}

void BtcpuiBaseView::showContextMenu(HbAbstractViewItem *item, const QPointF &coords)
{
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
}

void BtcpuiBaseView::handleDelegateCompleted(int error, BtAbstractDelegate* delegate)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, error );
    Q_UNUSED(error);
    delete mDelegate;
    mDelegate = 0;
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}
