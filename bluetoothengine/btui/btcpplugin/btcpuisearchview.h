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

#include "btcpuibaseview.h"

class HbLabel;
class HbPushButton;
class HbIcon;
class HbDocumentLoader;
class HbListView;
class BtAbstractDelegate;
class HbGroupBox;
class HbDataForm;

class BtcpuiSearchView : public BtcpuiBaseView
{
    Q_OBJECT
    
public:

    explicit BtcpuiSearchView(BtSettingModel &settingModel, 
            BtDeviceModel &deviceModel, 
            QGraphicsItem *parent = 0);
    virtual ~BtcpuiSearchView();
    virtual void activateView( const QVariant& value, bool backNavi);
    virtual void deactivateView();
    virtual void createContextMenuActions(int majorRole);
public slots:
    void changeOrientation( Qt::Orientation orientation );
    void stopSearching();
    void retrySearch();
    virtual void viewByDialogClosed(HbAction* action);
    void deviceSearchCompleted(int error);
        
private:
    virtual void take(BtAbstractDelegate *delegate);
    void startSearchDelegate();
    
private:
    HbDocumentLoader *mLoader;
    HbLabel *mDeviceIcon;
    HbDataForm *mDataForm;
    HbLabel *mLabelSearching;        
    HbListView *mDeviceList;
    
    Qt::Orientation mOrientation;

    HbAction* mViewBy;
    HbAction* mStop;
    HbAction* mRetry;

};

#endif//	BTCPUISEARCHVIEW_H
