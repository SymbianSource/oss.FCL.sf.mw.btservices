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
 * Description:
 *
 */
#include "btindicator.h" 
#include <QVariant.h>
#include "btindicatorconstants.h"


#define LOC_BLUETOOTH hbTrId("txt_bt_dblist_bluetooth") 

const QString BT_APPLICATION("btcpplugin.dll");

struct BTIndicatorInfo
{
    QString DecorationIcon;
    QString MonoDecorationIcon;
};

static const int BTIndicatorCount = 5;


static const BTIndicatorInfo IndicatorInfo[BTIndicatorCount] = { 
     { "qtg_large_bluetooth_off", "qtg_status_bluetooth_off" },
     { "qtg_large_bluetooth","qtg_status_bluetooth" },
     { "qtg_large_bluetooth_hide","qtg_status_bluetooth_hide" },
     { "qtg_large_bluetooth_active_connection", "qtg_status_bluetooth_connection"},
     { "qtg_large_bluetooth_hide_connection", "qtg_status_bluetooth_hide_connection"}
 };


// ----------------------------------------------------------------------------
// BTIndicator::BTIndicator
// ----------------------------------------------------------------------------
BTIndicator::BTIndicator(const QString &indicatorType) :
HbIndicatorInterface(indicatorType,
        HbIndicatorInterface::SettingCategory ,
        InteractionActivated),mRequest(0),mIndicatorStatus(0),mIndicatorlaunch(false)
{
    mSecondaryText << hbTrId("txt_bt_dblist_bluetooth_val_off")
            << hbTrId("txt_bt_dblist_bluetooth_val_on_and_visible")
            << hbTrId("txt_bt_dblist_bluetooth_val_on_and_hidden")
            << hbTrId("txt_bt_dblist_bluetooth_val_visible_and_connected")
            << hbTrId("txt_bt_dblist_bluetooth_val_hidden_and_connected");
}

// ----------------------------------------------------------------------------
// BTIndicator::~BTIndicator
// ----------------------------------------------------------------------------
BTIndicator::~BTIndicator()
{
	delete mRequest;
}


// ----------------------------------------------------------------------------
// BTIndicator::handleInteraction
// ----------------------------------------------------------------------------
bool BTIndicator::handleInteraction(InteractionType type)
{
    if (type == InteractionActivated) 
	{
 /*       if(!mIndicatorlaunch)
            {
            launchBTCpSettingView();
            mIndicatorlaunch = true;
            }*/
        
	}
    return true;
}

// ----------------------------------------------------------------------------
// BTIndicator::indicatorData
// returns the data and icon that needs to be displayed in the universal pop up and indicator menu 
// ----------------------------------------------------------------------------
QVariant BTIndicator::indicatorData(int role) const
{
	QVariant data("");

    if(mIndicatorStatus == 0)
	{
		data = QString();
	}
    else
	{
        switch(role) {
            case PrimaryTextRole: 
				data = QString(LOC_BLUETOOTH);
				break;
            case SecondaryTextRole:
				data = mSecondaryText[mIndicatorStatus];
				break;
            case DecorationNameRole:
				data = IndicatorInfo[mIndicatorStatus].DecorationIcon;
				break;
            case MonoDecorationNameRole :
				data =  IndicatorInfo[mIndicatorStatus].MonoDecorationIcon;
				break;
            default: 
                data = QString();      
				break;
		}
	}
	return data;
}

// ----------------------------------------------------------------------------
// BTIndicator::handleClientRequest
// handles client's activate and deactivate request
// ----------------------------------------------------------------------------
bool BTIndicator::handleClientRequest( RequestType type, 
        const QVariant &parameter)
{
    bool handled(false);
    switch (type) {
        case RequestActivate:
            {
            mIndicatorStatus = parameter.toInt();
            if(( !mIndicatorStatus )&&( !mIndicatorlaunch ))
                {
	            mSecondaryText = QStringList();
                emit deactivate();
                }
            else
                {
                emit dataChanged();
                }
            handled =  true;
            }
            break;
        case RequestDeactivate:
            {
            mSecondaryText = QStringList();
            mIndicatorStatus  = 0;
            emit deactivate();
            }
            break;
        default:
            break;
    }
    return handled;
}


void BTIndicator::launchBTCpSettingView()
{
    if(!mRequest)
    {
        mRequest = mAppMgr.create("com.nokia.symbian.ICpPluginLauncher","launchSettingView(QString,QVariant)",false);

        if (!mRequest)
        {
           qDebug("BTIndicator request returned with NULL");
            return;
        }
        else
        {
            connect(mRequest, SIGNAL(requestOk(QVariant)), SLOT(handleReturnValue(QVariant)));
            connect(mRequest, SIGNAL(requestError(int,QString)), SLOT(handleError(int,QString)));
            // Set arguments for request 
            QList<QVariant> args;
            args << QVariant(BT_APPLICATION);
            mRequest->setArguments(args);
            mRequest->setSynchronous(false);
        }
    }
        // Make the request
    if (!mRequest->send())
    {
        //report error  
        qDebug("BTIndicator::launchBTCpSettingView request not sent");
    }
	
/*	QString service("com.nokia.symbian.ICpPluginLauncher");
    QString operation("launchSettingView(QString,QVariant)");
    QList<QVariant> args;
    XQAiwRequest* request;
    XQApplicationManager appManager;
    request = appManager.create(service, operation, false); // not embedded
    if ( request == NULL )
        {
        //Could not create request because of the arguments passed to the create() API.
        User::Leave(KErrArgument);       
        }
    args << QVariant("btcpplugin.dll");
    request->setArguments(args);
    TInt error = KErrNone;
    if(!request->send())
        {
        // The only likely Symbian error code that can be associated is KErrNotSupported
        
        }
    delete request;*/
    

}


void BTIndicator::handleReturnValue(const QVariant &returnValue)
{
    Q_UNUSED(returnValue);
    mIndicatorlaunch = false;
    if(!mIndicatorStatus )
	{
        emit deactivate();
	}
}

void BTIndicator::handleError(int errorCode,const QString &errorMessage)
{
    Q_UNUSED(errorCode);
    Q_UNUSED(errorMessage);
    mIndicatorlaunch = false;
}
