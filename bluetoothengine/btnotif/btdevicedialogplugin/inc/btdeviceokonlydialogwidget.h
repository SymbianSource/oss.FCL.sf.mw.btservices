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
* Description:  BtDeviceOkOnlyDialogWidget class declaration.
*
*/


#ifndef BTDEVICEOKONLYDIALOGWIDGET_H
#define BTDEVICEOKONLYDIALOGWIDGET_H

#include <QVariantMap>

#include <hbdevicedialoginterface.h>
#include <hbdevicedialog.h>
#include <hbmessagebox.h>

/*!
    \class BtDeviceOkOnlyDialogWidget
    \brief Widget class with properties setting. 

    BtDeviceOkOnlyDialogWidget, inherited from HbDeviceDialogInterface, 
    implemented using HbMessageBox. 
    
 */
class BtDeviceOkOnlyDialogWidget :
    public QObject, public HbDeviceDialogInterface
{
    Q_OBJECT
    
public:
    BtDeviceOkOnlyDialogWidget(HbMessageBox::MessageBoxType type, const QVariantMap &parameters);
    
    // From base class HbDeviceDialogInterface
    virtual bool setDeviceDialogParameters(const QVariantMap &parameters);
    virtual int deviceDialogError() const;
    virtual void closeDeviceDialog(bool byClient);
    virtual HbDialog *deviceDialogWidget() const;
    virtual QObject *signalSender() const;
    
signals: 
    // Required by the framework
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);

public slots:
    void messageBoxClosed(HbAction*);
    
private:
    void processParam(const QVariantMap &parameters);
    bool constructQueryDialog(const QVariantMap &parameters);
    void resetProperties();
    QString& GetPasskeyEntryStatusString(int aStatus);
        
private:
    Q_DISABLE_COPY(BtDeviceOkOnlyDialogWidget)

    int mLastError;
    int mSendAction;
    bool mShowEventReceived;
    
    HbMessageBox *mMessageBox;
};

#endif // BTDEVICEOKONLYDIALOGWIDGET_H
