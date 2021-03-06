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


#ifndef BTMSGVIEWER_H
#define BTMSGVIEWER_H

#include <xqserviceprovider.h>
#include <hbview.h>
#include <xqappmgr.h>

#include "btmsgviewerutils.h"

class HbDeviceDialog;


class BTMsgViewer : public XQServiceProvider
{
    Q_OBJECT
    
public:
    BTMsgViewer (QObject* parent=0 );
    ~BTMsgViewer ();
    
public slots:
    void displaymsg(int messageId);
    
private slots:
    void handledialogClosed();

private:
    bool isError(int err);
    QString copyVCardToTemp(const QString& filepath);
    void deleteVCardFromTemp(const QString& filepath);
    void launchErrordDialog(int dialogTitle);
        
private:
    int mCurrentRequestIndex;
    HbDeviceDialog* mDialog;
    int mError;
};

#endif // BTMSGVIEWER_H
