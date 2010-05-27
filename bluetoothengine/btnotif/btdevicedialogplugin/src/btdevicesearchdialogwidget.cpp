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


#include "btdevicesearchdialogwidget.h"

#include <hblistview.h>
#include <hbtoolbar.h>
#include <hbselectiondialog.h>

#include <qstandarditemmodel.h>
#include <hbaction.h>

const char* DOCML_BTDEV_SEARCH_DIALOG = ":/docml/bt-device-search-dialog.docml";


BTDeviceSearchDialogWidget::BTDeviceSearchDialogWidget(const QVariantMap &parameters)
:HbDialog()
    {
    mDeviceLstIdx = 0;
    mViewByChosen = false;
    mSelectedType = 0;
    mDeviceDialogData = 0;
    constructDialog(parameters);
    }

BTDeviceSearchDialogWidget::~BTDeviceSearchDialogWidget()
    {
    if(mLoader)
        {
        delete mLoader;
        mLoader = NULL;
        }
    if(mContentItemModel)
        {
        delete mContentItemModel;
        mContentItemModel = NULL;
        }
 //   delete mRbl;
 //   delete mViewByDialog;
    }

bool BTDeviceSearchDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
    {
    if(parameters.keys().at(0).compare("Search Completed")==0)
        {
        mSearchLabel->hide();
        
        mSearchIconLabel->hide();
        
        mSearchDoneLabel->show();
        mSearchDoneLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchDoneLabel->setAlignment(Qt::AlignLeft);
        mSearchDoneLabel->setPlainText("Search Done");
        
        mStopRetryBtn->setText("Retry");
        }
    else
        {
        device newDevice;
    
       // newDevice.mDeviceName = parameters.value("deviceName").toString();
        newDevice.mDeviceName = parameters.value(parameters.keys().at(0)).toString();
        
     //   newDevice.mDeviceType = parameters.value("deviceType").toString();
        newDevice.mDeviceIdx = mDeviceLstIdx;
        
        mDeviceList.append(newDevice);
        mDeviceLstIdx++;
    
        QStringList info;
     //   if(!mViewByChosen)
            {
            info.append(newDevice.mDeviceName);
       //     info.append(newDevice.mDeviceType);
            QStandardItem* listitem = new QStandardItem();
            listitem->setData(info, Qt::DisplayRole);
        
            listitem->setIcon(icon());
        
            mContentItemModel->appendRow(listitem);
            }
     /*   else
            {
            if(mDeviceTypeList[mSelectedType] == newDevice.mDeviceType)
                {
                info.append(newDevice.mDeviceName);
                info.append(newDevice.mDeviceType);
                QStandardItem* listitem = new QStandardItem();
                listitem->setData(info, Qt::DisplayRole);
    
                listitem->setIcon(icon(newDevice.mDeviceType));
    
                mContentItemModel->appendRow(listitem);
                }
            }*/
        }
                
    return true;
    }

int BTDeviceSearchDialogWidget::deviceDialogError() const
    {
    return 0;
    }

void BTDeviceSearchDialogWidget::closeDeviceDialog(bool byClient)
    {
    Q_UNUSED(byClient);
    this->close();
    }

HbPopup* BTDeviceSearchDialogWidget::deviceDialogWidget() const
    {
    return const_cast<BTDeviceSearchDialogWidget*>(this);
    }

bool BTDeviceSearchDialogWidget::constructDialog(const QVariantMap &parameters)
    {
    (void) parameters;
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BTDEV_SEARCH_DIALOG, &ok);
    if(ok)
        {
        HbLabel* heading = qobject_cast<HbLabel*>(mLoader->findWidget("heading"));
        heading->setTextWrapping(Hb::TextWordWrap);
        heading->setAlignment(Qt::AlignHCenter);
        heading->setPlainText("Bluetooth - Found devices");
        setHeadingWidget(heading);
        
        mSearchLabel = qobject_cast<HbLabel*>(mLoader->findWidget("searchLabel"));
        mSearchLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchLabel->setAlignment(Qt::AlignHCenter);
        mSearchLabel->setPlainText("Searching...");
 
        mSearchIconLabel = qobject_cast<HbLabel*>(mLoader->findWidget("iconLabel"));
        mSearchIconLabel->setIcon(icon());

        mSearchDoneLabel = qobject_cast<HbLabel*>(mLoader->findWidget("searchDoneLabel"));
        mSearchDoneLabel->hide();
        
        setFrameType(HbDialog::Strong);
        setBackgroundFaded(false);

        mViewByBtn = qobject_cast<HbPushButton*>(mLoader->findWidget("viewby"));
        mStopRetryBtn = qobject_cast<HbPushButton*>(mLoader->findWidget("stop"));
        
        mListView = qobject_cast<HbListView*>(mLoader->findWidget("listView"));
        mListView->setSelectionMode(HbAbstractItemView::SingleSelection);

        mContentItemModel = new QStandardItemModel(this);
        mListView->setModel(mContentItemModel);//, prototype);

        connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
        connect(mStopRetryBtn, SIGNAL(clicked()), this, SLOT(stopRetryClicked()));
        connect(mViewByBtn, SIGNAL(clicked()), this, SLOT(viewByClicked()));
        
        QGraphicsWidget *widget = mLoader->findWidget(QString("container"));
        setContentWidget(widget);
        }
    setBackgroundFaded(false);
    setDismissPolicy(HbPopup::TapOutside);
    setTimeout(HbPopup::NoTimeout);
    
 /*   mViewByDialog = new HbDialog();
    mRbl = new HbRadioButtonList(mViewByDialog);
    connect(mRbl, SIGNAL(itemSelected(int)), this, SLOT(viewByItemSelected(int)));*/
    
    return true;
    }

