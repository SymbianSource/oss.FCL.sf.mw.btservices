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


#ifndef BTSPROGRESSTIMER_H
#define BTSPROGRESSTIMER_H


#include    <e32base.h>

class MBTServiceObserver;

// CLASS DECLARATION
/**
*  A timer class for updating progress dialog.
*/
NONSHARABLE_CLASS( CBTSProgressTimer ) : public CTimer
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */                           
        static CBTSProgressTimer* NewL( MBTServiceObserver* aProgressObserverPtr );
        
        /**
        * Destructor.
        */
        virtual ~CBTSProgressTimer();
     
    public: // New functions

        /**
        * Sets the timeout of the timer.
        * @param aTimeout The timeout in microseconds.
        * @return None.
        */
        void SetTimeout( TTimeIntervalMicroSeconds32 aTimeout );

        /**
        * Restarts the timer.
        * @return None.
        */
        void Tickle();

    private: // Functions from base classes

        /**
        * From CTimer Get's called when the timer expires.        
        * @return None.
        */
        void RunL();
        
        TInt RunError( TInt aError );

    private:

        /**
        * C++ default constructor.
        */
        CBTSProgressTimer( MBTServiceObserver* aProgressObserverPtr );
        
        /**
        * By default Symbian OS constructor is private.
        */      
        void ConstructL();

    private: // Data
        TTimeIntervalMicroSeconds32 iTimeout;
        MBTServiceObserver* iProgressObserverPtr;
    };

#endif      // BTSPROGRESSTIMER_H
            
// End of File
