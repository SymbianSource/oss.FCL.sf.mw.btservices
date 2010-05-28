/*
* Copyright (c) 2002 - 2007 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Symbian Foundation License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.symbianfoundation.org/legal/sfl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  ?Description
*
*/



// [INCLUDE FILES] - do not remove
#include <e32svr.h>
#include <StifParser.h>
#include <Stiftestinterface.h>
#include "ObexServiceSendUtilsApiTest.h"
#include "BTServiceAPI.h"
// EXTERNAL DATA STRUCTURES
//extern  ?external_data;

// EXTERNAL FUNCTION PROTOTYPES  
//extern ?external_function( ?arg_type,?arg_type );

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def

// LOCAL CONSTANTS AND MACROS
//const ?type ?constant_var = ?constant;
//#define ?macro_name ?macro_def

// MODULE DATA STRUCTURES
//enum ?declaration
//typedef ?declaration

// LOCAL FUNCTION PROTOTYPES
//?type ?function_name( ?arg_type, ?arg_type );

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// ?function_name ?description.
// ?description
// Returns: ?value_1: ?description
//          ?value_n: ?description_line1
//                    ?description_line2
// -----------------------------------------------------------------------------
//
/*
?type ?function_name(
    ?arg_type arg,  // ?description
    ?arg_type arg)  // ?description
    {

    ?code  // ?comment

    // ?comment
    ?code
    }
*/

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CObexServiceSendUtilsApiTest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CObexServiceSendUtilsApiTest::Delete() 
    {

    }

// -----------------------------------------------------------------------------
// CObexServiceSendUtilsApiTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CObexServiceSendUtilsApiTest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function. 
        ENTRY( "StartService", CObexServiceSendUtilsApiTest::StartServiceL ),
        //ADD NEW ENTRY HERE
        // [test cases entries] - Do not remove

        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

TInt CObexServiceSendUtilsApiTest::StartServiceL( CStifItemParser& aItem )
    {

      CBTServiceAPI* service = CBTServiceAPI::NewL();
      CleanupStack::PushL(service );
      STIF_ASSERT_NOT_NULL(service)
      
      CBTServiceParameterList* list = CBTServiceParameterList::NewL();
      iLog->Log(_L("CBTServiceParameterList::NewL"));
      CleanupStack::PushL(list);
      STIF_ASSERT_NOT_NULL(list)

      list->AddImageL(_L("e:\\testdata\\SmallPic.jpg"));
      iLog->Log(_L("list->AddImageL "));
      CleanupStack::Pop(list);
      
      TRAPD( err, service->StartSynchronousServiceL( EBTSendingService, list));
      iLog->Log(_L("service->StartServiceL result: %d"), err);
      CleanupStack::PopAndDestroy(1);
    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CObexServiceSendUtilsApiTest::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
/*
TInt CObexServiceSendUtilsApiTest::?member_function(
   CItemParser& aItem )
   {

   ?code

   }
*/

// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove
