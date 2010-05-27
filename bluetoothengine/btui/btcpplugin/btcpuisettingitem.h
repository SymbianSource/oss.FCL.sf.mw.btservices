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

#ifndef BTCPUISETTINGITEM_H
#define BTCPUISETTINGITEM_H

#include <cpsettingformentryitemdata.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>

#include "btcpuimainview.h"

class BtCpUiSettingItem : public CpSettingFormEntryItemData
{
	Q_OBJECT
public:
	explicit BtCpUiSettingItem(CpItemDataHelper &itemDataHelper,
		const QString &text = QString(),
		const QString &description = QString(),
		const HbIcon &icon = HbIcon(),
		const HbDataFormModelItem *parent = 0);	 
	virtual ~BtCpUiSettingItem();
private slots:
	void onLaunchView();
	void handleCloseView();
private:
	virtual CpBaseSettingView *createSettingView() const;
	
private:
	HbMainWindow* mMainWindow;
	
	BtCpUiMainView *mBtMainView;
	
	//Owns this model.
	BtSettingModel *mSettingModel;
	BtDeviceModel *mDeviceModel;
	
	HbView *mCpView;
	
};

#endif //BTCPUISETTINGITEM_H
