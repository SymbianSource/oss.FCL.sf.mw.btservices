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
* Description:  Main View of BT Application
*
*/

#ifndef BTCPUIMAINVIEW_H
#define BTCPUIMAINVIEW_H

#include "btcpuibaseview.h"
#include <btqtconstants.h>
#include <btuimodelsortfilter.h>

class HbLabel;
class HbLineEdit;
class HbPushButton;
class HbIcon;
class HbComboBox;
class HbDocumentLoader;
class HbGridView;
class BtAbstractDelegate;
class HbListView;
class HbGroupBox;

class BtCpUiMainView : public BtCpUiBaseView
{
    Q_OBJECT

public:
    explicit BtCpUiMainView(
            BtSettingModel &settingModel, 
            BtDeviceModel &deviceModel, 
            QGraphicsItem *parent = 0 );
    ~BtCpUiMainView();
    // from view manager
    void createViews();

    Qt::Orientation  orientation();

    // from base class BtCpUiBaseView
    virtual void setSoftkeyBack();
    virtual void activateView( const QVariant& value, int cmdId );
    virtual void deactivateView();

public slots: 
    void changeOrientation( Qt::Orientation orientation );
    void changePowerState();
    
    void updateSettingItems(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void deviceSelected(const QModelIndex& modelIndex);
    void goToDiscoveryView();
    void goToDeviceView(const QModelIndex& modelIndex);
    
    // from view manager
    void changeView(int targetViewId, bool fromBackButton, int cmdId, const QVariant& value = 0 );
    virtual void switchToPreviousView();
    
    void visibilityChanged (int index);
    void changeBtLocalName();
    
    //from delegate classes
    void powerDelegateCompleted(int status);
    void visibilityDelegateCompleted(int status);
    void btNameDelegateCompleted(int status, QVariant param);
    void allActionTriggered();
    void pairActiontriggered();
    
protected:

private:
    enum filterType {
        BtuiAll = 0,
        BtuiPaired
    };
    
private:
    VisibilityMode indexToVisibilityMode(int index);
    int visibilityModeToIndex(VisibilityMode mode);
    BtCpUiBaseView * idToView(int targetViewId);
    
    //Functions to set the Previous Local settings in case of error
    void setPrevBtLocalName();
    void setPrevVisibilityMode();
    
    void updateDeviceListFilter(BtCpUiMainView::filterType filter);
    
private:

    HbDocumentLoader *mLoader;

    HbLineEdit *mDeviceNameEdit;
    HbPushButton *mPowerButton;
    HbComboBox *mVisibilityMode;
    HbListView *mDeviceList;

    Qt::Orientation mOrientation;
    
    // from view manager
    HbMainWindow* mMainWindow;
    BtCpUiBaseView* mMainView;
    BtCpUiBaseView* mDeviceView;
    BtCpUiBaseView* mSearchView;
    BtCpUiBaseView* mCurrentView;
    int mCurrentViewId;
    QList<int> mPreviousViewIds;
    
    //poiter to abstract delegate, and it is instantiated at runtime
    BtAbstractDelegate* mAbstractDelegate;
    BtuiModelSortFilter *mMainFilterModel;
    
    HbAction *mAllAction;
    HbAction *mPairAction;
    HbGroupBox *mGroupBox;
};
#endif // BTCPUIMAINVIEW_H 
