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


#ifndef BTSENDDIALOGWIDGET_H
#define BTSENDDIALOGWIDGET_H

#include <QVariantMap>
#include <hbdialog.h>
#include <hbdevicedialoginterface.h>
#include <hbdocumentloader.h>
#include <hbprogressbar.h>
#include <hblabel.h>
#include <hbaction.h>


class BTSendDialogWidget : public QObject,
                                public HbDeviceDialogInterface
    {
    Q_OBJECT
    
public:
    BTSendDialogWidget(const QVariantMap &parameters);
    ~BTSendDialogWidget();
    
public: // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    virtual QObject *signalSender() const;
    
public slots:
    void cancelClicked();
private:
    void constructDialog(const QVariantMap &parameters);
    
signals:
    void deviceDialogClosed();
    
private:
    HbDocumentLoader *mLoader;
    HbProgressBar    *mProgressBar;
    HbLabel *mDialogHeading;
    HbLabel *mFileIconLabel;
	HbLabel	*mFileNameLabel;
	HbLabel	*mFileSizeLabel;
    HbDialog *mSendDialog;
    HbAction *mHideAction;
    HbAction *mCancelAction;
    int       mFileIndex;
    int 			mLastError;

    
    Q_DISABLE_COPY(BTSendDialogWidget)
    };

#endif /* BTSENDDIALOGWIDGET_H */
