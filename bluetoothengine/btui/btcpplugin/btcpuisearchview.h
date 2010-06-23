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

#ifndef	BTCPUISEARCHVIEW_H
#define	BTCPUISEARCHVIEW_H

#include <cpbasesettingview.h>
#include <hbaction.h>
#include <hbtoolbar.h>
#include "btcpuibaseview.h"
#include "btuimodelsortfilter.h"

class HbLabel;
class HbPushButton;
class HbIcon;
class HbDocumentLoader;
class HbListView;
class BtAbstractDelegate;

class BtCpUiSearchView : public BtCpUiBaseView
{
    Q_OBJECT
    
public:

    explicit BtCpUiSearchView(
            BtSettingModel &settingModel, 
            BtDeviceModel &deviceModel, 
            QGraphicsItem *parent = 0);
    virtual ~BtCpUiSearchView();
    virtual void activateView( const QVariant& value, int cmdId );
    virtual void deactivateView();
    virtual void setSoftkeyBack();
    
public slots:
    virtual void switchToPreviousView();
    void changeOrientation( Qt::Orientation orientation );
    void stopSearching();
    void retrySearch();
    void viewByDeviceTypeDialog();
    void viewByDialogClosed(HbAction* action);
    void searchDelegateCompleted(int error);
    void deviceSearchCompleted(int error);
    void deviceSelected(const QModelIndex& modelIndex);
    
private:
    void startSearchDelegate();
    
private:
    enum devTypeSelectionList {
        BtUiDevAudioDevice = 0,
        BtUiDevComputer,
        BtUiDevInputDevice,
        BtUiDevPhone,
        BtUiDevOtherDevice
    };
private:
    HbDocumentLoader *mLoader;
    HbLabel *mDeviceIcon;
    HbLabel *mLabelFoundDevices;
    HbLabel *mLabelSearching;        
    HbListView *mDeviceList;
    QStringList mDevTypeList;
    int mLastSelectionIndex;
    
    // data structures for switching between views
    bool mEventFilterInstalled;
    int mAutoCmdId;
    Qt::Orientation mOrientation;
    
    HbMainWindow*           mMainWindow;
    BtCpUiBaseView*         mMainView;
    BtCpUiBaseView*         mDeviceView;
    HbAction *              mSoftKeyBackAction;
    HbToolBar*              mToolBar;
    HbAction*               mViewBy;
    HbAction*               mStop;
    HbAction*               mRetry;
    HbAction*               mExit;
    HbAction*               mConnect;

    //pointer to abstract delegate, and it is instantiated at runtime
    BtAbstractDelegate*     mAbstractDelegate;
    QModelIndex*            mParentIndex;
    int                     mNumOfRows;
    BtuiModelSortFilter*    mBtuiModelSortFilter;
};

#endif//	BTCPUISEARCHVIEW_H
