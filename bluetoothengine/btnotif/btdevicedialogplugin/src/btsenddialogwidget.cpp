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

#include "btsenddialogwidget.h"
#include "btdevicedialogpluginerrors.h"


#define LOC_SENDING_FILES_TO_DEVICE hbTrId("txt_bt_title_sending_file_l1l2_to_3")

const char* DOCML_BT_SEND_DIALOG = ":/docml/bt-send-dialog.docml";

BTSendDialogWidget::BTSendDialogWidget(const QVariantMap &parameters)
{
    mLoader = 0;
    mLastError = NoError;
    constructDialog(parameters);
}

BTSendDialogWidget::~BTSendDialogWidget()
{
    delete mLoader;
}

bool BTSendDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
	mLastError = NoError;
    if(mFileIndex != parameters.value("currentFileIdx").toString().toInt() )
    {
        mDialogHeading->setTextWrapping(Hb::TextWordWrap);
        mDialogHeading->setAlignment(Qt::AlignHCenter);
    
        QString headLabel = QString(LOC_SENDING_FILES_TO_DEVICE).arg(parameters.value("currentFileIdx").toInt())
                                        .arg(parameters.value("totalFilesCnt").toInt())
                                        .arg(parameters.value("destinationName").toString());
        mDialogHeading->setPlainText(headLabel);
        
        //Todo - Insert file icons here instead of bluetooth image        
        QIcon icon(QString(":/icons/qtg_large_bluetooth.svg"));        
        mFileIconLabel->setIcon(icon);
        mFileNameLabel->setPlainText(parameters.value("fileName").toString());
        mFileSizeLabel->setPlainText(parameters.value("fileSzTxt").toString());
        mProgressBar->setMinimum(0);
        mProgressBar->setProgressValue(0);
        mProgressBar->setMaximum(parameters.value("fileSz").toInt());
        mFileIndex = parameters.value("currentFileIdx").toString().toInt();
    }
    else
    {
        mProgressBar->setProgressValue(parameters.value("progressValue").toInt());
    }
    return true;
}

int BTSendDialogWidget::deviceDialogError() const
{
    return mLastError;
}

void BTSendDialogWidget::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    mSendDialog->close();
// below redundant call is required because of the api documentation. 
    emit deviceDialogClosed();
}

HbPopup* BTSendDialogWidget::deviceDialogWidget() const
{
    return mSendDialog;
}

QObject *BTSendDialogWidget::signalSender() const
{
    return const_cast<BTSendDialogWidget*>(this);
}  

void BTSendDialogWidget::constructDialog(const QVariantMap& parameters)
{
		Q_UNUSED(parameters);
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_SEND_DIALOG, &ok);
    if(ok)
    {
        mSendDialog = qobject_cast<HbDialog*>(mLoader->findWidget("sendProgressDialog"));
        mDialogHeading = qobject_cast<HbLabel*>(mLoader->findWidget("sendDialogHeading"));
        mFileIconLabel = qobject_cast<HbLabel*>(mLoader->findWidget("fileIcon"));
        mFileNameLabel = qobject_cast<HbLabel*>(mLoader->findWidget("fileName"));
        mFileSizeLabel = qobject_cast<HbLabel*>(mLoader->findWidget("fileSize"));
         
        mProgressBar = qobject_cast<HbProgressBar*>(mLoader->findWidget("sendProgressBar"));
		    mSendDialog->setBackgroundFaded(false);
		    mSendDialog->setDismissPolicy(HbPopup::NoDismiss);
		    mSendDialog->setTimeout(HbPopup::NoTimeout);
		    mSendDialog->setAttribute(Qt::WA_DeleteOnClose);
		    
		    mCancelAction = static_cast<HbAction*>( mLoader->findObject( "cancelAction" ) );
		    
		    connect(mCancelAction, SIGNAL(triggered()), this, SLOT(cancelClicked()));
    }
    else
    	{
			mLastError = DocMLLoadingError;    		
    	}
}


void BTSendDialogWidget::cancelClicked()
{
    mSendDialog->close();
    emit deviceDialogClosed();
}


