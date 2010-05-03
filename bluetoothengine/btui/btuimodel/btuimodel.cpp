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

#include "btuimodel.h"
#include "btlocalsetting.h"

/*!
    This Constructor creates new instances of model data structure.
 */
BtuiModel::BtuiModel( QObject *parent )
    : QAbstractItemModel( parent )
{
   mLocalSetting = QSharedPointer<BtLocalSetting>( new BtLocalSetting( *this ) );
   bool b = connect( mLocalSetting.data(), SIGNAL(settingDataChanged(int,int,void*)), this, SLOT(btDataChanged(int,int,void*)));
   Q_ASSERT( b );
}

/*!
    This Constructor shares the instances of model data structure with the
    given model.
 */
BtuiModel::BtuiModel( const BtuiModel &model, QObject *parent )
    : QAbstractItemModel( parent )
{
    mLocalSetting = model.mLocalSetting;
    bool b = connect( mLocalSetting.data(), SIGNAL(settingDataChanged(int,int,void*)), this, SLOT(btDataChanged(int,int,void*)));
    Q_ASSERT( b );
}

/*!
    Destructor.
 */
BtuiModel::~BtuiModel()
{
}

/*!
    \reimp
 */
QModelIndex BtuiModel::index( int row, int column, const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    if ( row == LocalSettingRow
         && mLocalSetting->isValid( column ) ) {
        // local setting data:
        return createIndex( row, column, mLocalSetting.data() );
        }
    else if ( row == RemoteDeviceRow ) {
    
    }
    // invalid row and column:
    return QModelIndex();
}

/*!
    \reimp
 */
QModelIndex BtuiModel::parent( const QModelIndex &child ) const
{
    Q_UNUSED( child );
    // root level, no parent.
    return QModelIndex();
}

/*!
    \reimp
 */
int BtuiModel::rowCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return ModelRowCount;
}

/*!
    \reimp
 */
int BtuiModel::columnCount( const QModelIndex &parent ) const
{
    if ( parent.row() == LocalSettingRow 
         && mLocalSetting->isValid( parent.column() ) ) {
        // local setting data:
        return mLocalSetting->itemCount();
        }
    else if ( parent.row() == RemoteDeviceRow ) {
    }
    return 0;
}

/*!
    \reimp
 */
QVariant BtuiModel::data( const QModelIndex &index, int role ) const
{
    QVariant val( QVariant::Invalid );
    if ( index.row() == LocalSettingRow ) {
        mLocalSetting.data()->data( val, index.column(), role );
        }
    else if ( index.row() == RemoteDeviceRow ) {
    }
    return val;
}

QMap<int, QVariant> BtuiModel::itemData( const QModelIndex & index ) const
{
    if ( index.row() == LocalSettingRow ) {
        mLocalSetting.data()->itemData( index.column() );
        }
    else if ( index.row() == RemoteDeviceRow ) {
    }
    return QMap<int, QVariant>();
}

/*!
    Slot for receiving notification of changes from the data source.
    Forward the notification using dataChanged signal.
 */
void BtuiModel::btDataChanged( int row, int column, void *parent )
{
    (void) parent;
    QModelIndex idx = createIndex( row, column, parent );
    emit dataChanged( idx, idx );
}


