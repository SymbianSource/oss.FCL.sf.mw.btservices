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
#include <hblistwidget.h>
#include <qstandarditemmodel.h>
#include <hbaction.h>
#include "btsenddialogwidget.h"



const char* DOCML_BT_SEND_DIALOG = ":/docml/bt-send-dialog.docml";

BTSendDialogWidget::BTSendDialogWidget(const QVariantMap &parameters)
:HbDialog()
    {
 //   LOG(ELevel1,_L("BTSendDialogWidget::BTSendDialogWidget"));
    constructDialog(parameters);
    
    }

BTSendDialogWidget::~BTSendDialogWidget()
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
    }

bool BTSendDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
    {
 //   LOG(ELevel1,_L("BTSendDialogWidget::setDeviceDialogParameters "));
    
    if(mFileIndex != parameters.value("currentFileIdx").toString().toInt() )
        {
        mLabel->setTextWrapping(Hb::TextWordWrap);
        mLabel->setAlignment(Qt::AlignHCenter);
        //Todo - replace this with the actual text from parameters
    
        QString headLabel;
        headLabel.append(QString("Sending file "));
        headLabel.append(parameters.value("currentFileIdx").toString());
        headLabel.append('/');
        headLabel.append(parameters.value("totalFilesCnt").toString());
        headLabel.append(QString(" to "));
        headLabel.append(parameters.value("destinationName").toString());
        mLabel->setPlainText(headLabel);
        
        QStringList info;
        info.append(parameters.value("fileName").toString());
        info.append(parameters.value("fileSzTxt").toString());
                    
        QStandardItem* listitem = new QStandardItem();
        // parameters.
        listitem->setData(info, Qt::DisplayRole);
    
        //Todo - Insert icons based on the device class        
        QIcon icon(QString(":/icons/qtg_large_bluetooth.svg"));
        listitem->setIcon(icon);
    
        delete mContentItemModel;
        mContentItemModel = new QStandardItemModel(this);
        mListView->setModel(mContentItemModel);//, prototype);
    
        mContentItemModel->appendRow(listitem);

        mProgressBar->setMinimum(0);
        mProgressBar->setProgressValue(0);
        mProgressBar->setMaximum(parameters.value("fileSz").toInt());
        mFileIndex = parameters.value("currentFileIdx").toString().toInt();
        }
    else
        {
        mProgressBar->setProgressValue(parameters.value("progressValue").toInt());
        }
 //   LOG(ELevel1,_L("BTSendDialogWidget::setDeviceDialogParameters Completed"));
    return true;
    }

int BTSendDialogWidget::deviceDialogError() const
    {
    return 0;
    }

void BTSendDialogWidget::closeDeviceDialog(bool byClient)
    {
    Q_UNUSED(byClient);
    this->close();
    }

HbPopup* BTSendDialogWidget::deviceDialogWidget() const
    {
    return const_cast<BTSendDialogWidget*>(this);
    }

QObject *BTSendDialogWidget::signalSender() const
{
    return const_cast<BTSendDialogWidget*>(this);
}  

bool BTSendDialogWidget::constructDialog(const QVariantMap&/*parameters*/)
    {
 //   LOG(ELevel1,_L("BTSendDialogWidget::constructDialog "));
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_SEND_DIALOG, &ok);
    if(ok)
        {
        mLabel = qobject_cast<HbLabel*>(mLoader->findWidget("heading"));
        this->setHeadingWidget(mLabel);
        mListView = qobject_cast<HbListView*>(mLoader->findWidget("listView"));
        if(mListView)
            {
            //Todo - replace this with the actual text from parameters  
            mContentItemModel = new QStandardItemModel(this);
            mListView->setModel(mContentItemModel);//, prototype);
            }
         
        mProgressBar = qobject_cast<HbProgressBar*>(mLoader->findWidget("horizontalProgressBar"));
                
        HbAction* hide = new HbAction("Hide");
        HbAction* cancel = new HbAction("Cancel");
        
        this->addAction(hide);
        this->addAction(cancel);
        
        QGraphicsWidget *widget = mLoader->findWidget(QString("container"));
        this->setContentWidget(widget);
        }

    this->setBackgroundFaded(false);
    setDismissPolicy(HbPopup::NoDismiss);
    setTimeout(HbPopup::NoTimeout);
     
    this->actions().first()->disconnect(this);
    connect(this, SIGNAL(finished(HbAction*)), this, SLOT(inputClosed(HbAction*)));
    
    return true;
    }

void BTSendDialogWidget::hideEvent(QHideEvent *event)
    {
    HbDialog::hideEvent(event);
    emit deviceDialogClosed();
    }

void BTSendDialogWidget::showEvent(QShowEvent *event)
    {
    HbDialog::showEvent(event);
    }

/*void BTSendDialogWidget::hideClicked()
    {
    // TODO
    this->close();
    emit deviceDialogClosed();
    }

void BTSendDialogWidget::cancelClicked()
    {
    // TODO
    this->close();
    emit deviceDialogClosed();
    }*/

void BTSendDialogWidget::inputClosed(HbAction* action)
    {
    QVariantMap data;
     
    HbDialog *dlg=static_cast<HbDialog*>(sender());
    if(dlg->actions().first() == action) {
    } 
    else if(dlg->actions().at(1) == action) {
      }
    }

