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


#include "btdeviceinfowidget.h"
#include "bluetoothdevicedialogs.h"
#include "btdevicedialogpluginerrors.h"
#include <btuiiconutil.h>
#include <HbLabel>

const int timeOut = 30000;

BtDeviceInfoWidget::BtDeviceInfoWidget(const QVariantMap &parameters)
:mLastError(NoError)
{
    constructDialog(parameters);
}

BtDeviceInfoWidget::~BtDeviceInfoWidget()
{
    
}

bool BtDeviceInfoWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    Q_UNUSED(parameters);
    return true;
}

int BtDeviceInfoWidget::deviceDialogError() const
{
    return mLastError;
}

void BtDeviceInfoWidget::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    mMessageBox->close();
    emit deviceDialogClosed();
}

HbPopup* BtDeviceInfoWidget::deviceDialogWidget() const
{
    return mMessageBox;
}

QObject* BtDeviceInfoWidget::signalSender() const
    {
    return const_cast<BtDeviceInfoWidget*>(this);
    }

bool BtDeviceInfoWidget::constructDialog(const QVariantMap &parameters)
{
    QString keyStrDialogTitle, keyStrResource;
    keyStrDialogTitle.setNum(TBluetoothDialogParams::EDialogTitle);
    QVariantMap::const_iterator i = parameters.constFind(keyStrDialogTitle);
    
    keyStrResource.setNum(TBluetoothDialogParams::EResource);
    QVariantMap::const_iterator j = parameters.constFind(keyStrResource);
    
    if(i != parameters.end())
        {
        int dialogTitle = parameters.value(QString::number(TBluetoothDialogParams::EDialogTitle)).toInt();
        
        switch(dialogTitle)
            {
                case TBluetoothDialogParams::EBTAddress:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeInformation);
                    QString textStr(hbTrId("txt_bt_info_bluetooth_device_address_1"));
                    QString deviceAddr(parameters.value(QString::number(TBluetoothDeviceDialog::ELocalAddress)).toString());
                    mMessageBox->setText(textStr.arg(deviceAddr));
                    HbIcon icon(QString("qtg_large_bluetooth"));
                    mMessageBox->setIcon(icon);
                    mMessageBox->setTimeout(timeOut);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                //TODO: Remove this piece of code if notifier is used in toggle
                case TBluetoothDialogParams::EOfflineQuery:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
                    mMessageBox->setText(hbTrId("txt_bt_info_trun_bluetooth_on_ini_offline_mode"));
                    mMessageBox->setStandardButtons(HbMessageBox::Yes | HbMessageBox::No);
                    HbIcon icon(QString("qtg_large_bluetooth"));
                    mMessageBox->setIcon(icon);
                    mMessageBox->setTimeout(timeOut);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                
                case TBluetoothDialogParams::ERecvFailed:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeInformation);
                    QString textStr(hbTrId("txt_bt_dpophead_receiving_failed"));
                    QString devName(parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceName)).toString());
                    mMessageBox->setText(textStr.arg(devName));
                    int classOfDevice = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass)).toDouble();
                    HbIcon icon = getBadgedDeviceTypeIcon(classOfDevice);
                    mMessageBox->setIcon(icon);
                    mMessageBox->setDismissPolicy(HbPopup::NoDismiss);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
        
                case TBluetoothDialogParams::ESendFailed:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeInformation);
                    QString textStr(hbTrId("txt_bt_dpophead_sending_failed"));
                    QString devName(parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceName)).toString());
                    mMessageBox->setText(textStr.arg(devName));
                    int classOfDevice = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass)).toDouble();
                    HbIcon icon = getBadgedDeviceTypeIcon(classOfDevice);
                    mMessageBox->setIcon(icon);
                    mMessageBox->setDismissPolicy(HbPopup::NoDismiss);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                
                case TBluetoothDialogParams::EMemoryFull:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeWarning);
                    QString textStr(hbTrId("txt_error_info_memory_full_delete_data"));
                    QString driveLetter(parameters.value(QString::number(TBluetoothDeviceDialog::EDriveLetter)).toChar());
                    QString driveName(parameters.value(QString::number(TBluetoothDeviceDialog::EDriveName)).toString());
                    mMessageBox->setText(textStr.arg(driveLetter).arg(driveName));
                    mMessageBox->setDismissPolicy(HbPopup::NoDismiss);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                
                case TBluetoothDialogParams::EDriveNotFound:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeWarning);
                    mMessageBox->setText(hbTrId("txt_bt_info_file_not_fould_memory_card_has_been_r"));
                    mMessageBox->setDismissPolicy(HbPopup::NoDismiss);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                
                case TBluetoothDialogParams::EFileMoved:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeWarning);
                    mMessageBox->setText(hbTrId("txt_bt_info_file_not_fould_it_may_be_removed_or_d"));
                    mMessageBox->setDismissPolicy(HbPopup::NoDismiss);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                
                case TBluetoothDialogParams::EUnsupportedImages:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
                    QString textStr(parameters.value(QString::number(TBluetoothDeviceDialog::EAdditionalDesc)).toString());
                    mMessageBox->setText(textStr);
                    mMessageBox->setStandardButtons(HbMessageBox::Continue| HbMessageBox::Cancel);
                    int classOfDevice = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass)).toDouble();
                    HbIcon icon = getBadgedDeviceTypeIcon(classOfDevice);
                    mMessageBox->setIcon(icon);
                    mMessageBox->setDismissPolicy(HbPopup::NoDismiss);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                
                case TBluetoothDialogParams::ECannotUseSAPTemporarily:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeInformation);
                    mMessageBox->setText(hbTrId("txt_bt_info_sim_access_profile_is_used_next_time_t"));
                    mMessageBox->setDismissPolicy(HbPopup::TapOutside);
                    mMessageBox->setTimeout(HbPopup::StandardTimeout);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                
                case TBluetoothDialogParams::ENoSimInDevice:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
                    mMessageBox->setHeadingWidget(new HbLabel(hbTrId("txt_bt_title_no_sim_card_in_the_device")));
                    mMessageBox->setText(hbTrId("txt_bt_info_do_you_still_want_to_enable_sim_access"));
                    mMessageBox->setStandardButtons(HbMessageBox::Yes | HbMessageBox::No);
                    mMessageBox->setDismissPolicy(HbPopup::TapOutside);
                    mMessageBox->setTimeout(HbPopup::StandardTimeout);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                
                case TBluetoothDialogParams::EUnableToUseSAP:
                {
                    mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
                    mMessageBox->setHeadingWidget(new HbLabel(hbTrId("txt_bt_title_unable_to_enter_sim_access_profile")));
                    mMessageBox->setText(hbTrId("txt_bt_info_try_entering_the_sim_access_profile_ag"));
                    mMessageBox->setStandardButtons(HbMessageBox::Yes | HbMessageBox::No);
                    mMessageBox->setDismissPolicy(HbPopup::TapOutside);
                    mMessageBox->setTimeout(HbPopup::StandardTimeout);
                    connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                    
                default:
                    mLastError = UnknownDeviceDialogError;
                    break;
            }
        }
    
    else if(j != parameters.end())
        {
        int resourceType = parameters.value(QString::number(TBluetoothDialogParams::EResource)).toInt();
        
        switch(resourceType)
            {
            case EIssueOfflineQuery:
                {
                mMessageBox = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
                mMessageBox->setText(hbTrId("txt_bt_info_trun_bluetooth_on_ini_offline_mode"));
                mMessageBox->setStandardButtons(HbMessageBox::Yes | HbMessageBox::No);
                HbIcon icon(QString("qtg_large_bluetooth"));
                mMessageBox->setIcon(icon);
                mMessageBox->setTimeout(timeOut);
                connect(mMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxClosed(int)));
                }break;
                
            default:
                mLastError = UnknownDeviceDialogError;
                break; 
            }
        }
    
    else
        {
        mLastError = UnknownDeviceDialogError;
        }
    
    if(!mLastError)
        {
        mMessageBox->setAttribute(Qt::WA_DeleteOnClose);
        return true;
        }
    
    return false;
}

void BtDeviceInfoWidget::messageBoxClosed(int action)
    {
    QVariantMap data;
    if((action == HbMessageBox::Yes) || (action == HbMessageBox::Continue) || (action == HbMessageBox::Ok))
        {
        data.insert(QString("actionResult"), QVariant(true));
        }
    else if((action == HbMessageBox::No) || (action == HbMessageBox::Cancel))
        {
        data.insert(QString("actionResult"), QVariant(false));
        }
    emit deviceDialogData(data);
    emit deviceDialogClosed();
    }
