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


#include "btrecvprgrsdialogwidget.h"
#include "bluetoothdevicedialogs.h"

const char* DOCML_BT_RECV_PRGRS_DIALOG = ":/docml/bt-recv-progress-dialog.docml";


BTRecvPrgrsDialogWidget::BTRecvPrgrsDialogWidget(const QVariantMap &parameters)
{
    mLoader = 0;
    constructDialog(parameters);
}

BTRecvPrgrsDialogWidget::~BTRecvPrgrsDialogWidget()
{
    if(mLoader)
    {
        delete mLoader;
        mLoader = NULL;
    }
}

bool BTRecvPrgrsDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    mProgressBar->setMinimum(0);
    mProgressBar->setMaximum(mFileSz);
    mProgressBar->setProgressValue(parameters.value("progress").toInt());
    return true;
}

int BTRecvPrgrsDialogWidget::deviceDialogError() const
{
    return 0;
}

void BTRecvPrgrsDialogWidget::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    mDialog->close();
}

HbPopup* BTRecvPrgrsDialogWidget::deviceDialogWidget() const
{
    return mDialog;
}

QObject* BTRecvPrgrsDialogWidget::signalSender() const
{
    return const_cast<BTRecvPrgrsDialogWidget*>(this);
}

bool BTRecvPrgrsDialogWidget::constructDialog(const QVariantMap &parameters)
{
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_RECV_PRGRS_DIALOG, &ok);
    if(ok)
    {
        mDialog = qobject_cast<HbDialog*>(mLoader->findWidget("senddialog"));
        mHeading = qobject_cast<HbLabel*>(mLoader->findWidget("heading"));
        
        mFileName = qobject_cast<HbLabel*>(mLoader->findWidget("fileName"));
        mFileSize = qobject_cast<HbLabel*>(mLoader->findWidget("fileSize"));
        mFileCount = qobject_cast<HbLabel*>(mLoader->findWidget("fileCount"));
                
        //TODO - set icon based on the file icon.
        
        mHide = qobject_cast<HbAction*>(mLoader->findObject("hideaction"));
        mCancel = qobject_cast<HbAction*>(mLoader->findObject("cancelaction"));
        
        mProgressBar = qobject_cast<HbProgressBar*>(mLoader->findWidget("horizontalProgressBar"));
        
        int dialogType = parameters.value(QString::number(TBluetoothDialogParams::EDialogTitle)).toInt();
        switch(dialogType)
        {
            case TBluetoothDialogParams::EReceive:
            {
                //TODO - Localization
                QString headingStr(hbTrId("txt_bt_title_receiving_files_from_1"));
                QString senderName(parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceName)).toString());
                mHeading->setPlainText(headingStr.arg(senderName));
            }break;

            default:
                break;
        }
        mDialog->setHeadingWidget(mHeading);
        
        mFileName->setPlainText(parameters.value(QString::number(TBluetoothDeviceDialog::EReceivingFileName)).toString());
        
        mFileSz = parameters.value(QString::number(TBluetoothDeviceDialog::EReceivingFileSize)).toInt();
        mProgressBar->setMinimum(0);
        mProgressBar->setMaximum(mFileSz);
        mProgressBar->setProgressValue(0);
        
        //Format the file size into a more readable format
        if ( mFileSz >> 20 )    // size in MB
            {       
            float sizeInMB = 0;
            sizeInMB = ((float)mFileSz ) / (1024*1024);
            QString fileSzMb;
            fileSzMb.setNum(sizeInMB);
            fileSzMb.append(QString(" Mb"));
            mFileSize->setPlainText(fileSzMb);
            }
        
        else if( mFileSz >> 10 )        // size in KB
            {
            TInt64 sizeInKB = 0;
            sizeInKB = mFileSz >> 10;
            QString fileSzKb;
            fileSzKb.setNum(sizeInKB);
            fileSzKb.append(QString(" Kb"));
            mFileSize->setPlainText(fileSzKb);
            }

        else                              // size is unknown or less than 1K
            {
            QString fileSzB;
            fileSzB.setNum(mFileSz);
            fileSzB.append(QString(" Bytes"));
            mFileSize->setPlainText(fileSzB);
            }

        //TODO - Set the received file count
/*
        int count = parameters.value(QString::number(TBluetoothDeviceDialog::EReceivedFileCount)).toInt();
        QString fileCntStr;
        fileCntStr.setNum(count);
        mFileCount->setPlainText(fileCntStr);
*/        
    }

    mDialog->setBackgroundFaded(false);
    mDialog->setDismissPolicy(HbPopup::NoDismiss);
    mDialog->setTimeout(HbPopup::NoTimeout);
     
    connect(mHide, SIGNAL(triggered()), this, SLOT(hideClicked()));
    connect(mCancel, SIGNAL(triggered()), this, SLOT(cancelClicked()));
    
    return true;
}

void BTRecvPrgrsDialogWidget::hideClicked()
{
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(true));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

void BTRecvPrgrsDialogWidget::cancelClicked()
{
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(false));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

