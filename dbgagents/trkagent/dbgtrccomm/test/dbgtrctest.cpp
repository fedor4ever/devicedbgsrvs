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



#include <e32base.h>
#include <e32cons.h>

#include "dbgtrccomm.h"
#include "dummytraceserver.h"
#include "dbgtrctest.h"

LOCAL_C CConsoleIO* consoleIO = NULL;
#define SafeDelete(x) { if (x) delete x; x = NULL; }



CKeyboardInput::CKeyboardInput(CConsoleBase* aConsole)
	: CActive(EPriorityUserInput),
	  iConsole(aConsole)
{
	CActiveScheduler::Add(this);
}

CKeyboardInput::~CKeyboardInput()
{
	Cancel();
	Deque();
}


void CKeyboardInput::ListenToKeyboard()
{
	iConsole->Read(iStatus);
	SetActive();
}

void CKeyboardInput::RunL()
{
	TKeyCode key = iConsole->KeyCode();
	if (key == 'q' || key == 'Q')
		CActiveScheduler::Stop();
	else
		ListenToKeyboard();
}

void CKeyboardInput::DoCancel()
{
	iConsole->ReadCancel();
}

CConsoleIO::CConsoleIO()
	: iConsole(0),
	  iKeyboardInput(0)
{
	TRAPD(error, iConsole = Console::NewL(_L("DBGTRCTEST"), TSize(KConsFullScreen, KConsFullScreen)));
	if (error != KErrNone)
		User::Panic(_L("DBGTRCTEST failed to allocate CConsoleBase"), __LINE__);

	iKeyboardInput = new CKeyboardInput(iConsole);
	if (iKeyboardInput == 0)
		User::Panic(_L("DBGTRCTEST failed to allocate CKeyboardInput"), __LINE__);

	iKeyboardInput->ListenToKeyboard();
}

CConsoleIO::~CConsoleIO()
{
	SafeDelete(iKeyboardInput);
	SafeDelete(iConsole);
}

//
// CConsoleIO::PrintToScreen
//
//	Print a message to the screen
//
void CConsoleIO::PrintToScreen(TRefByValue<const TDesC> aFmt, ...)
{
	VA_LIST(l);
	VA_START(l, aFmt);
	iBuffer.FormatList(aFmt, l);

	iConsole->Printf(iBuffer);
}

//
// CConsoleIO::Terminating
//
//	Notify user of termination
//
void CConsoleIO::Terminating()
{
	iConsole->Printf(_L("\r\nPress any key to exit."));
	iKeyboardInput->Cancel();
	iConsole->Getch();
}



LOCAL_C void runBasicTests()
{	
	RDbgTrcComm acmPort;
	
	TInt err = acmPort.Connect();
	if (err != KErrNone)
	{
		consoleIO->PrintToScreen(_L("Connect failed: %d"), err);
		return;
	}
	consoleIO->PrintToScreen(_L("Successfully the DbgTrc Comms server\n"));

	
	err = acmPort.Open();
	if (err != KErrNone)
	{
		consoleIO->PrintToScreen(_L("Opening port failed: %d\n"), err);
		return;
	}
	consoleIO->PrintToScreen(_L("Successfully opened the DbgTrc Comm Port\n"));

	
	TAcmConfig config;	
	err = acmPort.GetAcmConfig(config);
	if (err != KErrNone)
	{
		consoleIO->PrintToScreen(_L("Unable to get port configuration: %d\n"), err);
		return;
	}
	consoleIO->PrintToScreen(_L("Current ACM port number: %d\n"), config().iPortNumber);

	err = acmPort.RegisterProtocolID(EOstProtTrk, EFalse);
	if (err != KErrNone)
	{
		consoleIO->PrintToScreen(_L("Unable to register protocol: %d\n"), err);
		return;
	}
	consoleIO->PrintToScreen(_L("Registered the protocol id: %d\n"), EOstProtTraceCore);
	
	TAcmConfigV01 config1(*(TAcmConfigV01*)config.Ptr());
	
	TBuf8<100> msg;
	_LIT8(KTRACEMSG, "Test trace server message: %d\n");
	
	TRequestStatus status;
	
	for (TInt i=0; i<1000; i++)
	{
		msg.Format(KTRACEMSG, i);
		acmPort.WriteMessage(status, msg, ETrue);
		User::WaitForRequest(status);
		
		User::After(100);
	}
	
	if (status.Int() != KErrNone)
	{
		consoleIO->PrintToScreen(_L("Unable to write the message: %d\n"), status.Int());
		return;	
	}
	consoleIO->PrintToScreen(_L("Successfuly sent the message\n"));

	err = acmPort.UnRegisterProtocolID(EOstProtTraceCore);
	if (err != KErrNone)
	{
		consoleIO->PrintToScreen(_L("Unable to unregister protocol: %d\n"), err);
		return;
	}
	consoleIO->PrintToScreen(_L("Successfuly unregistered the protocol id: %d\n"), EOstProtTraceCore);

	
	err = acmPort.Close();
	if (err != KErrNone)
	{
		consoleIO->PrintToScreen(_L("Unable to close the port: %d\n"), err);
		return;
	}
	consoleIO->PrintToScreen(_L("Successfuly closed the port\n"));
}


void MainL()
{
	//RProcess().SetPriority(EPriorityHigh);
	//RThread().SetPriority(EPriorityAbsoluteHigh);
	
	consoleIO->PrintToScreen(_L("DBGTRCTEST App\r\n"));
	
	//runBasicTests();	
	
	CDummyTraceServer* traceServer = CDummyTraceServer::NewLC(consoleIO);
	traceServer->Start();
	consoleIO->PrintToScreen(_L("Press q or Q to quit\n"));

	CActiveScheduler::Start();

	CleanupStack::PopAndDestroy(traceServer);
}

//
// EXE entrypoint
//
TInt E32Main()
{	
	__UHEAP_MARK;

	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	if (cleanupStack == 0)
		User::Panic(_L("DBGTRCTEST failed to allocate CTrapCleanup"), __LINE__);
	
	CActiveScheduler* scheduler = new CActiveScheduler;
	if (scheduler == 0)
		User::Panic(_L("DBGTRCTEST failed to allocate CActiveScheduler"), __LINE__);

	CActiveScheduler::Install(scheduler);

	consoleIO = new CConsoleIO;
	
	if (!consoleIO)
		User::Panic(_L("DBGTRCTEST failed to allocate CTrkConsoleIO"), __LINE__);
	
	TRAPD(error, MainL());

	if (error != KErrNone)
	{
		consoleIO->PrintToScreen(_L("\r\nUnexpected error %d"), error);
		consoleIO->Terminating();
	}
	
	SafeDelete(consoleIO);	
	SafeDelete(scheduler);
	SafeDelete(cleanupStack);

	__UHEAP_MARKEND;

	User::Exit(0);
	return 0;
}
