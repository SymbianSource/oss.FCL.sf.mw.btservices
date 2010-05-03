/*
* ============================================================================
*  Name        : bluetoothnotification.cpp
*  Part of     : bluetoothengine / btnotif
*  Description : Class for managing an actual user notification or query.
*                It hides UI framework-specifics in a private class.
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "bluetoothnotification.h"
#include <hb/hbcore/hbsymbianvariant.h>
#include "btnotificationmanager.h"
#include "btnotifserver.h"
#include "bluetoothtrace.h"
#include <utf.h>  // for debugging

/**  Identifier of Bluetooth device dialog plug-in. */
_LIT( KBTDevDialogId, "com.nokia.hb.btdevicedialog/1.0" );
/**  Key name of result. */
_LIT( KBTDevDialogResult, "result" );
_LIT( KBTDevDialogInput, "input" );

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBluetoothNotification::CBluetoothNotification( CBTNotificationManager* aManager )
:   iManager( aManager )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBluetoothNotification::ConstructL()
    {
    iNotificationData = CHbSymbianVariantMap::NewL();
    iDialog = CHbDeviceDialogSymbian::NewL();
    }


// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBluetoothNotification* CBluetoothNotification::NewL( 
    CBTNotificationManager* aManager )
    {
	BOstraceFunctionEntry0( DUMMY_DEVLIST );
    CBluetoothNotification* self = new( ELeave ) CBluetoothNotification( aManager );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
	BOstraceFunctionExit0( DUMMY_DEVLIST );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBluetoothNotification::~CBluetoothNotification()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    delete iDialog;
    delete iNotificationData;
    delete iReturnData;
	BOstraceFunctionExit1( DUMMY_DEVLIST, this )
}