void BTDeviceSearchDialogWidget::hideEvent(QHideEvent *event)
    {
    HbDialog::hideEvent(event);
    QVariantMap val;
    QVariant index(-1);
    val.insert("selectedindex",index);
    emit deviceDialogData(val);    
    emit deviceDialogClosed();
    }

void BTDeviceSearchDialogWidget::showEvent(QShowEvent *event)
    {
    HbDialog::showEvent(event);
    }

void BTDeviceSearchDialogWidget::stopRetryClicked()
    {
    QVariantMap val;
    if(mStopRetryBtn->text().compare("Retry")==0)
        {
        QVariant index("Retry");
        val.insert("Retry",index); 
        emit deviceDialogData(val);
        delete mContentItemModel;
        mContentItemModel = new QStandardItemModel(this);
        mListView->setModel(mContentItemModel);
        mStopRetryBtn->setText("Stop");
        
        mSearchLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchLabel->setAlignment(Qt::AlignHCenter);
        mSearchLabel->setPlainText("Searching...");
        
        mSearchIconLabel->setIcon(icon());     
        mSearchLabel->show();
        
        mSearchIconLabel->show();
        
        mSearchDoneLabel->hide();
        }
    else
        {
        mStopRetryBtn->setText("Retry");
        
        mSearchLabel->hide();
        
        mSearchIconLabel->hide();
        
        mSearchDoneLabel->show();
        mSearchDoneLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchDoneLabel->setAlignment(Qt::AlignLeft);
        mSearchDoneLabel->setPlainText("Search Done");        
        
        QVariantMap val;
        QVariant index("Stop");
        val.insert("Stop",index); 
        emit deviceDialogData(val);    
        }
    }

void BTDeviceSearchDialogWidget::retryClicked()
    {
    QVariantMap val;
    QVariant index("Retry");
    val.insert("Retry",index); 
    emit deviceDialogData(val);
    delete mContentItemModel;
    mContentItemModel = new QStandardItemModel(this);
    mListView->setModel(mContentItemModel);
    

    }

void BTDeviceSearchDialogWidget::viewByClicked()
    {
    QStringList list;
    list << "Select all" << "Audio devices" << "Computers" << "Input devices" << "Phones" << "Other devices";

    HbSelectionDialog *query = new HbSelectionDialog;
    query->setStringItems(list);
    query->setSelectionMode(HbAbstractItemView::MultiSelection);

    QList<QVariant> current;
    current.append(QVariant(0));
    query->setSelectedItems(current);

    query->setAttribute(Qt::WA_DeleteOnClose);

    query->open(this,SLOT(selectionDialogClosed(HbAction*)));
    
    //connect(query, SIGNAL(finished(HbAction*)), this, SLOT(selectionDialogClosed(HbAction*)));
    
/*    mViewByDialog->setDismissPolicy(HbPopup::NoDismiss);
    mViewByDialog->setTimeout(HbPopup::NoTimeout);

    bool foundEntry = false;
    QStringList st;
    st << "All";
    mDeviceTypeList.clear();
    for(int i = 0; i < mDeviceList.count(); i++)
        {
        for(int j = 0; j < mDeviceTypeList.count(); j++)
            {
            if(mDeviceTypeList[j] == mDeviceList[i].mDeviceType)
                {
                foundEntry = true;
                break;
                }
            }
        if(!foundEntry)
            {
            mDeviceTypeList.append(mDeviceList[i].mDeviceType);
            }
        foundEntry = false;
        }
    
    for(int k = 0; k < mDeviceTypeList.count(); k++)
        {
        st << mDeviceTypeList[k];
        }
    
    mRbl->setItems(st);
    mViewByDialog->setContentWidget(mRbl);
    mViewByDialog->setMaximumHeight(300);
    mViewByDialog->setMaximumWidth(500);

    mViewByDialog->show();*/
    }

