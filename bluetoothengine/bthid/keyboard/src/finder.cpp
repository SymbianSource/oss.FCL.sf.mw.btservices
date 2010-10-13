/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the implementation of application class
 *
*/


#include <e32std.h>
#include <e32svr.h>

#include "finder.h"

// ----------------------------------------------------------------------

#ifndef DBG
#ifdef _DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif
#endif

// ----------------------------------------------------------------------

TKeyboardFinder::TKeyboardFinder() :
    iStandardKeys(0), iModifierKeys(0), iLeds(0), iAppCollection(0)
    {
    // Nothing else to do
    }

// ----------------------------------------------------------------------

TBool TKeyboardFinder::BeginCollection(const CCollection* aCollection)
    {
    TBool examineCollection = ETrue;

    if ((aCollection->IsApplication()) && (iAppCollection == 0))
        {
        // Top-level application collection.

        if ((aCollection->UsagePage() == EUsagePageGenericDesktop)
                && (aCollection->Usage() == EGenericDesktopUsageKeyboard))
            {
            DBG(RDebug::Print(_L("[HID]\tTKeyboardFinder::BeginCollection: this is keyboard collection ")));
            // Collection is a keyboard device:
            iAppCollection = aCollection;
            iStandardKeys = iModifierKeys = iLeds = 0;
            }
        else
            {
            // (dont) Skip other types of top-level application collection:
            DBG(RDebug::Print(_L("[HID]\tTKeyboardFinder::BeginCollection: not keyboard collection ")));
            examineCollection = EFalse;
            }
        }

    return examineCollection;
    }

TBool TKeyboardFinder::EndCollection(const CCollection* aCollection)
    {
    TBool continueSearch = ETrue;

    if (aCollection == iAppCollection)
        {
        // Top-level application(Generic Desktop:Keyboard) finished:
        //
        iAppCollection = 0;

        // Stop if we've found a keyboard we can use in this
        // application collection:
        //
        continueSearch = !Found();
        }

    return continueSearch;
    }

void TKeyboardFinder::Field(const CField* aField)
    {
    if (iAppCollection)
        {
        if (IsStandardKeys(aField))
            {
            iStandardKeys = aField;
            }

        if (IsModifiers(aField))
            {
            iModifierKeys = aField;
            }

        if (IsLeds(aField))
            {
            iLeds = aField;
            }
        }
    }

// ----------------------------------------------------------------------

TBool TKeyboardFinder::IsModifiers(const CField* aField) const
    {
    TBool found = EFalse;

    if (aField->IsInput() && aField->IsData() && (aField->UsagePage()
            == EUsagePageKeyboard))
        {
        const TInt KKbdLeftControl = 224;
        const TInt KKbdRightGUI = 231;

        // Test for a field containing only modifier keys
        if ((aField->UsageMin() >= KKbdLeftControl) && (aField->UsageMax()
                <= KKbdRightGUI))
            {
            DBG(RDebug::Print(_L("[HID]\t   Modifier field found")));
            found = ETrue;
            }
        }

    return found;
    }

TBool TKeyboardFinder::IsStandardKeys(const CField* aField) const
    {
    TBool found = EFalse;

    if (aField->IsInput() && aField->IsData() && (aField->UsagePage()
            == EUsagePageKeyboard))
        {
        const TInt KKeyboardRollOver = 0x01;
        const TInt KKeyboardUpArrow = 0x52;

        if ((aField->UsageMin() <= KKeyboardRollOver) && (aField->UsageMax()
                >= KKeyboardUpArrow))
            {
            DBG(RDebug::Print(_L("[HID]\t   Standard keys field found")));
            found = ETrue;
            }
        }

    return found;
    }

TBool TKeyboardFinder::IsLeds(const CField* aField) const
    {
    TBool found = EFalse;

    if (aField->IsOutput() && aField->IsData() && (aField->UsagePage()
            == EUsagePageLEDs))
        {
        const TInt KNumLockLed = 1;
        const TInt KCapsLockLed = 2;

        // Test for a field containing at least num lock or caps lock
        if ((aField->UsageMin() <= KCapsLockLed) && (aField->UsageMax()
                >= KNumLockLed))
            {
            DBG(RDebug::Print(_L("[HID]\t   LED field found")));
            found = ETrue;
            }
        }

    return found;
    }

// ----------------------------------------------------------------------

TConsumerKeysFinder::TConsumerKeysFinder() :
    iField(0)
    {
    // Nothing else to do
    }

void TConsumerKeysFinder::Field(const CField* aField)
    {
    if (!Found() && aField->IsInput() && aField->IsData()
            && (aField->UsagePage() == EUsagePageConsumer))
        {
        DBG(RDebug::Print(_L("Consumer keys field found\r\n")));
        iField = aField;
        }
    }

TBool TConsumerKeysFinder::BeginCollection(const CCollection* aCollection)
    {
    const TInt KConsumerControl = 0x01;

    // Only look at top-level application (consumer devices: consumer
    // control) collections:
    //
    return aCollection->IsApplication() && (aCollection->UsagePage()
            == EUsagePageConsumer) && (aCollection->Usage()
            == KConsumerControl);
    }

TBool TConsumerKeysFinder::EndCollection(const CCollection* /*aCollection*/)
    {
    return !Found();
    }

// ----------------------------------------------------------------------

TPowerKeysFinder::TPowerKeysFinder() :
    iField(0)
    {
    // Nothing else to do
    }

void TPowerKeysFinder::Field(const CField* aField)
    {
    if (!Found() && aField->IsInput() && aField->IsData()
            && (aField->UsagePage() == EUsagePageGenericDesktop))
        {
        // See if the range includes one or more of the power down,
        // sleep or wakeup controls:

        const TInt KPowerDown = 0x81;
        const TInt KWakeUp = 0x83;

        if ((aField->UsageMin() <= KWakeUp) && (aField->UsageMax()
                >= KPowerDown))
            {
            DBG(RDebug::Print(_L("[HID]\t  Power keys field found\r\n")));
            iField = aField;
            }
        }
    }

TBool TPowerKeysFinder::BeginCollection(const CCollection* aCollection)
    {
    const TInt KSystemControl = 0x80;

    // Only look at top-level application (generic desktop: system
    // control) collections:
    //
    return aCollection->IsApplication() && (aCollection->UsagePage()
            == EUsagePageGenericDesktop) && (aCollection->Usage()
            == KSystemControl);
    }

TBool TPowerKeysFinder::EndCollection(const CCollection*)
    {
    return !Found();
    }
