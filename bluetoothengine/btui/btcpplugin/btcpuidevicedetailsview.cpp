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
* Description:  BtCpUiDeviceDetailsView implementation
*
*/
#include "btcpuidevicedetailsview.h"
#include <hbdataform.h>
#include <hbaction.h>
#include <bluetoothuitrace.h>
#include <HbParameterLengthLimiter>

// docml to load
const char* BTUI_DEVICEDETAILSVIEW_DOCML = ":/docml/bt-device-details-view.docml";

BtCpUiDeviceDetailsView::BtCpUiDeviceDetailsView(QGraphicsItem *parent)
    :CpBaseSettingView( 0 , parent )
{
   
    mSoftKeyBackAction = new HbAction(Hb::BackNaviAction, this);
    BTUI_ASSERT_X(mSoftKeyBackAction, "BtCpUiDeviceDetailsView::BtCpUiDeviceDetailsView", "can't create back action");

    // Create view for the application.
    // Set the name for the view. The name should be same as the view's
    // name in docml.
    setObjectName("bt_device_details_view");

    mLoader = new HbDocumentLoader();
    // Pass the view to documentloader. Document loader uses this view
    // when docml is parsed, instead of creating new view.
    QObjectList objectList;
    objectList.append(this);
    mLoader->setObjectTree(objectList);

    // read view info from docml file
    bool ok = false;
    mLoader->load( BTUI_DEVICEDETAILSVIEW_DOCML, &ok );
    // Exit if the file format is invalid
    BTUI_ASSERT_X( ok, "bt-device-details-view", "Invalid docml file" );
    
    mLayout = new QGraphicsLinearLayout(Qt::Vertical, this);
    mGroupBox = new HbGroupBox();
    mLayout->addItem(mGroupBox);
    
    setLayout( mLayout );

}

BtCpUiDeviceDetailsView::~BtCpUiDeviceDetailsView()
{

}

void BtCpUiDeviceDetailsView::setDeviceName(const QString &deviceName)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    
    if(mGroupBox) {        
        QString heading = HbParameterLengthLimiter(hbTrId(
                "txt_bt_subhead_1_details")).arg(deviceName);

        mGroupBox->setHeading(heading);
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetailsView::addItem(HbDataForm *item)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (item && mLayout) {
        mLayout->addItem(item);
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetailsView::removeItem(HbDataForm *item)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (item && mLayout) {
        //With out this item is not removed from the view.
        item->setVisible(false);
        mLayout->removeItem(item);
        delete item;
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}

void BtCpUiDeviceDetailsView::removeAllItems()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if(mLayout) {
        QGraphicsLayoutItem *item;
        int count = mLayout->count();
        for(int i=count-1; i>=0; i--) {
            item = mLayout->itemAt(i);
            mLayout->removeItem(item);
            delete item;
        }
    }
    BOstraceFunctionExit0(DUMMY_DEVLIST);
}