void BTDeviceSearchDialogWidget::selectionDialogClosed(HbAction* action)
    {
    Q_UNUSED(action);

 /*   HbSelectionDialog *dlg = (HbSelectionDialog*)(sender());
    if(dlg->actions().first() == action) {

     } 
    else if(dlg->actions().at(1) == action) {
     }*/
    }

void BTDeviceSearchDialogWidget::deviceSelected(const QModelIndex& modelIndex)
    {
    int row = 0;
    
 /*   if(mViewByChosen)
        {
        row = mDeviceLstOfType[modelIndex.row()].mDeviceIdx;
        }
    
    else*/
        {
        row = modelIndex.row();
        }
    
    QVariantMap val;
    QVariant index(row);
    val.insert("selectedindex",index);
    emit deviceDialogData(val);
//    mDeviceDialogData = 1;//flag is to say that device dialog data is emitted required when we cancel the dialog
    //emit deviceDialogClosed();
  //  this->close();
    }

//void BTDeviceSearchDialogWidget::viewByItemSelected(int index)
  //  {
    //  (void) index;
 /*   if(index == 0)
        {
        //Option 'All' selected    
        mViewByDialog->close();
        delete mContentItemModel;
        mContentItemModel = new QStandardItemModel(this);
        mListView->setModel(mContentItemModel);
        mViewByChosen = false;

        for(int i = 0; i < mDeviceList.count(); i++)
            {        
            QStandardItem* listitem = new QStandardItem();
                
            QStringList info;
            info << mDeviceList[i].mDeviceName << mDeviceList[i].mDeviceType ;
            listitem->setData(info, Qt::DisplayRole);
    
            //listitem->setIcon(icon(mDeviceList[i].mDeviceType));
    
            mContentItemModel->appendRow(listitem);
            }
        }
    else
        {
        index--;
        mSelectedType = index;
        mViewByDialog->close();
        
        delete mContentItemModel;
        mContentItemModel = new QStandardItemModel(this);
        mListView->setModel(mContentItemModel);
    
        mDeviceLstOfType.clear();
        for(int i = 0; i < mDeviceList.count(); i++)
            {
            if(mDeviceList[i].mDeviceType == mDeviceTypeList[index])
                {
                mDeviceLstOfType.append(mDeviceList[i]);
            
                QStandardItem* listitem = new QStandardItem();
                
                QStringList info;
                info << mDeviceList[i].mDeviceName << mDeviceTypeList[index];
                listitem->setData(info, Qt::DisplayRole);
        
                //listitem->setIcon(icon(mDeviceTypeList[index]));
        
                mContentItemModel->appendRow(listitem);
                }
            }
        mViewByChosen = true;
        }*/
 //   }

QIcon BTDeviceSearchDialogWidget::icon()
    {
 /*   if(deviceType == "Audio")
        {
        return (QIcon(QString(":/icons/qgn_prop_bt_audio.svg")));
        }
    else if(deviceType == "Car-kit")
        {
        return (QIcon(QString(":/icons/qgn_prop_bt_car_kit.svg")));
        }
    else if(deviceType == "Computer")
        {
        return (QIcon(QString(":/icons/qgn_prop_bt_computer.svg")));
        }
    else if(deviceType == "Headset")
        {
        return (QIcon(QString(":/icons/qgn_prop_bt_headset.svg")));
        }
    else if(deviceType == "Keyboard")
        {
        return (QIcon(QString(":/icons/qgn_prop_bt_keyboard.svg")));
        }
    else if(deviceType == "Mouse")
        {
        return (QIcon(QString(":/icons/qgn_prop_bt_mouse.svg")));
        }
    else if(deviceType == "Phone")
        {
        return (QIcon(QString(":/icons/qgn_prop_bt_phone.svg")));
        }
    else if(deviceType == "Printer")
        {
        return (QIcon(QString(":/icons/qgn_prop_bt_printer.svg")));
        }
    else
        {
        return (QIcon(QString(":/icons/qgn_prop_bt_unknown.svg")));
        }*/
    return QIcon(QString(":/icons/qtg_large_bluetooth.svg"));
    }

