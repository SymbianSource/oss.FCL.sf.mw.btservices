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

#include <btsettingmodel.h>
#include "btlocalsetting.h"
#include "bluetoothuitrace.h"

/*!
    This Constructor creates new instances of model data structure.
 */
BtSettingModel::BtSettingModel( QObject *parent )
    : QAbstractItemModel( parent )
{
   mLocalSetting = QSharedPointer<BtLocalSetting>( new BtLocalSetting( *this ) );
}

/*!
    This Constructor shares the instances of model data structure with the
    given model.
 */
BtSettingModel::BtSettingModel( const BtSettingModel &model, QObject *parent )
    : QAbstractItemModel( parent )
{
    mLocalSetting = model.mLocalSetting;
}

/*!
    Destructor.
 */
BtSettingModel::~BtSettingModel()
{
}

/*!
    \reimp
 */
QModelIndex BtSettingModel::index( int row, int column, const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    if ( mLocalSetting->isValid( row, column ) ) {
        return createIndex( row, column, mLocalSetting.data() );
    }
    // invalid row and column:
    return QModelIndex();
}

/*!
    \reimp
 */
QModelIndex BtSettingModel::parent( const QModelIndex &child ) const
{
    Q_UNUSED( child );
    // root level, no parent.
    return QModelIndex();
}

/*!
    \reimp
 */
int BtSettingModel::rowCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return mLocalSetting->rowCount();
}

/*!
    \reimp
 */
int BtSettingModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return mLocalSetting->columnCount();
}

/*!
    \reimp
 */
QVariant BtSettingModel::data( const QModelIndex &index, int role ) const
{
    QVariant val( QVariant::Invalid );
    mLocalSetting.data()->data( val, index.row(), index.column(), role );
    return val;
}

QMap<int, QVariant> BtSettingModel::itemData( const QModelIndex & index ) const
{
    return mLocalSetting.data()->itemData( index.row(), index.column() );
}

/*!
    emits dataChanged signal.
 */
void BtSettingModel::emitDataChanged( int row, int column, void *parent )
{
    QModelIndex idx = createIndex( row, column, parent );
    emit dataChanged( idx, idx );
}

void BtSettingModel::emitDataChanged(const QModelIndex &top, const QModelIndex &bottom )
    {
    emit dataChanged( top, bottom );
    }
