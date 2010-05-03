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

#ifndef BTUIMODEL_H
#define BTUIMODEL_H


#include <qglobal.h>
#include <QAbstractItemModel>
#include <QSharedPointer>

class BtLocalSetting;
class BtDeviceStore;

// A data item in this model. For example, power state item consists 
// of the information regarding the current Bluetooth power state.
typedef QMap< int, QVariant > BtuiModelDataItem;

// A category of the model data for specific group
typedef QList< BtuiModelDataItem > BtuiModelDataSource;

#ifdef BUILD_BTUIMODEL
#define BTUIMODEL_IMEXPORT Q_DECL_EXPORT
#else
#define BTUIMODEL_IMEXPORT Q_DECL_IMPORT
#endif

/*!
    \class BtuiModel
    \brief The data model provided to Bluetooth UIs in QT

    BtuiModel provides APIs for accessing certain BT data, such as local Bluetooth
    settings and the properties of remote BT devices. In addition, signals from this
    model are provided for being informed of data update. 
    
    This model is in two dimension (2 rows * n columns), i.e.,
    
    row 0 (BT local setting)
          col 0 (local device name)
          col 1 (power state)
          ...
    row 1 (remote devices)
          col 0 ( a remote device)
          col 1 ( another device)
          ...
    
    The data in this model is non-modifiable from the user interface (except device
    search may add a number of in-range devices temporarily) , 
    determined by the characteristics of Bluetooth, the underline BT software 
    services and the BT application requirements. 
    
    Whenever feasible, the detailed description should contain a simple
    example code example:
    \code
    // ...
    \endcode

    \sa \link model-view-programming.html Model/View Programming\endlink
 */

class BTUIMODEL_IMEXPORT BtuiModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_ENUMS( RowId LocalSettingCol LocalSettingDataRole )

public:
    
    /**
     * The parent row identifier of data model. Data in this model is grouped into
     * two categories: the local Bluetooth settings and the remote device 
     * database. The data of each category is stored in multiple 
     * rows of a specified row.
     */
    enum RowId {
        LocalSettingRow = 0,
        RemoteDeviceRow = 1,
        ModelRowCount = 2
    };
    //Q_DECLARE_FLAGS(Rows, BtuiModelRow)

    /**
     * child row identifiers of the local setting row
     */
    enum LocalSettingCol {
        BluetoothNameCol = 0,
        PowerStateCol ,
        VisibilityCol,
        SapModeCol,
        AllowedInOfflineCol,
        LocalSettingColCount,
    };
    //Q_DECLARE_FLAGS(BtuiModelLocalSettings, BtuiModelLocalSettingColumn)
    
    /**
     * Data roles that the items in the local setting row
     */
    enum LocalSettingDataRole {
        SettingIdentity = Qt::WhatsThisRole,
        settingDisplay = Qt::DisplayRole,
        SettingValue = Qt::EditRole,
        SettingValueParam = Qt::UserRole + 1,  // e.g., temp visibility time     
    };

public:
    
    explicit BtuiModel( QObject *parent = 0 );
    
    explicit BtuiModel( const BtuiModel &model, QObject *parent = 0 );
    
    virtual ~BtuiModel();
    
    // from QAbstractItemModel
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    
    virtual QModelIndex parent( const QModelIndex &child ) const;
    
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual QMap<int, QVariant> itemData( const QModelIndex & index ) const;
    
public slots:
    
    void btDataChanged(int row, int column, void *parent );
    
private:
    QSharedPointer<BtLocalSetting> mLocalSetting;
};

//Q_DECLARE_OPERATORS_FOR_FLAGS(BtuiModel::Rows)

Q_DECLARE_METATYPE(BtuiModelDataItem)
Q_DECLARE_METATYPE(BtuiModelDataSource)

#endif // BTUIMODEL_H
