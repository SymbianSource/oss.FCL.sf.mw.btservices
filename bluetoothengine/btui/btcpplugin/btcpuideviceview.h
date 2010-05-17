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

#ifndef	BTCPUIDEVICEVIEW_H
#define	BTCPUIDEVICEVIEW_H

#include <cpbasesettingview.h>
#include <hbaction.h>
#include <hbtoolbar.h>
#include "btcpuibaseview.h"

class HbGroupBox;
class HbLabel;
class HbTextEdit;
class HbPushButton;
class HbIcon;
class HbDocumentLoader;
class HbListView;
class HbDataFormModel;
class HbDataFormModelItem;
class CpSettingFormItemData;
class BtAbstractDelegate;


class BtCpUiDeviceView : public BtCpUiBaseView
{
    Q_OBJECT
    
public:
    explicit BtCpUiDeviceView(
            BtSettingModel &settingModel, 
            BtDeviceModel &deviceModel,            
            QGraphicsItem *parent = 0);
    virtual ~BtCpUiDeviceView();
    virtual void activateView( const QVariant& value, int cmdId );
    virtual void deactivateView();
    virtual void setSoftkeyBack();
    
public slots:
    
    virtual void switchToPreviousView();
    void updateDeviceData();
    void changeBtDeviceName();
    void pairUnpair();
    void connectDisconnect();
    void pairDelegateCompleted(int status);
    void unpairDelegateCompleted(int status);
    void connectDelegateCompleted(int status);
    void disconnectDelegateCompleted(int status);
 
private:
    void clearViewData();
    void pairDevice();
    void unpairDevice();
    void connectDevice();
    void disconnectDevice();
    void setDeviceCategory(int cod, int majorRole, int minorRole);//cod:class of device
    void setDeviceStatus(int majorRole);
    void setTextAndVisibilityOfButtons();

private:
    HbDocumentLoader *mLoader;
    HbGroupBox *mGroupBox;
    HbLabel *mDeviceIcon;
    HbTextEdit *mDeviceName;
    HbLabel *mDeviceCategory;
    HbLabel *mDeviceStatus;
    
    HbPushButton *mPair_Unpair;
    HbPushButton *mConnect_Disconnect;
    HbPushButton *mDeviceSetting;
    
    
    // data structures for switching between views
    bool mEventFilterInstalled;
    int mAutoCmdId;
    Qt::Orientation mOrientation;
    
    HbMainWindow* mMainWindow;
    BtCpUiBaseView* mMainView;
    //BtCpUiBaseView* mDeviceView;
    HbAction *mSoftKeyBackAction;
    
    QModelIndex mDeviceIndex;
    QVariant mDeviceBdAddr;
    
    //true -> device is paired; false -> device is unpaired
    bool mPairStatus;
    
    //true-> device is connected; false -> device is disconnected
    bool mConnectStatus;
    
	//true -> device is connectable
    //e.g. not possible to connect to a phone, but possible to connect to a headset
    bool mConnectable;
    
    BtAbstractDelegate* mAbstractDelegate;

    
};

#endif
