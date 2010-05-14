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

#ifndef	BTCPUISEARCHVIEW_H
#define	BTCPUISEARCHVIEW_H

#include <cpbasesettingview.h>
#include <hbaction.h>
#include <hbtoolbar.h>
#include "btcpuibaseview.h"

class HbLabel;
class HbPushButton;
class HbIcon;
class HbDocumentLoader;
class HbListView;
class HbDataFormModel;
class HbDataFormModelItem;
class CpSettingFormItemData;


class BtCpUiSearchView : public BtCpUiBaseView
{
    Q_OBJECT
    
public:
    explicit BtCpUiSearchView(BtuiModel &model, QGraphicsItem *parent = 0);
    virtual ~BtCpUiSearchView();
    virtual void activateView( const QVariant& value, int cmdId );
    virtual void deactivateView();
    virtual void setSoftkeyBack();
    
public slots:
    void deviceSelected(const QModelIndex& modelIndex);
    virtual void switchToPreviousView();

private:
    HbDocumentLoader *mLoader;
    HbLabel *mDeviceIcon;
    HbLabel *mDeviceName;
    HbListView *mDeviceList;
    
    // data structures for switching between views
    bool mEventFilterInstalled;
    int mAutoCmdId;
    Qt::Orientation mOrientation;
    
    HbMainWindow* mMainWindow;
    BtCpUiBaseView* mMainView;
    BtCpUiBaseView* mSearchView;
    HbAction *mSoftKeyBackAction;
//    CpCustomLabelViewItem*      mLabelItem;
//    CpCustomListViewItem*       mListItem;
    HbToolBar*                  mToolBar;
    HbAction*                   mViewBy;
    HbAction*                   mStop;
    HbAction*                   mRetry;
    HbDataFormModel*            mModel;    
};

#endif//	BTCPUISEARCHVIEW_H
