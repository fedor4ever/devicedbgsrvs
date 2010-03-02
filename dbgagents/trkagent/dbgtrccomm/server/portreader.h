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

#ifndef __PortReader_h__
#define __PortReader_h__

// System includes
#include <e32base.h>
#include <c32comm.h>

#include "datalistener.h"


class CPortReader : public CActive
{

	public:
		static 	CPortReader*	NewL(RComm& aPort);
				~CPortReader();
	
	//private construct
	private:
				CPortReader(RComm& aPort);
		void	ConstructL();
		void 	IssueReadRequest();

	public:
		void StartRead(MDataListener* aListener);
		void RunL();
		void DoCancel();

	// data members		
	private:
		RComm iPort;
				
		TBuf8<MAX_BUF_SIZE> iReceivedChars;
		TInt iNextReadChar;
		
		TBool iAlreadyReading;
		
		MDataListener* iListener;
		

};

#endif //__PortMgr_h__   
