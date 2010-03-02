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

#include "dummytraceserver.h"

_LIT8(KTRACEMSG, "Test trace server message: %d\n");

//
//
// CDummyTraceServer implementation
//
//

CDummyTraceServer*	CDummyTraceServer::NewLC(CConsoleIO* aConsoleIO)
{
	CDummyTraceServer* self = new(ELeave) CDummyTraceServer(aConsoleIO);
	CleanupStack::PushL(self);
	self->ConstructL();
	
	return self;
}

CDummyTraceServer::CDummyTraceServer(CConsoleIO* aConsoleIO)
	: CActive(EPriorityLow),
	  iConsoleIO(aConsoleIO),
	  iMessageId(0)
	  
{
	CActiveScheduler::Add(this);
}

void CDummyTraceServer::ConstructL()
{
	User::LeaveIfError(iTimer.CreateLocal());
}

CDummyTraceServer::~CDummyTraceServer()
{
	
}

void CDummyTraceServer::Start()
{
	
	TInt err = iAcmPort.Connect();
	if (err != KErrNone)
	{
		iConsoleIO->PrintToScreen(_L("Connect failed: %d"), err);
		return;
	}
	iConsoleIO->PrintToScreen(_L("Successfully the DbgTrc Comms server\n"));

	
	err = iAcmPort.Open();
	if (err != KErrNone)
	{
		iConsoleIO->PrintToScreen(_L("Opening port failed: %d\n"), err);
		return;
	}
	iConsoleIO->PrintToScreen(_L("Successfully opened the DbgTrc Comm Port\n"));

	
	TAcmConfig config;	
	err = iAcmPort.GetAcmConfig(config);
	if (err != KErrNone)
	{
		iConsoleIO->PrintToScreen(_L("Unable to get port configuration: %d\n"), err);
		return;
	}
	iConsoleIO->PrintToScreen(_L("Current ACM port number: %d\n"), config().iPortNumber);

	err = iAcmPort.RegisterProtocolID(EOstProtTraceCore, EFalse);
	if (err != KErrNone)
	{
		iConsoleIO->PrintToScreen(_L("Unable to register protocol: %d\n"), err);
		return;
	}
	iConsoleIO->PrintToScreen(_L("Registered the protocol id: %d\n"), EOstProtTraceCore);
		
	Activate();
	
	iConsoleIO->PrintToScreen(_L("Started sending dummy traces\n"));
}

void CDummyTraceServer::SendMessage()
{
	TBuf8<100> msg;
	TRequestStatus status;
		
	msg.Format(KTRACEMSG, iMessageId++);
	iAcmPort.WriteMessage(status, msg, EFalse);
	User::WaitForRequest(status);
}

void CDummyTraceServer::Activate()
{
	iTimer.After(iStatus, KPeriod);
	SetActive();		
}



void CDummyTraceServer::RunL()
{
	// now send the message
	SendMessage();
	// reactivate the timer
	Activate();
}

void CDummyTraceServer::DoCancel()
{
	iTimer.Cancel();

    Cancel();
    iTimer.Close();
    Deque();

    TInt err = iAcmPort.UnRegisterProtocolID(EOstProtTraceCore);
    if (err != KErrNone)
    {
        iConsoleIO->PrintToScreen(_L("Unable to unregister protocol: %d\n"), err);
        return;
    }
    iConsoleIO->PrintToScreen(_L("Successfuly unregistered the protocol id: %d\n"), EOstProtTraceCore);

    
    err = iAcmPort.Close();
    if (err != KErrNone)
    {
        iConsoleIO->PrintToScreen(_L("Unable to close the port: %d\n"), err);
        return;
    }
    iConsoleIO->PrintToScreen(_L("Successfuly closed the port\n"));

}

