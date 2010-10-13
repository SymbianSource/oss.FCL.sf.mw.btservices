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

#include <e32base.h>

/**
This class is a utility class used by the BIP and OPP controllers to
launch the Messaging App's Bluetooth conversation view.
*/
class CBtConversationViewLauncher : public CBase
    {
public:
    IMPORT_C static CBtConversationViewLauncher* NewL();
    IMPORT_C ~CBtConversationViewLauncher();
    IMPORT_C void LaunchConversationViewL();
    
private:
    CBtConversationViewLauncher();
    };
