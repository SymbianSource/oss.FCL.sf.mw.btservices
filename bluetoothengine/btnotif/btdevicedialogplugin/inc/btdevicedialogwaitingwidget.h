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
* Description:  BtDeviceDialogWaitingWidget class declaration.
*
*/


#ifndef BTDEVICEDIALOGWAITINGWIDGET_H
#define BTDEVICEDIALOGWAITINGWIDGET_H

#include <QObject>
#include <QVariantMap>
#include <hbdialog.h>
#include <hbdevicedialoginterface.h>
#include <hbpopup.h>
#include <hbdocumentloader.h>

class BtDeviceDialogWaitingWidget : public QObject, 
                          public HbDeviceDialogInterface
    {
    Q_OBJECT
    
public:
    BtDeviceDialogWaitingWidget(const QVariantMap &parameters);
    ~BtDeviceDialogWaitingWidget();
    
public: // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    virtual QObject *signalSender() const;
    
private:
    bool constructDialog(const QVariantMap &parameters);
    
signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);
    
private:
    HbDocumentLoader    *mLoader;
    HbDialog            *mDialog;
    int                 mLastError;
    
    Q_DISABLE_COPY(BtDeviceDialogWaitingWidget)
    };

#endif /* BTDEVICEDIALOGWAITINGWIDGET */
