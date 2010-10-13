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

#ifndef BTCPUIBASEVIEW_H
#define BTCPUIBASEVIEW_H

#include <hbview.h>
#include <qglobal.h>
#include <cpbasesettingview.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <btdelegateconsts.h>
#include <btuimodelsortfilter.h>
#include <HbAbstractViewItem>
#include <HbMenu>

class BtcpuiViewMgr;
class HbSelectionDialog;
class BtAbstractDelegate;
class HbMainWindow;

/*!
    \class BtUiBaseView
    \brief the class is the base class for all views in btapplication.

 */
class BtcpuiBaseView : public CpBaseSettingView
{
    Q_OBJECT

public:
    
    virtual ~BtcpuiBaseView();
    
    virtual void activateView( const QVariant& value, bool backNavi) = 0;
    virtual void deactivateView() = 0;
    virtual void setPreviousView(BtcpuiBaseView *view);
    void setViewMgr(BtcpuiViewMgr *mgr);
    virtual void take(BtAbstractDelegate *delegate);
    
public slots:
    
    virtual void backToPreviousView();
    void viewByDeviceTypeDialog();
    virtual void viewByDialogClosed(HbAction* action);
    void contextMenuTriggered(HbAction *action);
    void openDeviceView(const QModelIndex& modelIndex);
    void showContextMenu(HbAbstractViewItem *item, const QPointF &coords);
    void handleDelegateCompleted(int error, BtAbstractDelegate* delegate);
    
signals:

protected:
    explicit BtcpuiBaseView(QGraphicsItem *parent = 0);
    
    explicit BtcpuiBaseView(BtSettingModel &settingModel, 
            BtDeviceModel &deviceModel, 
            QGraphicsItem *parent = 0);
    
    void initialise();
    
    BtSettingModel *settingModel();
    
    BtDeviceModel *deviceModel();
    
    BtcpuiViewMgr *viewMgr();

    bool createDelegate(BtDelegate::EditorType type,
            QObject *receiver, const char *member);
    
    bool createExecuteDelegate(BtDelegate::EditorType type,
            QObject *receiver, const char *member, const QVariant &param);    

    int selectedDeviceTypes(HbAction* action);
    
    virtual void createContextMenuActions(int majorRole);

    virtual void connectToDevice(const QModelIndex& modelIndex);
    
    virtual void disconnectFromDevice(const QModelIndex& modelIndex);
    
protected:

    enum devTypeSelectionList {
        BtUiDevAudioDevice = 0,
        BtUiDevComputer,
        BtUiDevInputDevice,
        BtUiDevPhone,
        BtUiDevOtherDevice
    };

    // The view manager from which the device view can be accessed.
    // not owned.
    BtcpuiViewMgr *mViewMgr;
    
    HbMainWindow *mMainWindow;
    
    // owned
    BtSettingModel *mSettingModel;
    
    // owned
    BtDeviceModel *mDeviceModel;
    
    //The run-time singleton delegate in a btui view
    BtAbstractDelegate* mDelegate;
    
    // The view which this view is navigated from.
    // We will navigate to previous view if the back action is activated.
    BtcpuiBaseView *mPreviousView;
    
    // The back action of this view.
    HbAction *mBack;
    
    HbSelectionDialog *mQuery;
    
    HbMenu* mContextMenu;
    HbAbstractViewItem *mLongPressedItem;

    BtuiModelSortFilter*    mBtuiModelSortFilter;
    Q_DISABLE_COPY(BtcpuiBaseView)
};

#endif // BTCPUIBASEVIEW_H
