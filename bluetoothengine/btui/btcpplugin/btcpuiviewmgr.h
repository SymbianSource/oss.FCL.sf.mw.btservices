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

#ifndef BTCPUIVIEWMGR_H
#define BTCPUIVIEWMGR_H

#include <qglobal.h>

class BtcpuiBaseView;
class BtAbstractDelegate;

/*!
    \class BtcpuiViewMgr
    \brief the view management interface of Bluetooth CP plugin
 */
class BtcpuiViewMgr
{

public:

    /*!
     * Gets the device view instance.
     */
    virtual BtcpuiBaseView *deviceView() = 0;
    
    virtual void switchView(BtcpuiBaseView *from, BtcpuiBaseView *destination,
            const QVariant &init, bool backNavi) = 0;
    
};

#endif // BTCPUIVIEWMGR_H
