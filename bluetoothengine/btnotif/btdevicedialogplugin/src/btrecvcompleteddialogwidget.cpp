/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BTRecvcompletedDialogWidget class declaration.
*
*/

#include "btrecvcompleteddialogwidget.h"
#include "bluetoothdevicedialogs.h"
#include "btdevicedialogpluginerrors.h"

const char* DOCML_BT_RECV_CMPLTD_DIALOG = ":/docml/bt-receive-done-dialog.docml";


BTRecvcompletedDialogWidget::BTRecvcompletedDialogWidget(const QVariantMap &parameters)

:mLoader(0),
 mOpenConversationView(false),
 mError(NoError)
{
    constructDialog(parameters);
}

BTRecvcompletedDialogWidget::~BTRecvcompletedDialogWidget()
{
    if(mLoader)
    {
        delete mLoader;
        mLoader = NULL;
    }
}

bool BTRecvcompletedDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    Q_UNUSED(parameters);
    if(!mError)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int BTRecvcompletedDialogWidget::deviceDialogError() const
{
    return mError;
}

void BTRecvcompletedDialogWidget::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    mReceiveCompleteDialog->close();
    emit deviceDialogClosed();
}

HbPopup* BTRecvcompletedDialogWidget::deviceDialogWidget() const
{
    return mReceiveCompleteDialog;
}

QObject* BTRecvcompletedDialogWidget::signalSender() const
{
    return const_cast<BTRecvcompletedDialogWidget*>(this);
}

void BTRecvcompletedDialogWidget::constructDialog(const QVariantMap &parameters)
{
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_RECV_CMPLTD_DIALOG, &ok);
    if(ok)
    {
    	mReceiveCompleteDialog = qobject_cast<HbDialog*>(mLoader->findWidget("receiveCompleteDialog"));
        mHeading = qobject_cast<HbLabel*>(mLoader->findWidget("receiveCompleteHeading"));
        
        mFileName = qobject_cast<HbLabel*>(mLoader->findWidget("fileName"));
        mFileSize = qobject_cast<HbLabel*>(mLoader->findWidget("fileSize"));
        mFileCount = qobject_cast<HbLabel*>(mLoader->findWidget("fileCount"));
        mFileCount->setVisible(false);
        
        //TODO - set icon based on the file icon.
        
        mShowAction = qobject_cast<HbAction*>(mLoader->findObject("showAction"));
        mCancelAction = qobject_cast<HbAction*>(mLoader->findObject("cancelAction"));
        
        QString headingStr(hbTrId("txt_bt_title_received_from_1"));
        QString senderName(parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceName)).toString());
        mHeading->setPlainText(headingStr.arg(senderName));
        mReceiveCompleteDialog->setHeadingWidget(mHeading);
        
        mFileName->setPlainText(parameters.value(QString::number(TBluetoothDeviceDialog::EReceivingFileName)).toString());
        
        mFileSz = parameters.value(QString::number(TBluetoothDeviceDialog::EReceivingFileSize)).toInt();
        
        //Format the file size into a more readable format
        if ( mFileSz >> 20 )    // size in MB
            {       
            float sizeInMB = 0;
            sizeInMB = ((float)mFileSz ) / (1024*1024);
            QString fileSzMb;
            fileSzMb.setNum(sizeInMB);
            //TODO - check for localization
            fileSzMb.append(QString(" Mb"));
            mFileSize->setPlainText(fileSzMb);
            }
        
        else if( mFileSz >> 10 )        // size in KB
            {
            TInt64 sizeInKB = 0;
            sizeInKB = mFileSz >> 10;
            QString fileSzKb;
            fileSzKb.setNum(sizeInKB);
            //TODO - check for localization
            fileSzKb.append(QString(" Kb"));
            mFileSize->setPlainText(fileSzKb);
            }

        else                              // size is unknown or less than 1K
            {
            QString fileSzB;
            fileSzB.setNum(mFileSz);
            //TODO - check for localization
            fileSzB.append(QString(" Bytes"));
            mFileSize->setPlainText(fileSzB);
            }

        //Set the received file count
        int fCnt = parameters.value(QString::number(TBluetoothDeviceDialog::EReceivedFileCount)).toInt();
        if(fCnt > 1)
            {
            mFileCount->setVisible(true);  
            
            QString fCntStr(hbTrId("txt_bt_info_ln_other_files_received", (fCnt-1)));
            mFileCount->setPlainText(fCntStr);
            }

        mReceiveCompleteDialog->setBackgroundFaded(false);
        mReceiveCompleteDialog->setDismissPolicy(HbPopup::NoDismiss);
        mReceiveCompleteDialog->setTimeout(HbPopup::NoTimeout);
         
        connect(mShowAction, SIGNAL(triggered()), this, SLOT(showClicked()));
        connect(mCancelAction, SIGNAL(triggered()), this, SLOT(cancelClicked()));
        
        QVariantMap::const_iterator i = parameters.find("OpenCnvView");
        if(i != parameters.end())
            {
            mOpenConversationView = (i.value().toBool() == true) ? true : false; 
            }
    }
    else
    {
        mError = DocMLLoadingError;
    }
}

void BTRecvcompletedDialogWidget::showClicked()
{   
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(TBluetoothDialogParams::EShow));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

void BTRecvcompletedDialogWidget::cancelClicked()
{
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(TBluetoothDialogParams::ECancelShow));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

