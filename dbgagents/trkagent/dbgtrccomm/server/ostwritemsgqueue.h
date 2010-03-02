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
* Description: 
*
*/

#ifndef __OstWriteMsgQueue_h__
#define __OstWriteMsgQueue_h__

// System includes
#include <e32base.h>
#include "OstProtDefs.h"


class COstWriteMsgEntry : public CBase
{
	public:
		static 	COstWriteMsgEntry*	NewL(TDesC8& aMsg, CDbgTrcSrvSession* aMsgListener);
				~COstWriteMsgEntry();
	
	//private construct
	private:
				COstWriteMsgEntry();
		void	ConstructL(TDesC8& aMsg);

	public:	
		COstMessage* iMessage;
		CDbgTrcServSession* iWriteMsgListener;		
};

#endif //__OstWriteMsgQueue_h__   
