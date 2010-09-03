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

#ifndef BTABSTRACTDELEGATE_H
#define BTABSTRACTDELEGATE_H

#include <QObject>
#include <QVariant>
#include <QModelIndex>
#include <btdelegateconsts.h>

class BtSettingModel;
class BtDeviceModel;

#ifdef BUILD_BTUIDELEGATE
#define BTUIDELEGATE_IMEXPORT Q_DECL_EXPORT
#else
#define BTUIDELEGATE_IMEXPORT Q_DECL_IMPORT
#endif


/*!
    \class BtAbstractDelegate
    \brief the base class for handling user requests from BT application.


    \\sa btuidelegate
 */
class BTUIDELEGATE_IMEXPORT BtAbstractDelegate : public QObject
{
    Q_OBJECT

public:
    explicit BtAbstractDelegate( BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, QObject *parent = 0 );
    
    virtual ~BtAbstractDelegate();

    /*!
     * Tells whether this is executing an operation.
     */
    bool isExecuting();
    
    /*!
     * Returns the sum of supported editor types
     */
    virtual int supportedEditorTypes() const = 0;

    /*!
     * Execute an operation, e.g., powering BT.
     * \param params the parameters which is specific per
     * delegate.
     */
    virtual void exec( const QVariant &params ) = 0;

    /*!
     * Cancels an ongoing operation upon receiving the cancelling 
     * request from end user.
     */
    virtual void cancel();
    
signals:

    /*!
     * Signal for informing the relevant party that Ui Edit mode
     * for the current operation should be activated.
     */
    void beginUiEditMode();
    
    /*!
     * Signal for informing the relevant party that Ui Edit mode
     * for the current operation should be deactivated.
     */
    void endUiEditMode();
    
    /*!
     * Signal for informing that this delegate has completed the operation.
     * \param error the result of the operation - 0 if the operation
     * was successful; some error code, otherwise.
     */
    void delegateCompleted(int error, BtAbstractDelegate *delegate);
    
protected:
    
    /*!
     * access to setting model.
     */
    BtSettingModel *settingModel();
    
    /*!
     * access to device model.
     */
    BtDeviceModel *deviceModel();
    
    /*!
     * Tells whether BT is on.
     * \return true if BT is on; false otherwise
     */
    bool isBtPowerOn();
    
    void completeDelegateExecution(int error);
    
    void setExecuting(bool executing);
    
public slots:

private:

    // pointer to models. do not own. 
    BtSettingModel *mSettingModel;
    
    BtDeviceModel *mDeviceModel;
    
    bool mExecuting;
    
    Q_DISABLE_COPY(BtAbstractDelegate)

};

Q_DECLARE_METATYPE(QModelIndex)

#endif // BTABSTRACTDELEGATE_H
