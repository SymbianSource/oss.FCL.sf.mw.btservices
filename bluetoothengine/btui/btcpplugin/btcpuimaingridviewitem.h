/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  device view item
*
*/


#ifndef BTCPUIMAINGRIDVIEWITEM_H
#define BTCPUIMAINGRIDVIEWITEM_H

//#include <hblistviewitem.h>
#include <hbgridviewitem.h>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <QtGui/QGraphicsGridLayout>
#include "btuimodelsortfilter.h"

class BtCpUiMainGridViewItem : public HbGridViewItem
{
    Q_OBJECT
    
public:
    BtCpUiMainGridViewItem(QGraphicsItem * parent = 0);
    ~BtCpUiMainGridViewItem();
    
    HbAbstractViewItem * createItem();
    void updateChildItems();
    
    void setModelSortFilter(BtuiModelSortFilter *filter);
    
private:

    HbLabel *mDevTypeIconLabel;
    HbLabel *mDeviceNameLabel;
    HbLabel *mDevTypeTextLabel;
    BtuiModelSortFilter*    mBtuiModelSortFilter;
    
};

#endif /* BTCPUIMAINGRIDVIEWITEM_H */
