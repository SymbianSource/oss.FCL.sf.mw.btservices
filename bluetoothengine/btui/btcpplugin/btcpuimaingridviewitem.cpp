/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BTUISearchListViewItem implementation
*
*/


#include "btcpuimaingridviewitem.h"
#include "btdevicemodel.h"
#include "btuiiconutil.h"
#include <QGraphicsGridLayout>
#include <hbstyle.h>
#include <bluetoothuitrace.h>

BtCpUiMainGridViewItem::BtCpUiMainGridViewItem(QGraphicsItem * parent) :
    HbGridViewItem(parent)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    mDeviceNameLabel = 0;
    mDevTypeIconLabel = 0;
    mDevTypeTextLabel = 0;
    mBtuiModelSortFilter = ((BtCpUiMainGridViewItem *)parent)->mBtuiModelSortFilter;
    BOstraceFunctionExit0(DUMMY_DEVLIST); 
}

BtCpUiMainGridViewItem::~BtCpUiMainGridViewItem()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

/*
 * This method is called by the HbListView when it needs a new
 * view item element.
 *
 */
HbAbstractViewItem * BtCpUiMainGridViewItem::createItem()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    HbAbstractViewItem *item = new BtCpUiMainGridViewItem(*this); 
    BOstraceFunctionExitExt(DUMMY_DEVLIST, this, item);
    return item;
}

/*!
   UpdateChildItem updates the item graphics.
   Screen elements are created once if not already done.  This may increase the overall memory 
   consumption of the application, however, this is deemed inconsequential.  There might be a small 
   performance improvement with current style.
 */
void BtCpUiMainGridViewItem::updateChildItems()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    QModelIndex index;
    
    // Get device name from model
    if (mBtuiModelSortFilter)
        index = mBtuiModelSortFilter->mapToSource(modelIndex());
    else
        index = modelIndex();

    // create new icon label if needed
    if (!mDevTypeIconLabel) {
        mDevTypeIconLabel = new HbLabel(this);
        HbStyle::setItemName(mDevTypeIconLabel, "deviceIcon");
    }
   
    // create new label if needed
    if (!mDeviceNameLabel) {
        mDeviceNameLabel = new HbLabel(this);
        HbStyle::setItemName(mDeviceNameLabel, "deviceName");
    }
    
    // create new label if needed
    if (!mDevTypeTextLabel) {
        mDevTypeTextLabel = new HbLabel(this);
        HbStyle::setItemName(mDevTypeTextLabel, "deviceType");
    }
        
    QString data = index.data(Qt::DisplayRole).toString();
    int cod = (index.data(BtDeviceModel::CoDRole)).toInt();
    int majorProperty = (index.data(BtDeviceModel::MajorPropertyRole)).toInt();
    
    mDeviceNameLabel->setPlainText(data);
    mDevTypeTextLabel->setPlainText( getDeviceTypeString( cod ));
    
    HbIcon icon = 
    getBadgedDeviceTypeIcon( cod, majorProperty, 
            BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight );
    mDevTypeIconLabel->setIcon(icon);
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}


void BtCpUiMainGridViewItem::setModelSortFilter(BtuiModelSortFilter *filter)
{
    mBtuiModelSortFilter = filter;
}
