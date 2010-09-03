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


#include "btmsgviewer.h"
#include "apmstd.h"
#include <xqaiwrequest.h>
#include <xqconversions.h>
#include <apgcli.h>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <hbapplication.h>
#include <hbdevicedialog.h>

const QString KMimeTypeVCard("text/X-vCard");
const QString deviceDialogType = "com.nokia.hb.btdevicedialog/1.0";


BTMsgViewer::BTMsgViewer(QObject* parent)
: XQServiceProvider("com.nokia.services.btmsgdispservices.displaymsg", parent), 
  mCurrentRequestIndex(0),
  mDialog(0),
  mError(0)
    {    
    publishAll();
    connect(this, SIGNAL(clientDisconnected()), qApp, SLOT(quit()));
    }

BTMsgViewer::~BTMsgViewer()
    {
    delete mDialog;
    }

void BTMsgViewer::displaymsg( int messageId )
    { 
    mCurrentRequestIndex = setCurrentRequestAsync(); 
    CBtMsgViewerUtils* btViewerUtils = 0;
    
    TRAP(mError, btViewerUtils = CBtMsgViewerUtils::NewL());  
    if(isError(mError))
        {
        if(btViewerUtils)
            delete btViewerUtils;
        
        QVariant retVal(mError); 
        completeRequest(mCurrentRequestIndex, retVal);
        return;
        }
        
    HBufC* fileName = 0;
    fileName = btViewerUtils->GetMessagePath(messageId, mError);
    if( isError(mError))
        {
        if(fileName)
            delete fileName;
        
        delete btViewerUtils;

        if(mError == KErrNotFound || mError == KErrPathNotFound)
            {
            launchErrordDialog(TBluetoothDialogParams::EFileMoved);
            }
        else if(mError == KErrNotReady)
            {
            launchErrordDialog(TBluetoothDialogParams::EDriveNotFound);
            }
        else
            {
            QVariant retVal(mError); 
            completeRequest(mCurrentRequestIndex, retVal);
            }
        return;
        }
    
    QString attachmentFName = XQConversions::s60DescToQString(fileName->Des());
    QString mimeType = XQConversions::s60Desc8ToQString(btViewerUtils->GetMimeType()->Des());
    delete fileName;
    delete btViewerUtils;
    
    if(mimeType == KMimeTypeVCard)
        {
        mError = KErrGeneral;
        
        /*todo: copyVCardToTemp() has to be removed when phonebook updates it's capabilites to
                access messages from private folder*/
        QString newfilepath = copyVCardToTemp(attachmentFName);
    
        QString service("com.nokia.services.phonebookservices");
        QString interface("Fetch");
        QString operation("editCreateNew(QString)");
        XQApplicationManager appManager;
        XQAiwRequest* request = appManager.create(service, interface, operation, true); //embedded
        if(request)
        {
            QList<QVariant> args;
            args << newfilepath;
            request->setArguments(args);
            QVariant retValue;
            bool res = request->send(retValue);
            if  (!res) 
                {
                mError = request->lastError();
                }
            else
                {
                mError = retValue.toInt();
                }
            
            delete request;
        }

        /*todo: copyVCardToTemp() has to be removed when phonebook updates it's capabilites to
                access messages from private folder*/        
        deleteVCardFromTemp(newfilepath);
        
        QVariant retVal(mError); 
        completeRequest(mCurrentRequestIndex, retVal);
        return;
        }
    
    XQSharableFile sf;
    XQAiwRequest* request = 0;

    if (!sf.open(attachmentFName)) {
    QVariant retVal(KErrGeneral); 
    completeRequest(mCurrentRequestIndex, retVal);
    return;
    }

    // Get handlers
    XQApplicationManager appManager;
    QList<XQAiwInterfaceDescriptor> fileHandlers = appManager.list(sf);
    if (fileHandlers.count() > 0) {
        XQAiwInterfaceDescriptor d = fileHandlers.first();
        request = appManager.create(sf, d);

        if (!request) {
            sf.close();
            
            QVariant retVal(KErrGeneral); 
            completeRequest(mCurrentRequestIndex, retVal);
            return;
        }
    }
    else {
        sf.close();
        
        QVariant retVal(KErrGeneral); 
        completeRequest(mCurrentRequestIndex, retVal);
        return;
    }

    request->setEmbedded(true);
    request->setSynchronous(true);

    // Fill args
    QList<QVariant> args;
    args << qVariantFromValue(sf);
    request->setArguments(args);

    int err = 0;
    bool res = request->send();
    if  (!res) 
        {
        err = request->lastError();
        }

    // Cleanup
    sf.close();
    delete request;
    
    QVariant retVal(err); 
    completeRequest(mCurrentRequestIndex, retVal);
    return;
    }

bool BTMsgViewer::isError(int err)
    {
    return ((err < 0)?true:false);
    }

QString BTMsgViewer::copyVCardToTemp(const QString& filepath)
{
    QDir tempDir;
    QString tempFilePath(QDir::toNativeSeparators(tempDir.tempPath()));
    tempFilePath.append(QDir::separator());
    QFileInfo fInfo(filepath);
    tempFilePath.append(fInfo.fileName());
    QFile::copy(filepath, tempFilePath);
    return tempFilePath;
}

void BTMsgViewer::deleteVCardFromTemp(const QString& filepath)
{
    QFile::remove(filepath);
}

void BTMsgViewer::launchErrordDialog(int dialogTitle)
{
    if(!mDialog)
        {
        mDialog = new HbDeviceDialog();
        }
    
    connect(mDialog, SIGNAL(deviceDialogClosed()), this, SLOT(handledialogClosed()));
    QVariantMap parameters;
    
    parameters.insert(QString::number(TBluetoothDialogParams::EDialogType), 
      QString::number(TBluetoothDialogParams::EInformationDialog));
    
    parameters.insert(QString::number(TBluetoothDialogParams::EDialogTitle),
      QString::number(dialogTitle));
    
    mDialog->show(deviceDialogType, parameters);    
}

void BTMsgViewer::handledialogClosed()
{
    QVariant retVal(mError); 
    completeRequest(mCurrentRequestIndex, retVal);
}
