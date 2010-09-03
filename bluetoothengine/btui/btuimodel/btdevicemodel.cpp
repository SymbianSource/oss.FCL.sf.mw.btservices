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
#include "btdevicemodel_p.h"
#include "bluetoothuitrace.h"
#include "btuidevtypemap.h"

/*!
    This Constructor creates new instances of model data structure.
 */
BtDeviceModel::BtDeviceModel( QObject *parent )
    : QAbstractItemModel( parent )
{
   d = QSharedPointer<BtDeviceModelPrivate>( new BtDeviceModelPrivate( *this ) );
   connectModelSignals();
}

/*!
    This Constructor shares the private implementation of the device model.
 */
BtDeviceModel::BtDeviceModel( const BtDeviceModel &model, QObject *parent )
    : QAbstractItemModel( parent )
{
    d = model.d;
    connectModelSignals();
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
    return d->searchDevice();
}

/*!
    Cancels a possible outstanding device search request.
 */
void BtDeviceModel::cancelSearchDevice()
{
    d->cancelSearchDevice();
}

/*!
    Removes transient (not-in-registry) devices 
    which were added as the result of device search.
 */
void BtDeviceModel::removeTransientDevices()
{
    d->removeTransientDevices();
}

/*!
    \reimp
 */
QModelIndex BtDeviceModel::index( int row, int column, const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    if ( d->isValid( row, column ) ) {
        return createIndex( row, column, d.data() );
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
    return d->rowCount();
}

/*!
    \reimp
 */
int BtDeviceModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return d->columnCount();
}

/*!
    \reimp
 */
QVariant BtDeviceModel::data( const QModelIndex &index, int role ) const
{
    QVariant val( QVariant::Invalid );
    d.data()->data( val, index.row(), index.column(), role );
    return val;
}

QMap<int, QVariant> BtDeviceModel::itemData( const QModelIndex & index ) const
{
    return  d.data()->itemData( index.row(), index.column() );
}

/*!
    \Reimplemented just to match the model based on major property
 */
QModelIndexList BtDeviceModel::match(const QModelIndex & start, int role,
        const QVariant & value, int hits, Qt::MatchFlags flags ) const
{
    DevDataRole propRole = (DevDataRole)role;
    if(BtDeviceModel::MajorPropertyRole == propRole) {
        BtuiDevProperty::DevMajorProperty propValue = (BtuiDevProperty::DevMajorProperty)value.toInt();
        QModelIndexList indexList;
        int count = rowCount();
        if (count && (start.row() < count)) {
            for(int row = start.row(); row < count; row++) {
                QModelIndex modelIndex = index(row,0);
                int majorPropRole = data(modelIndex,BtDeviceModel::MajorPropertyRole).toInt();
                if (majorPropRole & propValue) {
                    indexList.append(modelIndex);
                }
            }
        }
        return indexList;
    }
    
    return QAbstractItemModel::match(start, role, value, hits, flags);
}


/*!
    Returns device count of devices having majorProperty.
 */
int BtDeviceModel::deviceCount(int majorProperty)
{
    int deviceCount = 0;
    int count = rowCount();
    BtuiDevProperty::DevMajorProperty propValue = (BtuiDevProperty::DevMajorProperty)majorProperty;
    
    for(int row = 0; row < count; row++) {
        QModelIndex modelIndex = index(row,0);
        int majorPropRole = data(modelIndex,BtDeviceModel::MajorPropertyRole).toInt();
        if (majorPropRole & propValue) {
            deviceCount++;
        }
    }

    return deviceCount;
}


/*!
    emits dataChanged signal.
 */
void BtDeviceModel::deviceDataChanged( int row, void *parent )
{
    QModelIndex idx = createIndex( row, 0, parent );
    emit dataChanged( idx, idx );
}

/*!
    emits dataChanged signal.
 */
void BtDeviceModel::deviceDataChanged( int first, int last, void *parent )
{
    QModelIndex top = createIndex( first, 0, parent );
    QModelIndex bottom = createIndex( last, 0, parent );
    emit dataChanged( top, bottom );
}

/*!
    call beginInsertRows.
 */
void BtDeviceModel::beginInsertDevices(int first, int last, void *parent)
{
    Q_UNUSED( parent);
    beginInsertRows(QModelIndex(), first, last); 
}

/*!
    calls endInsertRows.
 */
void BtDeviceModel::BtDeviceModel::endInsertDevices()
{
    endInsertRows();
}

/*!
    calls beginRemoveRows.
 */
void BtDeviceModel::beginRemoveDevices(int first, int last, void *parent)
{
    Q_UNUSED( parent);
    beginRemoveRows(QModelIndex(), first, last); 
}

/*!
    calls endRemoveRows.
 */
void BtDeviceModel::endRemoveDevices()
{
    endRemoveRows();
}

/*!
    emits deviceSearchCompleted signal.
 */
void BtDeviceModel::emitDeviceSearchCompleted( int error )
{
    emit deviceSearchCompleted( error );
}

/*!
 connects all signals of private impl to slots of this
 */
void BtDeviceModel::connectModelSignals()
{
    bool ok = connect(d.data(), SIGNAL(deviceDataChanged(int,void*)), SLOT(deviceDataChanged(int,void*)));
    BTUI_ASSERT_X( ok, "BtDeviceModel", "deviceDataChanged can't connect" );
    ok = connect(d.data(), SIGNAL(deviceDataChanged(int,int,void*)), SLOT(deviceDataChanged(int,int,void*)));
    BTUI_ASSERT_X( ok, "BtDeviceModel", "deviceDataChanged can't connect 2" );
    ok = connect(d.data(), SIGNAL(beginInsertDevices(int,int,void*)), SLOT(beginInsertDevices(int,int,void*)));
    BTUI_ASSERT_X( ok, "BtDeviceModel", "beginInsertDevices can't connect" );
    ok = connect(d.data(), SIGNAL(endInsertDevices()), SLOT(endInsertDevices()));
    BTUI_ASSERT_X( ok, "BtDeviceModel", "endInsertDevices can't connect" );    
    ok = connect(d.data(), SIGNAL(beginRemoveDevices(int,int,void*)), SLOT(beginRemoveDevices(int,int,void*)));
    BTUI_ASSERT_X( ok, "BtDeviceModel", "beginRemoveDevices can't connect" );
    ok = connect(d.data(), SIGNAL(endRemoveDevices()), SLOT(endRemoveDevices()));
    BTUI_ASSERT_X( ok, "BtDeviceModel", "endRemoveDevices can't connect" );
    ok = connect(d.data(), SIGNAL(deviceSearchCompleted(int)), SLOT(emitDeviceSearchCompleted(int)));
    BTUI_ASSERT_X( ok, "BtDeviceModel", "emitDeviceSearchCompleted can't connect" );    
}
