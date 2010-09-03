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


#ifndef BTXQADDRSERVICE_H
#define BTXQADDRSERVICE_H

#include <xqserviceprovider.h>

class HbDeviceDialog;


class BtxqAddrService : public XQServiceProvider
{
    Q_OBJECT
    
public:
    BtxqAddrService( QObject *parent = 0 );
    ~BtxqAddrService();

public slots:
    void showBTAddr();
    
public slots:
    void handleshowBTAddrCompleted();
    
private:
    HbDeviceDialog* mDialog;
    int mCurrentRequestIndex;
    int mError;
};

#endif // BTXQADDRSERVICE_H
