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

#ifndef	BTCPUIDEVICEVIEW_H
#define	BTCPUIDEVICEVIEW_H

#include <hbaction.h>
#include <hbtoolbar.h>
#include "btcpuibaseview.h"

class HbGroupBox;
class HbLabel;
class HbLineEdit;
class HbPushButton;
class HbIcon;
class HbDocumentLoader;
class HbDataForm;

class HbDataFormModel;
class HbDataFormModelItem;
class CpSettingFormItemData;
class BtAbstractDelegate;
class BtCpUiDeviceDetail;

class BtcpuiDeviceView : public BtcpuiBaseView
{
    Q_OBJECT
    
public:
    explicit BtcpuiDeviceView(BtSettingModel &settingModel, 
            BtDeviceModel &deviceModel,            
            QGraphicsItem *parent = 0);
    
    virtual ~BtcpuiDeviceView();
    virtual void activateView( const QVariant& value, bool backNavi);
    virtual void deactivateView();
        
public slots:
    
    void backToPreviousView();
    void changeOrientation( Qt::Orientation orientation );
    void updateDeviceData();
    void changeBtDeviceName();
    void pairDevice();
    void unpairDevice();
    void connectDevice();
    void disconnectDevice();
    void pairDelegateCompleted(int status);
    void unpairDelegateCompleted(int status);
    void connectDelegateCompleted(int status);
    void disconnectDelegateCompleted(int status);
    void changeDevNameDelegateCompleted(int status);
    void handleDeviceSetting();
    void handleDeviceSettingsChange(bool status);
    void connectionPreferenceChanged(int index);
    
private:
    enum connectionSelection {
        ConnectionAutomatic = 0,
        ConnectionAlwaysAsk, 
        ConnectionBlocked
    };
    void clearViewData();
    void setDeviceCategory(int cod, int majorRole);//cod:class of device
    void setDeviceStatus(int majorRole);
    void setConnectionCombobox();
    void setTextAndVisibilityOfButtons(int majorProperty);
    void loadDeviceDetails();
    void unloadDeviceDetails();
    void setPrevBtDeviceName();
    void setDeviceAuthorised();
    void setDeviceAlwaysAsk();
    void setDeviceBlocked();
    void updateButton(HbPushButton *button, const QString &iconName, const QString &text);
    
private:
    HbDocumentLoader *mLoader;
    HbLabel *mDeviceIcon;
    HbLineEdit *mDeviceName;
    HbLabel *mDeviceCategory;
    HbLabel *mDeviceStatus;
    
    HbDataForm *mConnectionDataForm;
    HbDataFormModel *mConnectionComboboxModel;
    HbDataFormModelItem *mCombobox;
    
    HbPushButton *mPair_Unpair;
    HbPushButton *mConnect_Disconnect;
    HbPushButton *mDeviceSetting;
    
    QModelIndex mDeviceIndex;
    QVariant mDeviceBdAddr;

    int mComboboxIndex;
    
    BtAbstractDelegate* mAbstractDelegate;

    BtCpUiDeviceDetail* mDeviceDetail;
    
};

#endif
