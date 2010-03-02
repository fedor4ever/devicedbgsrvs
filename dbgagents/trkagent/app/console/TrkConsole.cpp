/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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


#include "TrkConsole.h"
#include  "trksrvclisession.h"


#define SHOW_CONSOLE

#ifdef SHOW_CONSOLE
static CTrkConsoleIO* TrkIO = NULL;
#endif

#define SafeDelete(x) { if (x) delete x; x = NULL; }




//
//
// CKeyboardInput implementation
//
//

//
// CKeyboardInput constructor
//
CKeyboardInput::CKeyboardInput(CConsoleBase* aConsole)
	: CActive(EPriorityUserInput),
	  iConsole(aConsole)
{
	CActiveScheduler::Add(this);
}

//
// CKeyboardInput destructor
//
CKeyboardInput::~CKeyboardInput()
{
	Cancel();
	Deque();
}

//
// CKeyboardInput::ListenToKeyboard
//
// Wait for a key to be pressed
//
void CKeyboardInput::ListenToKeyboard()
{
	iConsole->Read(iStatus);
	SetActive();
}

//
// CKeyboardInput::RunL
//
// Called when a key is pressed
//
void CKeyboardInput::RunL()
{
	TKeyCode key = iConsole->KeyCode();
	if (key == 'q' || key == 'Q')
		CActiveScheduler::Stop();
	else
		ListenToKeyboard();
}

//
// CKeyboardInput::DoCancel
//
// Stop waiting for keyboard input
//
void CKeyboardInput::DoCancel()
{
	iConsole->ReadCancel();
}


//
//
// CTrkConsoleIO implementation
//
//

//
// CTrkConsoleIO constructor
//
CTrkConsoleIO::CTrkConsoleIO()
	: iConsole(0),
	  iKeyboardInput(0)
{
	TRAPD(error, iConsole = Console::NewL(_L("TRK"), TSize(KConsFullScreen, KConsFullScreen)));
	if (error != KErrNone)
		User::Panic(_L("TRK failed to allocate CConsoleBase"), __LINE__);

	iKeyboardInput = new CKeyboardInput(iConsole);
	if (iKeyboardInput == 0)
		User::Panic(_L("TRK failed to allocate CKeyboardInput"), __LINE__);

	iKeyboardInput->ListenToKeyboard();
}

//
// CTrkConsoleIO destructor
//
CTrkConsoleIO::~CTrkConsoleIO()
{
	SafeDelete(iKeyboardInput);
	SafeDelete(iConsole);
}

//
// CTrkConsoleIO::PrintToScreen
//
//	Print a message to the screen
//
void CTrkConsoleIO::PrintToScreen(TRefByValue<const TDesC> aFmt, ...)
{
	VA_LIST(l);
	VA_START(l, aFmt);
	iBuffer.FormatList(aFmt, l);

	iConsole->Printf(iBuffer);
}

//
// CTrkConsoleIO::Terminating
//
//	Notify user of termination
//
void CTrkConsoleIO::Terminating()
{
	iConsole->Printf(_L("\r\nPress any key to exit."));
	iKeyboardInput->Cancel();
	iConsole->Getch();
}
//
// MainL
//
// Create the engine and the communications interface and start the active scheduler
//
void MainL()
{
	RProcess().SetPriority(EPriorityHigh);
	RThread().SetPriority(EPriorityAbsoluteHigh);
	
	RTrkSrvCliSession trkSession;
	TTrkConnStatus iConnStatus;
	TBuf<KMaxPath> buf;
	TInt majorVersion, minorVersion, majorAPIVersion, minorAPIVersion, buildNumber;
	
	TInt err = trkSession.ConnectToServer();
	if (!err)
	{
	    // Ideally we need to connect here, but when the TRK server is started, connect automatically happens.
	    // So we don't need to explicitly connect here. If the server is changed not to connect automatically at startup
	    // then we need to connect here.	 
	    //err = trkSession.Connect();	        
        if(!err)
        {
            trkSession.GetDebugConnStatus(iConnStatus, buf);
            trkSession.GetTrkVersion(majorVersion, minorVersion, majorAPIVersion, minorAPIVersion, buildNumber);
        }
	 }

#ifdef SHOW_CONSOLE		
	TrkIO->PrintToScreen(_L("Welcome to TRK for Symbian OS\r\n"));
	TrkIO->PrintToScreen(_L("Version %d.%d.%d\r\n"), majorVersion, minorVersion, buildNumber);
	TrkIO->PrintToScreen(_L("Implementing TRK API version %d.%d\r\n"), majorAPIVersion, minorAPIVersion);
	TrkIO->PrintToScreen(_L("Press 'Q' to quit.\r\n\r\n"));
#endif
  	
#ifdef SHOW_CONSOLE	
	_LIT(KLfCr, "\r\n");
	
	TPtrC ptr(buf);
	TInt pos = 0;
	TInt lineStart = 0;
	TInt length = 0;
	
	while (KErrNotFound != (pos = ptr.Find(KLfCr)))
	{
		ptr.Set(&buf[lineStart], pos);
		lineStart = lineStart + pos + 2;
		TrkIO->PrintToScreen(ptr);
		TrkIO->PrintToScreen(KLfCr);
		length = buf.Length() - lineStart;
		if (length > 0)
		{
			ptr.Set(&buf[lineStart], length);
		}
	}
#endif

	CActiveScheduler::Start();
	
	trkSession.DisConnect();
	trkSession.ShutDownServer();
	trkSession.Close();
}

//
// EXE entrypoint
//
TInt E32Main()
{	
	__UHEAP_MARK;

	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	if (cleanupStack == 0)
		User::Panic(_L("TRK failed to allocate CTrapCleanup"), __LINE__);
	
	CActiveScheduler* scheduler = new CActiveScheduler;
	if (scheduler == 0)
		User::Panic(_L("TRK failed to allocate CActiveScheduler"), __LINE__);

	CActiveScheduler::Install(scheduler);

#ifdef SHOW_CONSOLE
	TrkIO = new CTrkConsoleIO;
	
	if (!TrkIO)
		User::Panic(_L("TRK failed to allocate CTrkConsoleIO"), __LINE__);
#endif
	
	TRAPD(error, MainL());

#ifdef SHOW_CONSOLE
	if (error != KErrNone)
	{
		TrkIO->PrintToScreen(_L("\r\nUnexpected error %d"), error);
		TrkIO->Terminating();
	}
	SafeDelete(TrkIO);
#endif
	
	SafeDelete(scheduler);
	SafeDelete(cleanupStack);

	__UHEAP_MARKEND;

	User::Exit(0);
	return 0;
}
