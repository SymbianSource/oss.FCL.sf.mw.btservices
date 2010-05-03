/*
* ============================================================================
*  Name        : btcpuibaseclass.cpp
*  Part of     : BluetoothUI / btapplication       *** Info from the SWAD
*  Description : Implements the baseclass for all views in btapplication.
*
*  Copyright � 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.1
*/

#include "btcpuibaseview.h"
#include <HbAction.h>

/*!
    Constructor.
 */
BtCpUiBaseView::BtCpUiBaseView( BtuiModel &model, QGraphicsItem *parent )
    :CpBaseSettingView( 0 , parent ),mModel(model)
{

}

/*!
    Destructor.
 */
BtCpUiBaseView::~BtCpUiBaseView()
{

}




