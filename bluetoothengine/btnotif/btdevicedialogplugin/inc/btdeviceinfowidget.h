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


#ifndef BTDEVICEINFOWIDGET_H
#define BTDEVICEINFOWIDGET_H

#include <QObject>
#include <QVariantMap>
#include <hbdevicedialoginterface.h>
#include <hbmessagebox.h>


class BtDeviceInfoWidget : public QObject, public HbDeviceDialogInterface
    {
    Q_OBJECT
    
public:
    BtDeviceInfoWidget(const QVariantMap &parameters);
    ~BtDeviceInfoWidget();
    
public: // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    QObject* signalSender() const;
            
signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);

private:
    bool constructDialog(const QVariantMap &parameters);
    
public slots:
    void messageBoxClosed(HbAction *action);
    void messageBoxClosed(int action);
    
private:
    HbMessageBox* mMessageBox;
    int mLastError;
    
    Q_DISABLE_COPY(BtDeviceInfoWidget)
    };

#endif /* BTDEVICEINFOWIDGET_H */
