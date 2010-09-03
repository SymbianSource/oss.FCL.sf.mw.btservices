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

#include "btdelegateremotedevname.h"
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <e32base.h>
#include <bluetoothuitrace.h>
#include <QRegExp>
#include "btuidevtypemap.h"


BtDelegateRemoteDevName::BtDelegateRemoteDevName(BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel,QObject *parent )
    :BtAbstractDelegate( settingModel, deviceModel, parent ), mRegistryActive(0), mRegistryOpened(false)
{
    
}
    
BtDelegateRemoteDevName::~BtDelegateRemoteDevName()
{
    delete mRegistryActive;
    mSymName.Close();
    if (mRegistryOpened)
        mBtRegistry.Close();//there is crash is close the btregistry without opening it first
    mBtRegServ.Close();
}

/*!
    Returns the supported editor types.
    \return the sum of supported editor types
 */
int BtDelegateRemoteDevName::supportedEditorTypes() const
{
    return BtDelegate::ChangeDeviceFriendlyName;
}

/*!
    Validate the bluetooth device name given by the user:
    Extra spaces (' ', '\n', '\t' and '\r') from the beginning, 
    middle and the end of the name are always removed;
    the maximum lengthof a name is 30.
    
    TODO:Add duplicate name checking. If the new name duplicates 
    existing name in registry, prompt user to change the name
 */

bool BtDelegateRemoteDevName::validateName(QString &name )
{
    // remove spaces at the beginning and end:
    name = name.trimmed();
    // regular expression of one or more consecutive spaces:
    QRegExp rx("[ \n\t\r]+");
    name.replace( rx, " ");
    if (name.length() > 30 ) {
        name.resize( 30 );
    }
    return name.length() > 0;
}

void BtDelegateRemoteDevName::exec( const QVariant &params )
{
    QList<QVariant> paramList = params.value< QList<QVariant> >(); 
    QVariant indexVariant = paramList.at(0); 
    QModelIndex index = indexVariant.value<QModelIndex>();
    QVariant nameVariant = paramList.at(1); 
    QString btRemoteDevName = nameVariant.toString();
    
    if (!validateName(btRemoteDevName)){
            emit delegateCompleted(KErrBadName, this);
            return;
    }
    mNewName = btRemoteDevName;

    int error = KErrNone;
    TPtrC ptrName(reinterpret_cast<const TText*>(btRemoteDevName.constData()));
  
    error = mSymName.Create(ptrName.Length());
    
    // todo (review comment): missing proper exception handling. 
    if(error == KErrNone) {
        mSymName.Copy(ptrName);
    }
    else{
        emit delegateCompleted(error,this);
        return;
    }
    
    QString strBtAddr = deviceModel()->data(index,
           BtDeviceModel::ReadableBdaddrRole).toString();

    TBuf<KBTDevAddrSize * 2> buffer(strBtAddr.utf16());
    mSymaddr.SetReadable( buffer );
     
    error = mBtRegServ.Connect();
    if ( error != KErrNone && error != KErrAlreadyExists) {
        emit delegateCompleted(error,this);
        return;
    }

    error = mBtRegistry.Open( mBtRegServ ) ;
    if ( error != KErrNone && error != KErrAlreadyExists) {
        emit delegateCompleted(error,this);
        return;
    }
    mRegistryOpened = true;
    
    if (!mRegistryActive){
        RequestIdentifiers requestId = Unknown;
        TRAP(error, mRegistryActive = CBtSimpleActive::NewL(
                       *this, requestId));
        if(error!=KErrNone) {
            emit delegateCompleted(KErrGeneral, this);
            return;
        }
    }
    //first check if this device is already in the registry
    
    int majorRole = (deviceModel()->data(index,BtDeviceModel::MajorPropertyRole)).toInt();
    int cod = (deviceModel()->data(index,BtDeviceModel::CoDRole)).toInt();
    if (!(majorRole & BtuiDevProperty::InRegistry)) {
        CBTDevice *symBtDevice;
        TRAP( error, {
               symBtDevice = CBTDevice::NewL( mSymaddr );
               symBtDevice->SetDeviceClass(cod);
               RequestIdentifiers requestId = AddDevice; 
               mRegistryActive->SetRequestId(requestId);
               mBtRegistry.AddDeviceL(*symBtDevice, mRegistryActive->iStatus);
               mRegistryActive->GoActive();
            });
    
    }
    else{
        RequestIdentifiers requestId = ModifyFriendlyName; 
        mRegistryActive->SetRequestId(requestId);
        TRAP( error, {
                mBtRegistry.ModifyFriendlyDeviceNameL(mSymaddr, mSymName, mRegistryActive->iStatus);
                mRegistryActive->GoActive();
        });
    }
    if ( error != KErrNone ) {
        emit delegateCompleted(error,this);
    }
  
}

void BtDelegateRemoteDevName::RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus ){
    
    if(aStatus != KErrNone){
        emit delegateCompleted(aStatus, this);
        return;
    }
    int error = KErrNone;
    if ( aActive->RequestId() == AddDevice ){
        RequestIdentifiers requestId = ModifyFriendlyName; 
        mRegistryActive->SetRequestId(requestId);
        TRAP( error, {
                mBtRegistry.ModifyFriendlyDeviceNameL(mSymaddr, mSymName, mRegistryActive->iStatus);
                mRegistryActive->GoActive();
        });
        if(error != KErrNone){
            emit delegateCompleted(error, this);
        }
    }
    else if ( aActive->RequestId() == ModifyFriendlyName ){
        emit delegateCompleted(error, this);
    }
    
}
    
void BtDelegateRemoteDevName::CancelRequest( TInt aRequestId ){
    if ( aRequestId == 1 ){
        mBtRegistry.CancelRequest(mRegistryActive->RequestStatus());
        emit delegateCompleted(KErrCancel, this);
    }
    
}
    
void BtDelegateRemoteDevName::HandleError( CBtSimpleActive* aActive, TInt aError ){
    //TODO: handle the error here
    Q_UNUSED( aActive );
    Q_UNUSED( aError );
    emit delegateCompleted(KErrGeneral,this);
}
