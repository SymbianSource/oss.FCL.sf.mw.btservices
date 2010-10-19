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

#include "btmoredevicesdialogwidget.h"
#include <hblistview.h>
#include <btuidevtypemap.h>
#include <btuiiconutil.h>
#include <bluetoothdevicedialogs.h>
#include "btdevicedialogpluginerrors.h"

const char* DOCML_BT_MORE_DEV_DIALOG = ":/docml/bt-more-devices-dialog.docml";


BTMoreDevicesDialogWidget::BTMoreDevicesDialogWidget(const QVariantMap &parameters)
{
    mLoader = 0;
    mContentItemModel = 0;
    mLastError = NoError;
    constructDialog(parameters);
}

BTMoreDevicesDialogWidget::~BTMoreDevicesDialogWidget()
{
    delete mLoader;
    delete mContentItemModel;
}

bool BTMoreDevicesDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    int cod  = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass)).toInt();
    int uiMajorDevice;
    int uiMinorDevice;

    if(!mLastError)
    {
        BtuiDevProperty::mapDeiveType(uiMajorDevice, uiMinorDevice, cod);
        if ((uiMajorDevice & BtuiDevProperty::Phone)||(uiMajorDevice & BtuiDevProperty::Computer) )
        {
            BtSendDataItem devData;
            //todo Need to create string constant for name as enum EDeviceName has an issue
            devData[NameAliasRole] = parameters.value("Name");
            devData[ReadableBdaddrRole] = parameters.value(QString::number(TBluetoothDeviceDialog::EAddress));
            devData[CoDRole] = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass));
            
            setMajorProperty(devData,BtuiDevProperty::Bonded,
                    parameters.value("Bonded").toBool());
            setMajorProperty(devData,BtuiDevProperty::Blocked,
                    parameters.value("Blocked").toBool());
            setMajorProperty(devData,BtuiDevProperty::Trusted,
                    parameters.value("Trusted").toBool());
            setMajorProperty(devData,BtuiDevProperty::Connected,
                    parameters.value("Connected").toBool());
            mData.append(devData);
            
            QStandardItem* listitem = new QStandardItem();
            QStringList info;
            info.append(devData[NameAliasRole].toString());
    
            listitem->setData(info, Qt::DisplayRole);
            HbIcon icon =  getBadgedDeviceTypeIcon(devData[CoDRole].toInt(),
                    devData[MajorPropertyRole].toInt(),
                    BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight);
            listitem->setIcon(icon.qicon());
            
            mContentItemModel->appendRow(listitem);        
        }
        return true;
    }
    else
    {
        return false;
    }
}




int BTMoreDevicesDialogWidget::deviceDialogError() const
{
    return mLastError;
}

void BTMoreDevicesDialogWidget::closeDeviceDialog(bool byClient)
{
    mLastUsedDeviceDialog->close();
    if(!byClient)
        {
        QVariantMap val;
        QVariant index(-1);
        val.insert("selectedindex",index);
        emit deviceDialogData(val);
        }
    emit deviceDialogClosed();
}

HbPopup* BTMoreDevicesDialogWidget::deviceDialogWidget() const
{
    return mLastUsedDeviceDialog;
}

QObject* BTMoreDevicesDialogWidget::signalSender() const
{
    return const_cast<BTMoreDevicesDialogWidget*>(this);
}

void BTMoreDevicesDialogWidget::constructDialog(const QVariantMap &parameters)
{
    Q_UNUSED(parameters);
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_MORE_DEV_DIALOG, &ok);
    if(ok)
    {
        mLastUsedDeviceDialog = qobject_cast<HbDialog*>(mLoader->findWidget("lastUsedDevicesDialog"));
  
        HbListView* deviceList = qobject_cast<HbListView*>(mLoader->findWidget("deviceList"));
        deviceList->setSelectionMode(HbAbstractItemView::SingleSelection);

        mContentItemModel = new QStandardItemModel(this);
        deviceList->setModel(mContentItemModel);
        
        connect(deviceList, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
        
        mMoreAction = static_cast<HbAction*>( mLoader->findObject( "moreAction" ) );
        
        mCancelAction = static_cast<HbAction*>( mLoader->findObject( "cancelAction" ) );
        
        connect(mMoreAction, SIGNAL(triggered()), this, SLOT(moreDevicesClicked()));
        connect(mCancelAction, SIGNAL(triggered()), this, SLOT(cancelClicked()));
        mLastUsedDeviceDialog->setBackgroundFaded(false);
        mLastUsedDeviceDialog->setDismissPolicy(HbPopup::NoDismiss);
        mLastUsedDeviceDialog->setTimeout(HbPopup::NoTimeout);
        mLastUsedDeviceDialog->setAttribute(Qt::WA_DeleteOnClose);
    }
    else
    {
        mLastError = DocMLLoadingError;			    		
    }
}


void BTMoreDevicesDialogWidget::moreDevicesClicked()
{
    QVariantMap val;
    QVariant index("MoreDevices");
    val.insert("MoreDevices",index);    
    emit deviceDialogData(val);
    emit deviceDialogClosed();
}

void BTMoreDevicesDialogWidget::cancelClicked()
{
    mLastUsedDeviceDialog->close();
    QVariantMap val;
    QVariant index(-1);
    val.insert("selectedindex",index);
    emit deviceDialogData(val);    
    emit deviceDialogClosed();
}

void BTMoreDevicesDialogWidget::deviceSelected(const QModelIndex& modelIndex)
{
    int row = modelIndex.row();
    QVariantMap val;
    const BtSendDataItem& qtdev = mData.at(row);
    val.insert("selectedindex",QVariant(row));
    val.insert("devicename",QVariant(qtdev[NameAliasRole]));
    val.insert("deviceaddress",QVariant(qtdev[ReadableBdaddrRole]));
    val.insert("deviceclass",QVariant(qtdev[CoDRole]));
    emit deviceDialogData(val);
}
 
