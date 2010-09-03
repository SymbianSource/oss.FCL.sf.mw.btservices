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

#ifndef BTDELEGATECONSTS_H
#define BTDELEGATECONSTS_H

#include <QObject>

namespace BtDelegate
    {
    enum EditorType
        {
        InvalidEditor =            0x000000, // invalid editor type
        ManagePower =              0x000001,
        ChangeLocalName =          0x000002,
        ChangeVisibility =         0x000004,
        Inquiry =                  0x000008,
        ConnectService =           0x000010,
        DisconnectService =        0x000020,
        DisconnectAllConnections = 0x000040,
        PairDevice =               0x000080,
        UnpairDevice =             0x000100,
        BlockDevice =              0x000200,
        UnblockDevice =            0x000400,
        TrustDevice =              0x000800,
        UntrustDevice =            0x001000,
        ChangeDeviceFriendlyName = 0x002000,
        RemoveAllRegistryDevices = 0x004000,
        RemovePairedDevices =      0x008000,
        RemoveBlockedDevices =     0x010000,
        };
    }

#endif // BTDELEGATECONSTS_H
