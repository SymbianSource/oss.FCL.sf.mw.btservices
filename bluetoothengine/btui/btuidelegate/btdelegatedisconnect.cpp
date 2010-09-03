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
#include "btdelegatedisconnect.h"
#include "btuiutil.h"
#include "btuiiconutil.h"
#include <QModelIndex>
#include <hblabel.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <hbnotificationdialog.h>
#include <bluetoothuitrace.h>

BtDelegateDisconnect::BtDelegateDisconnect(            
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtengConnMan(0), mPhyLinks(0),
	 mActiveHandling(false), mAddrArrayIndex(0), mDisconOpt(DisconUnknown)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

BtDelegateDisconnect::~BtDelegateDisconnect()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    delete mBtengConnMan;
    delete mPhyLinks;
    mSocketServ.Close();
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}


/*!
    Returns the supported editor types.
    \return the sum of supported editor types
 */
int BtDelegateDisconnect::supportedEditorTypes() const
{
    return BtDelegate::DisconnectService
           | BtDelegate::DisconnectAllConnections;
}

/*!
 * disconnects remote device(s) from local device
 * params of type QList<QVariant>: 
 *         1) DisconnectOption (either ServiceLevel, PhysicalLink or AllOngoingConnections)
 *         2) remote device address (QString, not needed for AllOngoingConnections)
 */
void BtDelegateDisconnect::exec( const QVariant &params )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    int err;
    
    // check if in use already
    if ( mActiveHandling ) {
        emit delegateCompleted(KErrInUse, this);
        BOstraceFunctionExitExt( DUMMY_DEVLIST, this, KErrInUse);
        return;
    }
    
    // check parameters
    QList<QVariant> paramList = params.value< QList<QVariant> >(); 
    if (!((paramList.count() == 1) || (paramList.count() == 2))) {
        // wrong parameter count
        emit delegateCompleted( KErrArgument, this ); 
        BOstraceFunctionExitExt( DUMMY_DEVLIST, this, KErrArgument );
        return;
    }
    
    // create btengconnman
    if ( ! mBtengConnMan ){
        TRAP( err, mBtengConnMan = CBTEngConnMan::NewL(this) );
    }
    if( err ) {
        emit delegateCompleted(err, this);
        BOstraceFunctionExitExt( DUMMY_DEVLIST, this, err );
        return;
    }
    
    mDisconOpt = (DisconnectOption)paramList.at(0).toInt();    // DisconnectOption
    
    if ( (mDisconOpt == ServiceLevel) || (mDisconOpt == PhysicalLink) ) {
        // check 2nd parameter is ok
        if ((paramList.count() != 2) || !paramList.at(1).canConvert<QString>()) {
            emit delegateCompleted( KErrArgument, this ); 
            BOstraceFunctionExitExt( DUMMY_DEVLIST, this, KErrArgument );
            return;
        }
        QString strBtAddr = paramList.at(1).toString();  // remote device to operate on
        BtTraceQString1( TRACE_DEBUG, DUMMY_DEVLIST, "device addr=", strBtAddr);
        addrReadbleStringToSymbian( strBtAddr, mBtEngAddr );
        
        mActiveHandling = true;

        if (mDisconOpt == ServiceLevel){
            disconnectServiceLevel();
        }
        else if (mDisconOpt == PhysicalLink){
            disconnectPhysicalLink();       
        }
    } 
    else if ( mDisconOpt == AllOngoingConnections ) {
        err = mBtengConnMan->GetConnectedAddresses(mDevAddrArray);
        //Added this condition because GetConnectedAddresses returns 0 even if no addresses
        //are returned.
        if(err || !(mDevAddrArray.Count())) {
            emit delegateCompleted(err, this);
            BOstraceFunctionExitExt( DUMMY_DEVLIST, this, err );
            return;
        }
        
        mActiveHandling = true;
        
        disconnectAllConnections_service();
    } 
    else {
        BTUI_ASSERT_X( 0, "BtDelegateDisconnect::exec()", "incorrect parameter" );
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}


void BtDelegateDisconnect::disconnectAllConnections_service() 
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    QString btStringAddr;
    addrSymbianToReadbleString( btStringAddr, mDevAddrArray[mAddrArrayIndex] );
    QModelIndex start = deviceModel()->index(0,0);
    QModelIndexList indexList = deviceModel()->match(start,BtDeviceModel::ReadableBdaddrRole, btStringAddr);
    // ToDo:  what happens if device not found from the model?  error handling needed!
    QModelIndex index = indexList.at(0);
    
    mBtEngAddr = mDevAddrArray[mAddrArrayIndex];
    
    disconnectServiceLevel(); 
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDisconnect::disconnectAllConnections_physical()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    QString btStringAddr;
    addrSymbianToReadbleString( btStringAddr, mDevAddrArray[mAddrArrayIndex] );
    QModelIndex start = deviceModel()->index(0,0);
    QModelIndexList indexList = deviceModel()->match(start,BtDeviceModel::ReadableBdaddrRole, btStringAddr);
    // ToDo:  what happens if device not found from the model?  error handling needed!
    QModelIndex index = indexList.at(0);
    
    mBtEngAddr = mDevAddrArray[mAddrArrayIndex];
    
    disconnectPhysicalLink();
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDisconnect::disconnectServiceLevel()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    int err;
    TBTEngConnectionStatus connStatus = EBTEngNotConnected;
    BtTraceBtAddr1( TRACE_DEBUG, DUMMY_DEVLIST, "calling btengconnman->isConnected() with device addr=", mBtEngAddr );
    err = mBtengConnMan->IsConnected(mBtEngAddr, connStatus);
    BOstraceExt2( TRACE_DEBUG, DUMMY_DEVLIST, "isConnected() ret = %d, conn status (0/2) = %d",  err, connStatus);
    
    //if ( !err && (connStatus == EBTEngConnected) ) {
        // asynchronous call, see callback DisconnectComplete()
        BtTraceBtAddr1( TRACE_DEBUG, DUMMY_DEVLIST, "btengconnman->Disconnect() with param device addr=", mBtEngAddr );
        err = mBtengConnMan->Disconnect(mBtEngAddr, EBTDiscGraceful);  
        BOstraceExt1( TRACE_DEBUG, DUMMY_DEVLIST, "Disconnect() ret = %d",  err);
    //}
    if (err) {
        // handle error
        disconnectServiceLevelCompleted(err);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}
        
