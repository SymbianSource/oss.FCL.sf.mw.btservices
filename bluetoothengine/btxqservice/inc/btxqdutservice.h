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


#ifndef BTXQDUTSERVICE_H
#define BTXQDUTSERVICE_H

#include <xqserviceprovider.h>

class HbDeviceDialog;


class BtxqDutService : public XQServiceProvider
{
    Q_OBJECT
    
public:
    BtxqDutService( QObject *parent = 0 );
    ~BtxqDutService();

public slots:
    void activateDeviceTestMode();
    
public slots:
    void dutModeActivated();
    
private:
    HbDeviceDialog* mNotificationDialog;
    int mCurrentRequestIndex;
    int mError;
};

#endif /* BTXQDUTSERVICE_H */
