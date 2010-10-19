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


#include "btdevicesearchdialogwidget.h"

#include <hblistview.h>
#include <hbtoolbar.h>


#include <qstandarditemmodel.h>
#include <hbaction.h>
#include <xqconversions.h>
#include <qtranslator.h>
#include <qcoreapplication.h>
#include <bluetoothdevicedialogs.h>
#include <btuidevtypemap.h>
#include <btuiiconutil.h>
#include "btdevicedialogpluginerrors.h"

const char* DOCML_BTDEV_SEARCH_DIALOG = ":/docml/bt-device-search-dialog.docml";


#define LOC_SEARCHING_DEVICE hbTrId("txt_bt_subhead_searching")
#define LOC_SEARCH_DONE hbTrId("txt_bt_subhead_search_done")
#define LOC_SEARCH_STOP hbTrId("txt_common_button_stop")
#define LOC_SEARCH_RETRY hbTrId("txt_common_button_retry")
#define LOC_SHOW_DIALOG_TITLE hbTrId("txt_bt_title_show")


BTDeviceSearchDialogWidget::BTDeviceSearchDialogWidget(const QVariantMap &parameters)
{
    mSelectedDeviceType = 0;
    mLoader = 0;
    mContentItemModel = 0;
    mStopRetryFlag = 0; // Stop 
    mQuery = 0;
    mLastError = NoError;
    mSelectedDeviceType |= (BtuiDevProperty::AVDev | BtuiDevProperty::Computer |
            BtuiDevProperty::Phone | BtuiDevProperty::Peripheral |
            BtuiDevProperty::LANAccessDev | BtuiDevProperty::Toy |
            BtuiDevProperty::WearableDev | BtuiDevProperty::ImagingDev |
            BtuiDevProperty::HealthDev | BtuiDevProperty::UncategorizedDev);       
    constructDialog(parameters);
}

BTDeviceSearchDialogWidget::~BTDeviceSearchDialogWidget()
{
    delete mLoader;
    delete mContentItemModel;
    if ( mQuery )
    {
        delete mQuery;
    }
}

bool BTDeviceSearchDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    if(!mLastError)
    {
        if(parameters.keys().contains("Search Completed"))
        {
            mStopRetryFlag = 1; // Retry 
            mSearchLabel->hide();
            
            mSearchIconLabel->hide();
            
            mSearchDoneLabel->show();
            mSearchDoneLabel->setTextWrapping(Hb::TextWordWrap);
            mSearchDoneLabel->setAlignment(Qt::AlignLeft);
            mSearchDoneLabel->setPlainText(LOC_SEARCH_DONE);
            
            mStopRetryAction->setText(LOC_SEARCH_RETRY);
        }
        else
        {
            int cod  = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass)).toInt();
            int uiMajorDevice;
            int uiMinorDevice;
        
            BtuiDevProperty::mapDeiveType(uiMajorDevice, uiMinorDevice, cod);
    
            BtSendDataItem devData;
            //TODO Need to create string constant for Name as enum EDeviceName is not working for this
            devData[NameAliasRole] =parameters.value("Name").toString();
            devData[ReadableBdaddrRole] = parameters.value(QString::number(TBluetoothDeviceDialog::EAddress));
            devData[CoDRole] = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass));
            devData[DeviceTypeRole] = QVariant(uiMajorDevice);
            setMajorProperty(devData,BtuiDevProperty::Bonded,
                    parameters.value("Bonded").toBool());
            setMajorProperty(devData,BtuiDevProperty::Blocked,
                    parameters.value("Blocked").toBool());
            setMajorProperty(devData,BtuiDevProperty::Trusted,
                    parameters.value("Trusted").toBool());
            setMajorProperty(devData,BtuiDevProperty::Connected,
                    parameters.value("Connected").toBool());
            mData.append(devData);
            
            if(mSelectedDeviceType & devData[DeviceTypeRole].toInt())
            {
                QStandardItem* listitem = new QStandardItem();
                QStringList info;
                info.append(devData[NameAliasRole].toString());
                listitem->setData(info, Qt::DisplayRole);
                HbIcon icon =  getBadgedDeviceTypeIcon(devData[CoDRole].toInt(),
                        devData[MajorPropertyRole].toInt(),
                        BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight);
                listitem->setIcon(icon.qicon());
                mContentItemModel->appendRow(listitem);    
                mSelectedData.append(devData);
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

int BTDeviceSearchDialogWidget::deviceDialogError() const
{
    return mLastError;
}

void BTDeviceSearchDialogWidget::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    mSearchDevicesDialog->close();
    if(!byClient)
        {
        QVariantMap val;
        QVariant index(-1);
        val.insert("selectedindex",index);
        emit deviceDialogData(val);    
        }
    emit deviceDialogClosed();
}

