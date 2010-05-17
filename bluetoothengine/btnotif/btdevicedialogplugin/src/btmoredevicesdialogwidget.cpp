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


#include <hblabel.h>
#include <hblistview.h>
#include <hbtoolbar.h>
#include <hbpushbutton.h>
#include <hblistwidget.h>
#include <qstandarditemmodel.h>
#include "btmoredevicesdialogwidget.h"


const char* DOCML_BT_MORE_DEV_DIALOG = ":/docml/bt-more-devices-dialog.docml";


BTMoreDevicesDialogWidget::BTMoreDevicesDialogWidget(const QVariantMap &parameters)
:HbDialog()
    {
    mDeviceDialogData = 0;
    constructDialog(parameters);
    }

BTMoreDevicesDialogWidget::~BTMoreDevicesDialogWidget()
    {
    if(mLoader)
        {
        delete mLoader;
        mLoader = NULL;
        }
    if(mContentItemModel)
        {
        delete mContentItemModel;
        mContentItemModel =NULL;
        }
    }

bool BTMoreDevicesDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
    {
    QStandardItem* listitem = new QStandardItem();
    QStringList info;
   // info.append(parameters.value("deviceName").toString());
    //info.append(parameters.value("deviceType").toString());
    info.append(parameters.value(parameters.keys().at(0)).toString());


    listitem->setData(info, Qt::DisplayRole);
    listitem->setIcon(icon());
 //   listitem->setIcon(icon(parameters.value("deviceType").toString()));
    
    mContentItemModel->appendRow(listitem);

    return true;
    }

int BTMoreDevicesDialogWidget::deviceDialogError() const
    {
    return 0;
    }

void BTMoreDevicesDialogWidget::closeDeviceDialog(bool byClient)
    {
    Q_UNUSED(byClient);
    this->close();
    }

HbPopup* BTMoreDevicesDialogWidget::deviceDialogWidget() const
    {
    return const_cast<BTMoreDevicesDialogWidget*>(this);
    }

bool BTMoreDevicesDialogWidget::constructDialog(const QVariantMap &/*parameters*/)
    {
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_MORE_DEV_DIALOG, &ok);
    if(ok)
        {
        HbLabel* label = qobject_cast<HbLabel*>(mLoader->findWidget("label"));
        if(label)
            {
            label->setTextWrapping(Hb::TextWordWrap);
            label->setPlainText("Send to:");
            }
        this->setHeadingWidget(label);
        HbPushButton* moreDevices = qobject_cast<HbPushButton*>(mLoader->findWidget("moreDevices"));
        HbPushButton* cancel = qobject_cast<HbPushButton*>(mLoader->findWidget("cancel"));
        
        HbListView* listView = qobject_cast<HbListView*>(mLoader->findWidget("listView"));
        listView->setSelectionMode(HbAbstractItemView::SingleSelection);

        mContentItemModel = new QStandardItemModel(this);
        listView->setModel(mContentItemModel);//, prototype);
        
 //       QList<QVariant> values = parameters.values();
        
 /*       for(int i=0;i < values.count();i++)
            {
            QStandardItem* listitem = new QStandardItem();
            // parameters.
      //      QString string = values.at(i).toString();
                        
            listitem->setData(values.at(i).toString(), Qt::DisplayRole);
   //         listitem->setData(QString("search"), Qt::DisplayRole);
            //Todo - Insert icons based on the device class        
            QIcon icon(QString(":/qgn_prop_sml_bt.svg"));
            listitem->setIcon(icon);
        
            mContentItemModel->appendRow(listitem);
            }*/
        
        connect(listView, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
        connect(moreDevices, SIGNAL(clicked()), this, SLOT(moreDevicesClicked()));
        connect(cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
        
        QGraphicsWidget *widget = mLoader->findWidget(QString("container"));
        this->setContentWidget(widget);
        }


    this->setBackgroundFaded(false);
    setDismissPolicy(HbPopup::NoDismiss);
    setTimeout(HbPopup::NoTimeout);
    return true;
    }

void BTMoreDevicesDialogWidget::hideEvent(QHideEvent *event)
    {
    HbDialog::hideEvent(event);
//    if(mDeviceDialogData == 0)
        {
        QVariantMap val;
        QVariant index(-1);
        val.insert("selectedindex",index);
        emit deviceDialogData(val);    
        emit deviceDialogClosed();
        }    
 //   
    }

void BTMoreDevicesDialogWidget::showEvent(QShowEvent *event)
    {
    HbDialog::showEvent(event);
    }

void BTMoreDevicesDialogWidget::moreDevicesClicked()
    {
    QVariantMap val;
    QVariant index("MoreDevices");
    val.insert("MoreDevices",index);    
    emit deviceDialogData(val);
 //   mDeviceDialogData = 1;//flag is to say that device dialog data is emitted required when we cancel the dialog
   // this->close();
    // TODO
    }

void BTMoreDevicesDialogWidget::cancelClicked()
    {
    // TODO
    this->close();
    }

void BTMoreDevicesDialogWidget::deviceSelected(const QModelIndex& modelIndex)
    {
    int row = modelIndex.row();
    QVariantMap val;
    QVariant index(row);
    val.insert("selectedindex",index);
    


    
    emit deviceDialogData(val);
  //  mDeviceDialogData = 1;//flag is to say that device dialog data is emitted required when we cancel the dialog    
   // this->close();

    }
  
QIcon BTMoreDevicesDialogWidget::icon(/*QString deviceType*/)
    {
 /*   if(deviceType == "Audio")
        {
        return (QIcon(QString(":/qgn_prop_bt_audio.svg")));
        }
    else if(deviceType == "Car-kit")
        {
        return (QIcon(QString(":/qgn_prop_bt_car_kit.svg")));
        }
    else if(deviceType == "Computer")
        {
        return (QIcon(QString(":/qgn_prop_bt_computer.svg")));
        }
    else if(deviceType == "Headset")
        {
        return (QIcon(QString(":/qgn_prop_bt_headset.svg")));
        }
    else if(deviceType == "Keyboard")
        {
        return (QIcon(QString(":/qgn_prop_bt_keyboard.svg")));
        }
    else if(deviceType == "Mouse")
        {
        return (QIcon(QString(":/qgn_prop_bt_mouse.svg")));
        }
    else if(deviceType == "Phone")
        {
        return (QIcon(QString(":/qgn_prop_bt_phone.svg")));
        }
    else if(deviceType == "Printer")
        {
        return (QIcon(QString(":/qgn_prop_bt_printer.svg")));
        }
    else
        {
        return (QIcon(QString(":/qgn_prop_bt_unknown.svg")));
        }*/
    return QIcon(QString(":/icons/qtg_large_bluetooth.svg"));
    }

