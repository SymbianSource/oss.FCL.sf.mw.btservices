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

#ifndef BTDEVICEMODEL_H
#define BTDEVICEMODEL_H

#include <qglobal.h>
#include <QAbstractItemModel>
#include <QSharedPointer>
#include <btuimodeltypes.h>

class BtDeviceData;

/*!
    \class BtDeviceModel
    \brief The data model provided to Bluetooth UIs in QT

    BtDeviceModel provides APIs for accessing the data of remote devices. 
    In addition, signals from this
    model are provided for being informed of data update. 
    
    This model is in one dimension (n rows * 1 columns), i.e.,
    
          row 0 ( a remote device)
          row 1 ( another device)
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

class BTUIMODEL_IMEXPORT BtDeviceModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_ENUMS( DevDataRole DevMajorProperty AVDevMinorProperty PeripheralMinorProperty )

public:

    // the roles for catogerizing Bluetooth device properties
    enum DevDataRole {
        NameAliasRole = Qt::DisplayRole, // QVariant::String, the name showing in UI
        ReadableBdaddrRole = Qt::UserRole, // QString, the readable format of a BD_ADDR (BT Device address)
        LastUsedTimeRole, // QDateTime
        RssiRole,         // QVariant::Int
        MajorPropertyRole,  // QVariant::Int, bits of DevMajorProperty
        MinorPropertyRole,  // QVariant::Int, bits according to an item from DevMajorProperty
        CoDRole,  // QVariant::Int, the value of Class of Device
    };
    
    /*
     * Major device property values.
     */
    enum DevMajorProperty {
        NullProperty   = 0x00000000, // device without any specific filter.
        Bonded         = 0x00000001, // device is in registry and bonded with phone
        Blocked        = 0x00000002, // device is in registry and blocked by user
        RecentlyUsed   = 0x00000004, // device is in registry and was used in last 30 days.
        Trusted        = 0x00000008, // device is in registry and authorized by user.
        InRegistry     = 0x00000010, // device exists in registry.
        
        Connected      = 0x00000020, // device is currently connected to one or more 
                                     // services managed by Bluetooth Engine.
        Connectable    = 0x00000040, // device is connectable to one or more 
                                     // services managed by Bluetooth Engine.
        InRange        = 0x00000100, // device is in range

        // bits re-defined according to Class of Device:
        Computer         = 0x00010000, // a computer
        Phone            = 0x00020000, // a phone
        LANAccessDev     = 0x00040000, // a LAN access point
        AVDev            = 0x00080000, // an A/V device
        Peripheral       = 0x00100000, // a peripheral
        ImagingDev       = 0x00200000, // an imaging device
        WearableDev      = 0x00400000, // a wearable device
        Toy              = 0x00800000, // a toy
        HealthDev        = 0x01000000, // a health device
        UncategorizedDev = 0x02000000, // a generic device that is uncategorized
        
        // all properties derived from BT registry
        RegistryProperties = Bonded |
            Blocked | RecentlyUsed | Trusted | InRegistry,
        
        // all properties derived from CoD
        CodProperties = Computer | Phone | LANAccessDev |
            AVDev | Peripheral | ImagingDev | WearableDev | 
            Toy | HealthDev  | UncategorizedDev,
    };
    
    /*
     * Minor device filters for major property \code AVDev \endcode
     */
    enum AVDevMinorProperty {
        Carkit   = 0x00000001,
        Headset  = 0x00000002,
    };
    
    /*
     * Minor device filters for major property \code Peripheral \endcode
     */
    enum PeripheralMinorProperty {
        Mouse    = 0x00000001,
        Keyboard = 0x00000002,
    };
    
public:
    
    explicit BtDeviceModel( QObject *parent = 0 );
    
    explicit BtDeviceModel( const BtDeviceModel &model, QObject *parent = 0 );
    
    virtual ~BtDeviceModel();
    
    bool searchDevice();
    
    void cancelSearchDevice();
    
    void removeTransientDevices();
    
    // from QAbstractItemModel
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    
    virtual QModelIndex parent( const QModelIndex &child ) const;
    
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual QMap<int, QVariant> itemData( const QModelIndex & index ) const;
    
signals:

    void deviceSearchCompleted(int error);
    
private:
    
    void emitDataChanged(int row, int column, void *parent );
    
    void emitDataChanged(const QModelIndex &top, const QModelIndex &bottom );
    
    void emitdeviceSearchCompleted(int error);
    
private:
    QSharedPointer<BtDeviceData> mDeviceData;

    friend class BtDeviceData;
};

#endif // BTUIMODEL_H
