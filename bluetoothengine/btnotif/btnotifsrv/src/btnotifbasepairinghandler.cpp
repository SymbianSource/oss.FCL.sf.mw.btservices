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

#include "btnotifpairingmanager.h"
#include "btnotifbasepairinghandler.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifBasePairingHandler::CBTNotifBasePairingHandler( CBTNotifPairingManager& aParent, const TBTDevAddr& aAddr)
    : iAddr( aAddr ), iParent( aParent )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::BaseConstructL( )
    {
    iActive = CBtSimpleActive::NewL( *this, 0 );  
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifBasePairingHandler::~CBTNotifBasePairingHandler()
    {
    delete iActive;
    }

// ---------------------------------------------------------------------------
// Message passes through only if the result is for the same device this 
// object is for.
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::HandlePairServerResult( const TBTDevAddr& aAddr, TInt aResult )
    {
    if ( aAddr == iAddr )
        {
         DoHandlePairServerResult( aResult );
        }
    }

// ---------------------------------------------------------------------------
// Message passes through only if the result is for the same device this 
// object is for.
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::HandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev )
    {
    if ( aDev.Address() == iAddr )
        {
        DoHandleRegistryNewPairedEvent( aDev );
        }
    }

// ---------------------------------------------------------------------------
// Default impl of virtual function. do nothing
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::CancelOutgoingPair()
    {
    }

// ---------------------------------------------------------------------------
// Default impl does not offer a known PIN code for pairing 
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::GetPinCode(
        TBTPinCode& aPin, const TBTDevAddr& aAddr, TInt aMinPinLength )
    {
    aPin().iLength = 0;
    (void) aAddr;
    (void) aMinPinLength;
    }

// ---------------------------------------------------------------------------
// Invalidate iPairResultSet
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::UnSetPairResult()
    {
    iPairResultSet = EFalse;
    }

// ---------------------------------------------------------------------------
// Save the result and validate the flag
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::SetPairResult( TInt aResult )
    {
    iPairResult = aResult;
    iPairResultSet = ETrue;
    }

// ---------------------------------------------------------------------------
// Returns the flag
// ---------------------------------------------------------------------------
//
TBool CBTNotifBasePairingHandler::IsPairResultSet()
    {
    return iPairResultSet;
    }




