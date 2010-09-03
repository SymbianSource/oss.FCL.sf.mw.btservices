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


#ifndef BTXQPOWERSERVICE_H
#define BTXQPOWERSERVICE_H

#include <xqserviceprovider.h>
#include <btserversdkcrkeys.h>
#include <btengsettings.h>

class HbDeviceDialog;


class BtxqPowerService: public XQServiceProvider, public MBTEngSettingsObserver
    {
    Q_OBJECT
    
public:
    BtxqPowerService(QObject *parent = 0);
    ~BtxqPowerService();
    
public slots:
    void togglePower();
    void handleDialogClosed();
    void handleDataReceived(QVariantMap data);
    
private:
    void changeBtPowerMode(TBTPowerStateValue aNewPowerState);
    bool checkOfflineMode();
    void showBTStatusNotification(TBool powerState);
    void showOfflineDialog();
    
private: // from MBTEngSettingsObserver
    void PowerStateChanged( TBTPowerStateValue aState );
    void VisibilityModeChanged( TBTVisibilityMode aState );
    
private:
    //CBtToggleDi* mBtToggle;
    CBTEngSettings* mSettings;
    int mCurrentRequestIndex;
    HbDeviceDialog* mNotificationDialog;
    HbDeviceDialog* mOfflineDialog;
    bool            mOfflineQueryShown;
    };

#endif 
