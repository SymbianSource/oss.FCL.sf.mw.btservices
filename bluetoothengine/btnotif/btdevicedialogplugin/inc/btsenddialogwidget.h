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


#ifndef BTSENDDIALOGWIDGET_H_
#define BTSENDDIALOGWIDGET_H_

#include <QObject>
#include <QVariantMap>
#include <hbdialog.h>
#include <hbdevicedialoginterface.h>
#include <hbpopup.h>
#include <hbdocumentloader.h>
#include <qstandarditemmodel.h>
#include <hbprogressbar.h>
#include <hblabel.h>
#include <hblistview.h>


class BTSendDialogWidget : public HbDialog,
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
//    void hideClicked();
//    void cancelClicked();
    void inputClosed(HbAction* action);
private:
    bool constructDialog(const QVariantMap &parameters);
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    
signals:
    void deviceDialogClosed();
    
private:
    HbDocumentLoader *mLoader;
    QStandardItemModel* mContentItemModel;
    HbProgressBar*      mProgressBar;
    HbLabel*            mLabel;
    HbListView*         mListView;
    int                 mFileIndex;
    
    Q_DISABLE_COPY(BTSendDialogWidget)
    };

#endif /* BTSENDDIALOGWIDGET_H_ */
