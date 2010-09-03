/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BtDeviceDialogWidget class declaration.
*
*/


#ifndef BTDEVICEDIALOGQUERYWIDGET_H
#define BTDEVICEDIALOGQUERYWIDGET_H

#include <QVariantMap>

#include <hbdevicedialoginterface.h>
#include <hbdevicedialog.h>
#include <hbmessagebox.h>

/*!
    \class BtDeviceDialogQueryWidget
    \brief Widget class with properties setting. 

    BtDeviceDialogQueryWidget, inherited from HbDeviceDialogInterface, 
    implemented using HbMessageBox. 
    
 */
class BtDeviceDialogQueryWidget :
    public QObject, public HbDeviceDialogInterface
{
    Q_OBJECT
    
public:
    BtDeviceDialogQueryWidget(HbMessageBox::MessageBoxType type, const QVariantMap &parameters);
    
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
    Q_DISABLE_COPY(BtDeviceDialogQueryWidget)

    int mLastError;
    int mSendAction;
    bool mShowEventReceived;
    QString mDisplayPasskeyStatus;
    bool mSendPasskeyEntryCompletedAction;
    int mDispPassKeyNumDigit;
    
    HbMessageBox *mMessageBox;
};

#endif // BTDEVICEDIALOGQUERYWIDGET_H
