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

#ifndef __OstMessage_h__
#define __OstMessage_h__

// System includes
#include <e32base.h>
#include "ostprotdefs.h"


class COstMessage : public CBase
{
	public:
		static 	COstMessage*	NewL(TDesC8& aMsg);
				~COstMessage();
	
	//private construct
	private:
				COstMessage();
		void	ConstructL(TDesC8& aMsg);

	public:	
		HBufC8 *iMsgBuffer;
		TOstProtIds iProtId;
		TUint iLength;
};

#endif //__OstMessage_h__   
