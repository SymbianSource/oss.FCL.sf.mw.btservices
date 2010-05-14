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

#ifndef BTQTCONSTANTS_H
#define BTQTCONSTANTS_H

#include <btengconstants.h>


enum VisibilityMode {
    BtHidden = 0x10,  // using a different number space than TBTVisibilityMode
    BtVisible,
    BtTemporary,
    BtUnknown
    
};

// used for mapping between UI row and VisibilityMode item
enum VisibilityModeUiRowMapping {
    UiRowBtHidden = 0,
    UiRowBtVisible,
    UiRowBtTemporary
};

inline VisibilityMode QtVisibilityMode(TBTVisibilityMode btEngMode)
{
    VisibilityMode mode; 
    switch(btEngMode) {
    case EBTVisibilityModeHidden:
        mode = BtHidden;
        break;
    case EBTVisibilityModeGeneral:
        mode = BtVisible;
        break;
    case EBTVisibilityModeTemporary:
        mode = BtTemporary;
        break;
    default:
        mode = BtUnknown;
    }
    return mode;
}

inline TBTVisibilityMode  BtEngVisibilityMode(VisibilityMode btQtMode)
{
    TBTVisibilityMode mode; 
    switch(btQtMode) {
    case BtHidden:
        mode = EBTVisibilityModeHidden;
        break;
    case BtVisible:
        mode = EBTVisibilityModeGeneral;
        break;
    case BtTemporary:
        mode = EBTVisibilityModeTemporary;
        break;
    default:
        mode = (TBTVisibilityMode)KErrUnknown;
    }
    return mode;
}


#endif // BTQTCONSTANTS_H
