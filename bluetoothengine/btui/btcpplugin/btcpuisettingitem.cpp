/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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

#include "btcpuisettingitem.h"
#include <cpitemdatahelper.h>

#include <HbInstance>

BtCpUiSettingItem::BtCpUiSettingItem(CpItemDataHelper &itemDataHelper,
													   const QString &text /*= QString()*/,
													   const QString &description /*= QString()*/,
													   const HbIcon &icon /*= HbIcon()*/,
													   const HbDataFormModelItem *parent /*= 0*/)
													   : CpSettingFormEntryItemData(itemDataHelper,
													   text,
													   description,
													   icon,
													   parent)
{
}

BtCpUiSettingItem::~BtCpUiSettingItem()
{

}

void BtCpUiSettingItem::onLaunchView()
{
    mModel = new BtuiModel();
    
    mMainWindow = hbInstance->allMainWindows().first();
    
    mBtMainView = new BtCpUiMainView(*mModel);
    
    mCpView = mMainWindow->currentView();
    
    mMainWindow->addView(mBtMainView);
    mMainWindow->setCurrentView(mBtMainView);
    
    connect(mBtMainView, SIGNAL(aboutToClose()), this, SLOT(handleCloseView()));
    
}

void BtCpUiSettingItem::handleCloseView()
{
    mBtMainView->deactivateView();
    mMainWindow->setCurrentView(mCpView);
    delete mBtMainView;
    delete mModel;
}


CpBaseSettingView *BtCpUiSettingItem::createSettingView() const
{
	return 0;
}
