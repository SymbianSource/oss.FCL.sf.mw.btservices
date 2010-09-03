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
* Description: 
*
*/


#include "btdelegatefactory.h"
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include "btdelegatepower.h"
#include "btdelegatedevname.h"
#include "btdelegateremotedevname.h"
#include "btdelegatevisibility.h"
#include "btdelegateinquiry.h"
#include "btdelegateconnect.h"
#include "btdelegatepair.h"
#include "btdelegatedisconnect.h"
#include "btdelegatedevsecurity.h"
#include "btdelegatedevremove.h"

/*!
    Constructor.
 */
BtAbstractDelegate *BtDelegateFactory::newDelegate(
        BtDelegate::EditorType editor,             
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, 
        QObject *parent )
{
    switch ( editor ) {
        case BtDelegate::ManagePower:
            return new BtDelegatePower( settingModel, deviceModel, parent );
        case BtDelegate::ChangeLocalName:
            return new BtDelegateDevName( parent );
        case BtDelegate::ChangeVisibility:
            return new BtDelegateVisibility( parent );
        case BtDelegate::Inquiry:
            return new BtDelegateInquiry( settingModel, deviceModel, parent );
        case BtDelegate::ConnectService:
            return new BtDelegateConnect( settingModel, deviceModel, parent );
        case BtDelegate::DisconnectService:
        case BtDelegate::DisconnectAllConnections:
            return new BtDelegateDisconnect( settingModel, deviceModel, parent );
        case BtDelegate::PairDevice:
            return new BtDelegatePair( settingModel, deviceModel, parent );
        case BtDelegate::UnpairDevice:
        case BtDelegate::BlockDevice:
        case BtDelegate::UnblockDevice:
        case BtDelegate::TrustDevice:
        case BtDelegate::UntrustDevice:
            return new BtDelegateDevSecurity( settingModel, deviceModel, parent );
        case BtDelegate::ChangeDeviceFriendlyName:
            return new BtDelegateRemoteDevName( settingModel, deviceModel, parent );
        case BtDelegate::RemoveAllRegistryDevices:
        case BtDelegate::RemovePairedDevices:
        case BtDelegate::RemoveBlockedDevices:
            return new BtDelegateDevRemove( settingModel, deviceModel, parent );
    }
    return 0;
}


