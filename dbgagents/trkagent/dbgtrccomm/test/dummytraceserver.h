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
#ifndef __DBGTRCTESTTIMER_H__
#define __DBGTRCTESTTIMER_H__

#include <e32base.h>
#include "dbgtrctest.h"
#include "dbgtrccomm.h"
//
// class CDummyTraceServer
//
// Active object used to avoid auto-switch off
//
class CDummyTraceServer: public CActive
{
public:
	static CDummyTraceServer*	NewLC(CConsoleIO* aConsoleIO);	
	void ConstructL();
	~CDummyTraceServer();
	void Start();

protected:
	void RunL();
	void DoCancel();
	
private:
	CDummyTraceServer(CConsoleIO* aConsoleIO);
	void Activate();
	void SendMessage();

private:

	enum { KPeriod = 5 * 10 };
	
	CConsoleIO* iConsoleIO;

	RTimer iTimer;	
	RDbgTrcComm iAcmPort;
	
	TUint iMessageId;
};

#endif //__DBGTRCTESTTIMER_H__
