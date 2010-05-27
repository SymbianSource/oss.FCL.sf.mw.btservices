/*
* ============================================================================
*  Name        : btcpuibaseview.h
*  Part of     : BluetoothUI / btapplication       *** Info from the SWAD
*  Description : Declaration of the baseclass for all views in btapplication.
*
*  Copyright � 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.2
*/

#ifndef BTCPUIBASEVIEW_H
#define BTCPUIBASEVIEW_H

#include <hbview.h>
#include <qglobal.h>
#include <cpbasesettingview.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>

/*!
    \class BtUiBaseView
    \brief the class is the base class for all views in btapplication.

 */
class BtCpUiBaseView : public CpBaseSettingView
{
    Q_OBJECT

public:
    
    virtual ~BtCpUiBaseView();
    virtual void activateView( const QVariant& value, int cmdId ) = 0;
    virtual void deactivateView() = 0;    
    virtual void switchToPreviousView() = 0;
    
signals:

protected:
    explicit BtCpUiBaseView( 
            BtSettingModel &settingModel, 
            BtDeviceModel &deviceModel, 
            QGraphicsItem *parent = 0);
    
    virtual void setSoftkeyBack() = 0;

    BtSettingModel &getSettingModel();
    BtDeviceModel &getDeviceModel();
    
protected:
    
    // do not owned
    BtSettingModel *mSettingModel;
    
    //do not owned
    BtDeviceModel *mDeviceModel;
    
    QGraphicsItem *mParent;
    int mPreviousViewId;
    
    Q_DISABLE_COPY(BtCpUiBaseView)
};

#endif // BTCPUIBASEVIEW_H
