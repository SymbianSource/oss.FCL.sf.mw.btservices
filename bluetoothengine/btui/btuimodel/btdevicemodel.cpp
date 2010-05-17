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

#include <btdevicemodel.h>
#include "btdevicedata.h"

/*!
    This Constructor creates new instances of model data structure.
 */
BtDeviceModel::BtDeviceModel( QObject *parent )
    : QAbstractItemModel( parent )
{
   mDeviceData = QSharedPointer<BtDeviceData>( new BtDeviceData( *this ) );
}

/*!
    This Constructor shares the instances of model data structure with the
    given model.
 */
BtDeviceModel::BtDeviceModel( const BtDeviceModel &model, QObject *parent )
    : QAbstractItemModel( parent )
{
    mDeviceData = model.mDeviceData;
}

/*!
    Destructor.
 */
BtDeviceModel::~BtDeviceModel()
{
}

/*!
    Requests the model to searching Bluetooth devices.
    \return true if the request is accepted; false otherwise
 */
bool BtDeviceModel::searchDevice()
{
    return mDeviceData->searchDevice();
}

/*!
    Cancels a possible outstanding device search request.
 */
void BtDeviceModel::cancelSearchDevice()
{
    mDeviceData->cancelSearchDevice();
}

/*!
    Removes transient (not-in-registry) devices 
    which were added as the result of device search.
 */
void BtDeviceModel::removeTransientDevices()
{
    mDeviceData->removeTransientDevices();
}

/*!
    \reimp
 */
QModelIndex BtDeviceModel::index( int row, int column, const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    if ( mDeviceData->isValid( row, column ) ) {
        return createIndex( row, column, mDeviceData.data() );
    }
    // invalid row and column:
    return QModelIndex();
}

/*!
    \reimp
 */
QModelIndex BtDeviceModel::parent( const QModelIndex &child ) const
{
    Q_UNUSED( child );
    // root level, no parent.
    return QModelIndex();
}

/*!
    \reimp
 */
int BtDeviceModel::rowCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return mDeviceData->rowCount();
}

/*!
    \reimp
 */
int BtDeviceModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return mDeviceData->columnCount();
}

/*!
    \reimp
 */
QVariant BtDeviceModel::data( const QModelIndex &index, int role ) const
{
    QVariant val( QVariant::Invalid );
    mDeviceData.data()->data( val, index.row(), index.column(), role );
    return val;
}

QMap<int, QVariant> BtDeviceModel::itemData( const QModelIndex & index ) const
{
    return  mDeviceData.data()->itemData( index.row(), index.column() );
}

/*!
    emits dataChanged signal.
 */
void BtDeviceModel::emitDataChanged( int row, int column, void *parent )
{
    QModelIndex idx = createIndex( row, column, parent );
    emit dataChanged( idx, idx );
}

void BtDeviceModel::emitDataChanged(const QModelIndex &top, const QModelIndex &bottom )
    {
    emit dataChanged( top, bottom );
    }

/*!
    emits deviceSearchCompleted signal.
 */
void BtDeviceModel::emitdeviceSearchCompleted( int error )
{
    emit deviceSearchCompleted( error );
}
