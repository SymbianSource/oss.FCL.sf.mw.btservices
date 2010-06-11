/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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


// INCLUDE FILES
#include "BTSProgresstimer.h"
#include "BTServiceStarter.h"
#include "BTSUDebug.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSProgressTimer::CBTSProgressTimer( MBTServiceObserver* aProgressObserverPtr)
                               : CTimer( EPriorityLow ), 
                                       iProgressObserverPtr( aProgressObserverPtr )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::ConstructL
// Symbian OS default constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSProgressTimer::ConstructL()
    {
    FLOG(_L("[OBEXUTILS]\t CBTSProgressTimer::ConstructL()"));

    CTimer::ConstructL();

    FLOG(_L("[OBEXUTILS]\t CBTSProgressTimer::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::NewL
// -----------------------------------------------------------------------------
 CBTSProgressTimer* CBTSProgressTimer::NewL( MBTServiceObserver* aProgressObserverPtr)
    {
    CBTSProgressTimer* self = 
        new( ELeave ) CBTSProgressTimer( aProgressObserverPtr );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CBTSProgressTimer::~CBTSProgressTimer()
    {
    Cancel();
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::Tickle
// -----------------------------------------------------------------------------
//
 void CBTSProgressTimer::Tickle()
    {
    FLOG(_L("[OBEXUTILS]\t CBTSProgressTimer::Tickle()"));

    Cancel();
    After( iTimeout );

    FLOG(_L("[OBEXUTILS]\t CBTSProgressTimer::Tickle() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::RunL
// -----------------------------------------------------------------------------
//
void CBTSProgressTimer::RunL()
    {
    FLOG(_L("[OBEXUTILS]\t CBTSProgressTimer::RunL()"));
    if (iProgressObserverPtr)
        {
        iProgressObserverPtr->UpdateProgressInfoL();
        }

    FLOG(_L("[OBEXUTILS]\t CBTSProgressTimer::RunL() completed"));
    }

TInt CBTSProgressTimer::RunError( TInt aError )
    {
    FLOG(_L("[OBEXUTILS]\t CBTSProgressTimer::RunError()"));
    (void) aError;
    FLOG(_L("[OBEXUTILS]\t CBTSProgressTimer::RunError() - completed"));
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::SetTimeout
// -----------------------------------------------------------------------------
//
 void CBTSProgressTimer::SetTimeout( TTimeIntervalMicroSeconds32 aTimeout )
    {
    iTimeout = aTimeout;
    }

//  End of File  
