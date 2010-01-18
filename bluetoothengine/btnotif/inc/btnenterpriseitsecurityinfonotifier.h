/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares generic information notifier class.
*
*/

#ifndef BTNENTERPRISEITSECURITYINFONNOTIFIER_H
#define BTNENTERPRISEITSECURITYINFONNOTIFIER_H

// INCLUDES
#include "btnotifier.h" // Base class

// CLASS DECLARATION
NONSHARABLE_CLASS(CBTEnterpriseItSecurityInfoNotifier) : public CBTNotifierBase
    {
public:		// Constructors and destructor
	/**
	* Two-phased constructor.
	*/
	static CBTEnterpriseItSecurityInfoNotifier* NewL();

	/**
	* Destructor.
	*/
	virtual ~CBTEnterpriseItSecurityInfoNotifier();

private:	// Functions from base classes
	/**
	* From CBTNotifierBase Called when a notifier is first loaded 
	* to allow any initial construction that is required.
	* @param None.
	* @return A structure containing priority and channel info.
	*/
	TNotifierInfo RegisterL();
	
	/** From CBTNotifierBase Synchronic notifier launch.        
	* @param aBuffer Received parameter data.
	* @return A pointer to return value.
	*/
	TPtrC8 StartL(const TDesC8& aBuffer);

	/**
	* From CBTNotifierBase Used in asynchronous notifier launch to 
	* store received parameters into members variables and 
	* make needed initializations.
	* @param aBuffer A buffer containing received parameters
	* @param aReturnVal The return value to be passed back.
	* @param aMessage Should be completed when the notifier is deactivated.
	* @return None.
	*/
	void GetParamsL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage);

	/**
	* From CBTNotifierBase
	*/      
	void HandleGetDeviceCompletedL(const CBTDevice* aDev);        

private:
	/**
	* C++ default constructor.
	*/
	CBTEnterpriseItSecurityInfoNotifier();

	/**
	 * Show Information note and complete message. 
	 */
	void ShowNoteAndCompleteL();
    };

#endif	//BTNENTERPRISEITSECURITYINFONNOTIFIER_H

// End of File