HbPopup* BTDeviceSearchDialogWidget::deviceDialogWidget() const
{
    return mSearchDevicesDialog;
}

QObject* BTDeviceSearchDialogWidget::signalSender() const
{
    return const_cast<BTDeviceSearchDialogWidget*>(this);
}

void BTDeviceSearchDialogWidget::constructDialog(const QVariantMap &parameters)
{
    Q_UNUSED(parameters);
    bool ok = false;
    
    mLoader = new HbDocumentLoader();
    mLoader->load(DOCML_BTDEV_SEARCH_DIALOG, &ok);
    if(ok)
    {
        mSearchDevicesDialog = qobject_cast<HbDialog*>(mLoader->findWidget("searchDialog"));
        
        mSearchLabel = qobject_cast<HbLabel*>(mLoader->findWidget("searchLabel"));
        mSearchLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchLabel->setAlignment(Qt::AlignHCenter);
        mSearchLabel->setPlainText(LOC_SEARCHING_DEVICE);
 
        mSearchIconLabel = qobject_cast<HbLabel*>(mLoader->findWidget("searchIconLabel"));
        mSearchIconLabel->setIcon(HbIcon("qtg_large_bluetooth"));

        mSearchDoneLabel = qobject_cast<HbLabel*>(mLoader->findWidget("searchDoneLabel"));
        mSearchDoneLabel->hide();
        
        mDeviceList = qobject_cast<HbListView*>(mLoader->findWidget("deviceList"));
        mDeviceList->setSelectionMode(HbAbstractItemView::SingleSelection);

        mContentItemModel = new QStandardItemModel(this);
        mDeviceList->setModel(mContentItemModel);//, prototype);

        connect(mDeviceList, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
        
        mShowAction = static_cast<HbAction*>( mLoader->findObject( "viewByAction" ) );
        //if action is not disconnected the dialog will be closed when action is clicked 
        mShowAction->disconnect(mSearchDevicesDialog); 
        
        
        mStopRetryAction = static_cast<HbAction*>( mLoader->findObject( "stopRetryAction" ) );
        //if action is not disconnected the dialog will be closed when action is clicked         
        mStopRetryAction->disconnect(mSearchDevicesDialog); 
        
        connect(mShowAction, SIGNAL(triggered()), this, SLOT(viewByClicked()));
        connect(mStopRetryAction, SIGNAL(triggered()), this, SLOT(stopRetryClicked()));

        connect(mSearchDevicesDialog, SIGNAL(aboutToClose()), this, SLOT(searchDialogClosed()));
        mSearchDevicesDialog->setBackgroundFaded(false);
        mSearchDevicesDialog->setDismissPolicy(HbPopup::NoDismiss);
        mSearchDevicesDialog->setTimeout(HbPopup::NoTimeout);
        mSearchDevicesDialog->setAttribute(Qt::WA_DeleteOnClose);
        
        mDevTypeList << hbTrId("txt_bt_list_audio_devices")
                << hbTrId("txt_bt_list_computers") 
                << hbTrId("txt_bt_list_input_devices") 
                << hbTrId("txt_bt_list_phones") 
                << hbTrId("txt_bt_list_other_devices");
    }
    else
    {
        mLastError = DocMLLoadingError;			    		    		
    }

}


void BTDeviceSearchDialogWidget::stopRetryClicked()
{
    QVariantMap val;
    if(mStopRetryFlag == 1)
    {
        mStopRetryFlag = 0; // Stop 
        QVariant index("Retry");
        val.insert("Retry",index); 
        emit deviceDialogData(val);
        delete mContentItemModel;
        mContentItemModel = new QStandardItemModel(this);
        mDeviceList->setModel(mContentItemModel);
        mStopRetryAction->setText(LOC_SEARCH_STOP);
        
        mSearchLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchLabel->setAlignment(Qt::AlignHCenter);
        mSearchLabel->setPlainText(LOC_SEARCHING_DEVICE);
        
        mSearchIconLabel->setIcon(QIcon(QString(":/icons/qtg_large_bluetooth.svg")));     
        mSearchLabel->show();
        
        mSearchIconLabel->show();
        
        mSearchDoneLabel->hide();
    }
    else
    {
        mStopRetryFlag = 1; //Retry 
        mStopRetryAction->setText(LOC_SEARCH_RETRY);
        
        mSearchLabel->hide();
        
        mSearchIconLabel->hide();
        
        mSearchDoneLabel->show();
        mSearchDoneLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchDoneLabel->setAlignment(Qt::AlignLeft);
        mSearchDoneLabel->setPlainText(LOC_SEARCH_DONE);        
        
        QVariantMap val;
        QVariant index("Stop");
        val.insert("Stop",index); 
        emit deviceDialogData(val);    
    }
}

void BTDeviceSearchDialogWidget::viewByClicked()
{
    if ( !mQuery )
    {
        mQuery = new HbSelectionDialog;
        mQuery->setStringItems(mDevTypeList, 0);
        mQuery->setSelectionMode(HbAbstractItemView::MultiSelection);
    
        QList<QVariant> current;
        current.append(QVariant(0));
        mQuery->setSelectedItems(current);
 
        //todo need to check whether the dialog is destroyed without setting this flag
        //if not destoryed then set this flag in the destructor and then delete it
        
//        mQuery->setAttribute(Qt::WA_DeleteOnClose);
        // Set the heading for the dialog.
        HbLabel *headingLabel = new HbLabel(LOC_SHOW_DIALOG_TITLE, mQuery);
        mQuery->setHeadingWidget(headingLabel);
    }
    mQuery->open(this,SLOT(selectionDialogClosed(HbAction*)));
}

void BTDeviceSearchDialogWidget::searchDialogClosed() 
{
    QVariantMap val;
    QVariant index(-1);
    val.insert("selectedindex",index);
    emit deviceDialogData(val);    
    emit deviceDialogClosed();
}

void BTDeviceSearchDialogWidget::selectionDialogClosed(HbAction* action)
{
    disconnect( mQuery, 0, this, 0 ); 
    int devTypesWanted = 0;

    if (action == mQuery->actions().first())
    {  // user pressed "Ok"
        // Get selected items.
        QList<QVariant> selections;
        selections = mQuery->selectedItems();
        
        for (int i=0; i < selections.count(); i++) 
        {
            switch (selections.at(i).toInt()) 
            {
                case BtUiDevAudioDevice:
                    devTypesWanted |= BtuiDevProperty::AVDev;
                    break;
                case BtUiDevComputer:
                    devTypesWanted |= BtuiDevProperty::Computer;
                    break;
                case BtUiDevInputDevice:
                    devTypesWanted |= BtuiDevProperty::Peripheral;
                    break;
                case BtUiDevPhone:
                    devTypesWanted |= BtuiDevProperty::Phone;
                    break;
                case BtUiDevOtherDevice:
                    devTypesWanted |= (BtuiDevProperty::LANAccessDev |
                            BtuiDevProperty::Toy |
                            BtuiDevProperty::WearableDev |
                            BtuiDevProperty::ImagingDev |
                            BtuiDevProperty::HealthDev |
                            BtuiDevProperty::UncategorizedDev);
                    break;
                default:
                    // should never get here
                    break;
            }
        }
    }
    else
    {
        devTypesWanted = mSelectedDeviceType;
    }

    if((devTypesWanted != mSelectedDeviceType) &&(devTypesWanted !=0))
    {
        mSelectedDeviceType = devTypesWanted;
        delete mContentItemModel;
        mContentItemModel = new QStandardItemModel(this);
        mDeviceList->setModel(mContentItemModel);
        mSelectedData.clear();
        for(int i=0;i<mData.count();i++)
        {
            const BtSendDataItem& qtdev = mData[i];
            if(devTypesWanted & qtdev[DeviceTypeRole].toInt() )
            {
                QStandardItem* listitem = new QStandardItem();
                QStringList info;
                info.append(qtdev[NameAliasRole].toString());
    
                listitem->setData(info, Qt::DisplayRole);
                HbIcon icon =  getBadgedDeviceTypeIcon(qtdev[CoDRole].toDouble(),
                        qtdev[MajorPropertyRole].toInt(),
                         BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight);
                listitem->setIcon(icon.qicon());
                mContentItemModel->appendRow(listitem);        
                mSelectedData.append(qtdev);
            }
        }
    }
}

void BTDeviceSearchDialogWidget::deviceSelected(const QModelIndex& modelIndex)
{
    int row = 0;
    QVariantMap val;
    
    row = modelIndex.row();
    const BtSendDataItem& qtdev = mSelectedData.at(row);
    val.insert("selectedindex",QVariant(row));
    val.insert("devicename",qtdev[NameAliasRole]);
    val.insert("deviceaddress",qtdev[ReadableBdaddrRole]);
    val.insert("deviceclass",qtdev[CoDRole]);

    emit deviceDialogData(val);
}


