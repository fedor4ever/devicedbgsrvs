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

#ifndef __PortWriter_h__
#define __PortWriter_h__

// System includes
#include <e32base.h>
#include <c32comm.h>

#include "datalistener.h"


class CPortWriter : public CActive
{

	public:
		static 	CPortWriter*	NewL(RComm& aPort);
				~CPortWriter();
	
	//private construct
	private:
				CPortWriter(RComm& aPort);
		void	ConstructL();

	public:
		void StartWrite(TDesC8& aMsg, MDataListener* aListener);
		void RunL();
		void DoCancel();
		TBool IsBusy() { return iAlreadyWriting; }

	// data members		
	private:	
		RComm iPort;
		MDataListener* iListener;

		TBool iAlreadyWriting;


};

#endif //__PortWriter_h__   