void BtDelegateDisconnect::disconnectPhysicalLink()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    int err;
    if ( !mSocketServ.Handle() ) {
        err = mSocketServ.Connect();
    }
    if ( !err && !mPhyLinks ) {
        TRAP( err, 
            mPhyLinks = CBluetoothPhysicalLinks::NewL( *this, mSocketServ ) );
        if (err) {
            disconnectPhysicalLinkCompleted(err); // handle error
        }
    }
    // ToDo:  consider using DisconnectAll()
    BtTraceBtAddr1( TRACE_DEBUG, DUMMY_DEVLIST, "phy->Disconnect() with param device addr=", mBtEngAddr );
    err = mPhyLinks->Disconnect( mBtEngAddr );  // async call, see callback HandleDisconnectCompleteL()
    BOstraceExt1( TRACE_DEBUG, DUMMY_DEVLIST, "Phy Disconnect() ret = %d",  err);
    if (err) {
        disconnectPhysicalLinkCompleted(err);  // handle error
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDisconnect::disconnectServiceLevelCompleted(int err)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, err );   
    if (mDisconOpt == ServiceLevel){
        mActiveHandling = false;
        emit delegateCompleted(err, this);
    }
    else if (mDisconOpt == AllOngoingConnections){
        if (err){
            mActiveHandling = false;
            emit delegateCompleted(err, this);
        }
        else{
            mAddrArrayIndex++;
            if ( mAddrArrayIndex < mDevAddrArray.Count()) {
                disconnectAllConnections_service();  // recursive call
            }
            else {
                mDevAddrArray.Reset();
                mAddrArrayIndex = 0;
                err = mBtengConnMan->GetConnectedAddresses(mDevAddrArray);
                if(err) {
                    mActiveHandling = false;
                    emit delegateCompleted(err, this);
                    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, err );
                    return;
                }
                disconnectAllConnections_physical();
            }
        }
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDisconnect::disconnectPhysicalLinkCompleted(int err)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, err );   
    if (mDisconOpt == PhysicalLink){
        mActiveHandling = false;
        emit delegateCompleted(err, this);
    }
    else if (mDisconOpt == AllOngoingConnections) {
        if (err) {
            mActiveHandling = false;
            emit delegateCompleted(err, this);
        }
        else{
            mAddrArrayIndex++;
            if ( mAddrArrayIndex < mDevAddrArray.Count()) {
                disconnectAllConnections_physical();  // recursive call
            }
            else {  // DONE!
                //TODO: check if there is still ongoing connection from BTEngConnMan. and close them again if there is any new 
                mActiveHandling = false;
                emit delegateCompleted(KErrNone, this);
            }
        }
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDisconnect::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aErr );  
    BtTraceBtAddr1( TRACE_DEBUG, DUMMY_DEVLIST, "device addr=", aAddr );
    Q_UNUSED(aAddr);
    Q_UNUSED(aConflicts);  
    Q_UNUSED(aErr);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDisconnect::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aErr );  
    BtTraceBtAddr1( TRACE_DEBUG, DUMMY_DEVLIST, "device addr=", aAddr );
    if ( mBtEngAddr != aAddr || !mActiveHandling ) { 
        BOstraceFunctionExit1( DUMMY_DEVLIST, this );
        return;
    }
    disconnectServiceLevelCompleted(aErr);  
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}


void BtDelegateDisconnect::cancel()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDisconnect::HandleCreateConnectionCompleteL( TInt err )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, err );   
    Q_UNUSED( err );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

// callback for CBluetoothPhysicalLinks::Disconnect()
void BtDelegateDisconnect::HandleDisconnectCompleteL( TInt err )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, err );  
    if ( !mActiveHandling ) {
        BOstraceFunctionExit1( DUMMY_DEVLIST, this );
        return;
    } 
    disconnectPhysicalLinkCompleted(err);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

// callback for CBluetoothPhysicalLinks::DisconnectAll()
void BtDelegateDisconnect::HandleDisconnectAllCompleteL( TInt err )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, err );   
    Q_UNUSED( err );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}




