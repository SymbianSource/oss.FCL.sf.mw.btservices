/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <QStringListModel>

class HbLabel;
class HbLineEdit;
class HbPushButton;
class HbIcon;
class HbComboBox;
class HbDocumentLoader;
class HbGridView;
class BtAbstractDelegate;


class BtCpUiMainView : public BtCpUiBaseView
{
    Q_OBJECT

public:
    enum ViewIndex {
        MainView,
        SearchView, 
        DeviceView,
        LastView
    };
    explicit BtCpUiMainView( BtuiModel &model, QGraphicsItem *parent = 0 );
    ~BtCpUiMainView();
    // from view manager
    void createViews();

    Qt::Orientation  orientation();

    // from base class BtCpUiBaseView
    virtual void setSoftkeyBack();
    virtual void activateView( const QVariant& value, int cmdId );
    virtual void deactivateView();

public slots: 
    void commandCompleted( int cmdId, int err, const QString &diagnostic );
    void changeOrientation( Qt::Orientation orientation );
    void itemActivated(QModelIndex index); 
    void changePowerState();
    void updateSettingItems(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    
    void goToDiscoveryView();
    
    // from view manager
    void changeView(int targetViewId, bool fromBackButton, int cmdId, const QVariant& value = 0 );
    void switchToPreviousViewReally();
    virtual void switchToPreviousView();
    
    void visibilityChanged (int index);
    void changeBtLocalName();
    
    //from delegate classes
    void powerDelegateCompleted(int status);
    void visibilityDelegateCompleted(int status);
    void btNameDelegateCompleted(int status, QVariant param);
    
protected:

    
private:
    VisibilityMode indexToVisibilityMode(int index);
    int visibilityModeToIndex(VisibilityMode mode);
    BtCpUiBaseView * idToView(int targetViewId);
    
    //Functions to set the Previous Local settings in case of error
    void setPrevBtLocalName();
    void setPrevVisibilityMode();
    
private:
    QAbstractItemModel* mSubModel;
    HbDocumentLoader *mLoader;
    HbLineEdit *mDeviceNameEdit;
    HbPushButton *mPowerButton;
    HbComboBox *mVisibilityMode;
    QStringListModel *mVisiListModel;
    HbGridView *mDeviceList;
    
    // data structures for switching between views
    bool mEventFilterInstalled;
    int mAutoCmdId;
    Qt::Orientation mOrientation;
    
    // from view manager
    HbMainWindow* mMainWindow;
    BtCpUiBaseView* mMainView;
    BtCpUiBaseView* mDeviceView;
    BtCpUiBaseView* mSearchView;
    BtCpUiBaseView* mCurrentView;
    int mCurrentViewId;
    HbAction *mBackAction;
    QList<int> mPreviousViewIds;
    
    //poiter to abstract delegate, and it is instantiated at runtime
    BtAbstractDelegate* mAbstractDelegate;
    
};
#endif // BTCPUIMAINVIEW_H 
