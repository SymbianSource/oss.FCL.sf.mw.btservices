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
#include "btdelegatepower.h"
#include "btuimodel.h"
#include "btdelegatedevname.h"
#include "btdelegatevisibility.h"


/*!
    Constructor.
 */
BtAbstractDelegate * BtDelegateFactory::newDelegate(
        BtDelegate::Command cmd, BtuiModel& model, QObject *parent )
{
    switch ( cmd ) {
        case BtDelegate::ManagePower:
            return new BtDelegatePower( model, parent );
        case BtDelegate::DeviceName:
            return new BtDelegateDevName( model, parent );
        case BtDelegate::Visibility:
                    return new BtDelegateVisibility( model, parent );
    }
    return 0;
}