// ---------------------------------------------------------------------------
// Resets the notification, clean all the internals.
// ---------------------------------------------------------------------------
//
void CBluetoothNotification::Reset()
    {
	BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    iType = TBluetoothDialogParams::EInvalidDialog;
    iResourceId = ENoResource;
    iObserver = NULL;
    iDialog->Cancel();
    iDialog->SetObserver( NULL );   // Not interested in a callback anymore.
    delete iNotificationData;
    iNotificationData = NULL;
    iNotificationData = CHbSymbianVariantMap::NewL();
    delete iReturnData;
    iReturnData = NULL;
    iReturnData = CHbSymbianVariantMap::NewL();
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Sets the data to be shown to the user.
// ---------------------------------------------------------------------------
//
TInt CBluetoothNotification::SetData( TInt aDataType, const TDesC& aData )
    {
    TRAPD( err, SetDataL( aDataType, aData ) );
    return (int) err;
    }


// ---------------------------------------------------------------------------
// Sets the data to be shown to the user.
// ---------------------------------------------------------------------------
//
TInt CBluetoothNotification::SetData( TInt aDataType, TInt aData )
    {
    TRAPD( err, SetDataL( aDataType, aData ) );
    return (int) err;
    }

// ---------------------------------------------------------------------------
// Updates the data to be shown to the user.
// ---------------------------------------------------------------------------
//
TInt CBluetoothNotification::Update( const TDesC& aData )
    {
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    (void) aData;
    int ret = iDialog->Update( *iNotificationData );
    delete iNotificationData;
    iNotificationData = NULL;
    iNotificationData = CHbSymbianVariantMap::NewL();
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    return ret;
    }


// ---------------------------------------------------------------------------
// Show the notification, which means that it is added to the queue.
// ---------------------------------------------------------------------------
//
TInt CBluetoothNotification::Show()
    {
	BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    TRAPD( err, SetDataL( TBluetoothDialogParams::EDialogType, iType ) );
    if( !err )
        {
        TRAP( err, SetDataL( TBluetoothDialogParams::EResource, iResourceId ) );
        }
    delete iReturnData;
    iReturnData = NULL;
    if( !err )
        {
        TRAP( err, iReturnData = CHbSymbianVariantMap::NewL() );
        }
    if( !err )
        {
        err = iDialog->Show( KBTDevDialogId(), *iNotificationData, this );
        }
    delete iNotificationData;
    iNotificationData = NULL;
    iNotificationData = CHbSymbianVariantMap::NewL();
    
    const TInt KPluginErr = CHbDeviceDialogSymbian::EPluginErrors + 1;
    if( err == KPluginErr )
        {
        err = KErrNotFound;
        }
	BOstraceFunctionExitExt( DUMMY_DEVLIST, this, err );
    return err;
    }


// ---------------------------------------------------------------------------
// Stop showing the notification.
// ---------------------------------------------------------------------------
//
TInt CBluetoothNotification::Close()
    {
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    iDialog->Cancel();
    iManager->ReleaseNotification( this );
	BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// Sets the data to be shown to the user.
// ---------------------------------------------------------------------------
//
void CBluetoothNotification::SetDataL( TInt aType, const TDesC& aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aType );
    TBuf16<6> key;
    TInt err = 0;
    CHbSymbianVariant* value = NULL;
    switch( aType )
        {
        case TBluetoothDialogParams::EAddress:
        case TBluetoothDeviceDialog::EDeviceName:
        case TBluetoothDeviceDialog::EAdditionalDesc:
        case TBluetoothDialogParams::EDialogTitle:
            key.Num(aType);
            value = CHbSymbianVariant::NewL( (TAny*) &aData, CHbSymbianVariant::EDes );
			BtTraceBlock( 
                    TBuf<32> buf;
                    switch (aType) {
                        case TBluetoothDialogParams::EAddress:
                            _LIT(KAddress,"EAddress");
                            buf.Append(KAddress); 
                            break;
                        case TBluetoothDeviceDialog::EDeviceName:
                            _LIT(KDeviceName,"EDeviceName");
                            buf.Append(KDeviceName); 
                            break;
                        case TBluetoothDeviceDialog::EAdditionalDesc:
                            _LIT(KAdditionalDesc,"EAdditionalDesc");
                            buf.Append(KAdditionalDesc); 
                            break;
                    }
                    TPtrC p(buf);
                    TPtrC16 *ptr = (TPtrC16 *)value->Data();
                    BOstraceExt2( TRACE_DEBUG, DUMMY_DEVLIST, "SetData [%S] = [%S]", &p, ptr);
                    );
            err = iNotificationData->Add( key, value );   // Takes ownership of value
            if ( err )
                {
                // Note: need a proper exception handling. 
                // NOTIF_NOTHANDLED( err )
                }
            break;
        case TBluetoothDialogParams::EResource:
        case TBluetoothDeviceDialog::EDeviceClass:
        case TBluetoothDeviceDialog::EAdditionalInt:
            PanicServer( EBTNotifPanicBadArgument );
            break;
        case TBluetoothDialogParams::ENoParams:
        case TBluetoothDeviceDialog::ENoParams:
        default:
            break;
        }
		BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }


// ---------------------------------------------------------------------------
// Sets the data to be shown to the user.
// ---------------------------------------------------------------------------
//
void CBluetoothNotification::SetDataL( TInt aType, TInt aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aType );
    TBuf<6> key;
    TInt err = 0;
    CHbSymbianVariant* value = NULL;
    switch( aType )
        {
        case TBluetoothDialogParams::EDialogType:
        case TBluetoothDialogParams::EResource:
        case TBluetoothDialogParams::EDialogTitle:
        case TBluetoothDeviceDialog::EDeviceClass:
        case TBluetoothDeviceDialog::EAdditionalInt:
            key.Num(aType);
            value = CHbSymbianVariant::NewL( (TAny*) &aData, CHbSymbianVariant::EInt );
			BtTraceBlock( 
                    TBuf<32> buf;
                    switch (aType) {
                        case TBluetoothDialogParams::EDialogType:
                            buf = _L("EDialogType"); 
                            break;
                        case TBluetoothDialogParams::EResource:
                            buf = _L("EResource");
                            break;
                        case TBluetoothDeviceDialog::EDeviceClass:
                            buf = _L("EDeviceClass");
                            break;
                        case TBluetoothDeviceDialog::EAdditionalInt:
                            buf = _L("EAdditionalInt");
                            break;
                    }
                    TPtrC p(buf);
                    TInt *intPtr = (TInt *)value->Data();
                    BOstraceExt2( TRACE_DEBUG, DUMMY_DEVLIST, "SetData [%S] = [%d]", &p, *intPtr);
                    );
			err = iNotificationData->Add( key, value );   // Takes ownership of value
	         if ( err )
	             {
                 // need a proper exception handling.
                 //NOTIF_NOTHANDLED( !err )
	             }
            
            break;
        case TBluetoothDialogParams::EAddress:
        case TBluetoothDeviceDialog::EDeviceName:
            PanicServer( EBTNotifPanicBadArgument );
            break;
        case TBluetoothDialogParams::ENoParams:
        case TBluetoothDeviceDialog::ENoParams:
        default:
            break;
        }
		BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// From class MHbDeviceDialogObserver.
// Callback called when data is received from a device dialog.
// ---------------------------------------------------------------------------
//
void CBluetoothNotification::DataReceived( CHbSymbianVariantMap& aData )
    {
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    BtTraceBlock( debugHbSymbianVariantMap(aData); );
    for( TInt i = 0; i < aData.Keys().MdcaCount(); i++ )
        {
        TPtrC key( aData.Keys().MdcaPoint( i ).Ptr(), aData.Keys().MdcaPoint( i ).Length() );
        const CHbSymbianVariant* valueRef = aData.Get( key );
        CHbSymbianVariant* value = CHbSymbianVariant::NewL( valueRef->Data(), valueRef->Type() );
        TInt err = iReturnData->Add( key, value );
        NOTIF_NOTHANDLED( !err )
        }
    iObserver->MBRDataReceived( aData );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

#ifdef BLUETOOTHTRACE_ENABLED

void CBluetoothNotification::debugHbSymbianVariantMap( CHbSymbianVariantMap& aData)
    {
    for( TInt i = 0; i < aData.Keys().MdcaCount(); i++ )
        {
        TBuf<128> buf;
        TPtrC key( aData.Keys().MdcaPoint( i ).Ptr(), aData.Keys().MdcaPoint( i ).Length() );
        buf = key;
        buf.Append(_L(" = "));
        const CHbSymbianVariant* value = aData.Get( key );
        TBuf<16> nbr;
        TBuf<32> newBuf;
        switch (value->Type()) {
            case CHbSymbianVariant::EInt :
                buf.Append(_L("[EInt] ")); 
                nbr.Num(*((TInt*)value->Data()));
                buf.Append(nbr);
                break;
            case  CHbSymbianVariant::EBool :
                buf.Append(_L("[EBool] ")); 
                buf.Append(*((TBool*)value->Data()) ? _L("True") : _L("False"));
                break;
            case CHbSymbianVariant::EUint :
                buf.Append( _L("[EUint] ")); 
                nbr.Num(*((TUint*)value->Data()));
                buf.Append(nbr);
                break;
            case CHbSymbianVariant::EReal  :
                buf.Append(_L("[EReal] ")); 
                nbr.Num(*((TReal*)value->Data()));
                buf.Append(nbr);
                break;
            case CHbSymbianVariant::EDes :  // TDesC
                buf.Append(_L("[EDes] ")); 
                buf.Append(*((TPtrC16 *)value->Data()));
                break;
            case CHbSymbianVariant::EBinary :  // TDesC8
                buf.Append(_L("[EBinary] ")); 
                // the following function caused problems when converting this function to 
                // a trace function in bluetoothtrace.h
                CnvUtfConverter::ConvertToUnicodeFromUtf8(newBuf,*((TPtrC8 *)value->Data()) );
                buf.Append(newBuf);
                break;
            case CHbSymbianVariant::EChar  : // a TChar
                buf.Append(_L("[EChar] ")); 
                buf.Append(*((TChar *)value->Data()));
                break;
            case CHbSymbianVariant::ERect  : // a TRect
            case CHbSymbianVariant::EPoint : // a TPoint
            case CHbSymbianVariant::ESize  : // a TSize
            case CHbSymbianVariant::EDesArray : //  a MDesCArray
                break;
            default:
                break;
            }
        TPtrC p(buf);
        BOstraceExt1( TRACE_DEBUG, DUMMY_DEVLIST, "HbSymbianVariantMap [%S]", &p);
        }
    }
#endif // BLUETOOTHTRACE_ENABLED
// ---------------------------------------------------------------------------
// From class MHbDeviceDialogObserver.
// Callback called when a device dialog is closed.
// ---------------------------------------------------------------------------
//
void CBluetoothNotification::DeviceDialogClosed( TInt aCompletionCode )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aCompletionCode );
    TPckg<TBool> result( EFalse );
    TPtrC8 resultPtr( result );
    const CHbSymbianVariant* value = iReturnData->Get( KBTDevDialogResult );
    if( value && value->IsValid() )
        {
        result() = *value->Value<TBool>();
        }
    RBuf8 resultBuf;
    value = iReturnData->Get( KBTDevDialogInput );
    if( value && value->IsValid() )
        {
        HBufC8* data = value->Value<HBufC8>();
        if( !resultBuf.Create( data->Length() + result.Length() ) )
            {
            resultBuf = *data;
            resultBuf.Insert( 0, result );
            resultPtr.Set( resultBuf );
            }
        }
    if( iObserver )
        {
        if( aCompletionCode == CHbDeviceDialogSymbian::ECancelledError )
            {
            aCompletionCode = KErrCancel;
            }
        iObserver->MBRNotificationClosed( aCompletionCode, resultPtr );
        }
    iManager->ReleaseNotification( this );
    // Note that we might get deleted after releasing ourselves.
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

