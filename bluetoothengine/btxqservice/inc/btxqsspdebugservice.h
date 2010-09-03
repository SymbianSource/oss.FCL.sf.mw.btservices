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


#ifndef BTXQSSPDEBUGSERVICE_H_
#define BTXQSSPDEBUGSERVICE_H_

#include <xqserviceprovider.h>


class BtxqSspDebugService : public XQServiceProvider
{
    Q_OBJECT
    
public:
    BtxqSspDebugService( QObject *parent = 0 );
    ~BtxqSspDebugService();

public slots:
    void toggleSspDebugMode();
    
private:
    int mCurrentRequestIndex;
    int mError;
};

#endif /* BTXQSSPDEBUGSERVICE_H_ */
