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
#include <btdelegateconsts.h>
#include "btcpuiviewmgr.h"

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
class HbDataForm;

class BtcpuiMainView : public BtcpuiBaseView, public BtcpuiViewMgr
{
    Q_OBJECT

public:
    
    explicit BtcpuiMainView(QGraphicsItem *parent = 0 );    
    
    explicit BtcpuiMainView(BtSettingModel &settingModel, 
            BtDeviceModel &deviceModel, 
            QGraphicsItem *parent = 0 );
    
    ~BtcpuiMainView();

    // from base class BtcpuiBaseView
    virtual void activateView( const QVariant& value, bool backNavi);
    
    virtual void deactivateView();

    virtual void createContextMenuActions(int majorRole);
    
public slots: 
    void changeOrientation( Qt::Orientation orientation );
    void changePowerState();
    void changeVisibility(int index);
    void changeBtLocalName();
    
    void updateSettingItems(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void launchDeviceDiscovery();
    
    void goToSearchView();

    //from delegate classes
    void visibilityDelegateCompleted(int status);
    void btNameDelegateCompleted(int status);
    void allActionTriggered();
    void pairActionTriggered(); 
    void menuActionTriggered(HbAction *action);
    virtual void viewByDialogClosed(HbAction* action);
    
    void disconnectAllDelegateCompleted(int status);
    void onRemoveQuestionDialogClosed(int action);
    void aboutToShowOptionsMenu();
    
    // returns the search view instance.
    BtcpuiBaseView *searchView();
    
    // From BtcpuiViewMgr
    BtcpuiBaseView *deviceView();
    
    void switchView(BtcpuiBaseView *from, BtcpuiBaseView *destination,
            const QVariant &init, bool backNavi);
    
private:
    enum filterType {
        BtuiAll = 0,
        BtuiPaired
    };
    
private:
    
    void loadDocument();
    
    VisibilityMode indexToVisibilityMode(int index);
    int visibilityModeToIndex(VisibilityMode mode);
    
    //Functions to set the Previous Local settings in case of error
    void setPrevBtLocalName();
    void setPrevVisibilityMode();
    void removeRegistryDevices(BtDelegate::EditorType type);
    
    void updateDeviceListFilter(BtcpuiMainView::filterType filter);
    
    void updateOptionsMenu();
    
private:

    // search view. Owned
    BtcpuiBaseView *mSearchView;
    // device view. Owned.
    BtcpuiBaseView *mDeviceView;
    
    HbDocumentLoader *mLoader;

    HbLineEdit *mDeviceNameEdit;
    HbPushButton *mPowerButton;
    HbComboBox *mVisibilityMode;
    HbGridView *mDeviceList;

    Qt::Orientation mOrientation;
    
    HbAction *mAllAction;
    HbAction *mPairAction;
    HbDataForm *mDataForm;
    HbAction* mRemovePairedDevices;
    HbAction* mRemoveDevices;
    HbMenu* mSubMenu;
    BtDelegate::EditorType mRemoveDevDelegateType;
    
};
#endif // BTCPUIMAINVIEW_H 
