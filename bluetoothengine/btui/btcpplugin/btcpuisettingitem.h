/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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


class BtSettingModel;
class BtDeviceModel;
class BtcpuiBaseView;
class HbTranslator;
class HbMainWindow;
class HbView;

class BtcpuiSettingItem : public CpSettingFormEntryItemData
{
	Q_OBJECT
public:
	explicit BtcpuiSettingItem(CpItemDataHelper &itemDataHelper);	 
	virtual ~BtcpuiSettingItem();
	
private slots:
	void onLaunchView();
	void handleCloseMainView();
	void handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
	
private: 
	// From CpSettingFormEntryItemData
	virtual CpBaseSettingView *createSettingView() const;
    
private:
	
	void updateStatus();
	void loadTranslators();
	
private:
	HbMainWindow* mMainWindow;
	
	BtcpuiBaseView *mMainView;
	
	//Owns this model.
	BtSettingModel *mSettingModel;
	BtDeviceModel *mDeviceModel;
	
	HbView *mCpView;
	
	HbTranslator *mViewTranslator;
	HbTranslator *mDialogTranslator;
	
};

#endif //BTCPUISETTINGITEM_H
