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


#ifndef BTDEVICESEARCHDIALOGWIDGET_H
#define BTDEVICESEARCHDIALOGWIDGET_H

#include <QObject>
#include <QVariantMap>
#include <hbdialog.h>
#include <hbdevicedialoginterface.h>
#include <hbpopup.h>
#include <hbdocumentloader.h>
#include <qstandarditemmodel.h>
#include <hbradiobuttonlist.h>
#include <hblistwidget.h>


struct device
    {
    QString mDeviceName;
    QString mDeviceType;
    int     mDeviceIdx;
    };

class BTDeviceSearchDialogWidget : public HbDialog,
                                public HbDeviceDialogInterface
    {
    Q_OBJECT
    
public:
    BTDeviceSearchDialogWidget(const QVariantMap &parameters);
    ~BTDeviceSearchDialogWidget();
    
public: // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    
public slots:
    void stopClicked();
    void retryClicked();
//    void viewByClicked();
    void deviceSelected(const QModelIndex& modelIndex);
//    void viewByItemSelected(int index);
    
private:
    bool constructDialog(const QVariantMap &parameters);
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    void appendToDeviceList(const QString deviceName);
    bool appendToDeviceTypeList(const QString deviceType);
    QIcon icon();
    
signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap  data);
    
private:
    HbDocumentLoader *mLoader;
    
    /**
     * 
     * item model for content list view.
     */
    
    QStandardItemModel* mContentItemModel;
    HbDialog*           mViewByDialog;
    HbRadioButtonList*  mRbl;
    QList<QString>      mDeviceTypeList;
    QList<device>       mDeviceList;
    HbListView*         mListView;
    bool                mViewByChosen;
    QList<device>       mDeviceLstOfType;
    int                 mDeviceLstIdx;
    int                 mSelectedType;
    int                 mDeviceDialogData;
    
    Q_DISABLE_COPY(BTDeviceSearchDialogWidget)
    };

#endif /* BTDEVICESEARCHDIALOGWIDGET_H */
