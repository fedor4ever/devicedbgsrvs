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


#include "trkserver.h"

static CTrkIO* TrkIO = NULL;

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
// CTrkIO implementation
//
//

//
// CTrkIO constructor
//
CTrkIO::CTrkIO()
	: iConsole(0),
	  iKeyboardInput(0)
{
	TRAPD(error, iConsole = Console::NewL(_L("Trk"), TSize(KConsFullScreen, KConsFullScreen)));
	if (error != KErrNone)
		User::Panic(_L("TrkServer failed to allocate CConsoleBase"), __LINE__);

	iKeyboardInput = new CKeyboardInput(iConsole);
	if (iKeyboardInput == 0)
		User::Panic(_L("TrkServer failed to allocate CKeyboardInput"), __LINE__);

	iKeyboardInput->ListenToKeyboard();
}

//
// CTrkIO destructor
//
CTrkIO::~CTrkIO()
{
	SafeDelete(iKeyboardInput);
	SafeDelete(iConsole);
}

//
// CTrkIO::PrintToScreen
//
//	Print a message to the screen
//
void CTrkIO::PrintToScreen(TRefByValue<const TDesC> aFmt, ...)
{
	VA_LIST(l);
	VA_START(l, aFmt);
	iBuffer.FormatList(aFmt, l);

	iConsole->Printf(iBuffer);
}

//
// CTrkIO::Terminating
//
//	Notify user of termination
//
void CTrkIO::Terminating()
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
	
	TInt sid = RProcess().SecureId();
		
	//TrkIO->PrintToScreen(_L("Welcome to TrkServer for Trk\r\n"));
	//TrkIO->PrintToScreen(_L("Press 'Q' to quit.\r\n\r\n"));

	CActiveScheduler::Start();
}

//
// EXE entrypoint
//
TInt E32Main()
{	
	__UHEAP_MARK;

	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	if (cleanupStack == 0)
		User::Panic(_L("Trk failed to allocate CTrapCleanup"), __LINE__);
	
	CActiveScheduler* scheduler = new CActiveScheduler;
	if (scheduler == 0)
		User::Panic(_L("Trk failed to allocate CActiveScheduler"), __LINE__);

	CActiveScheduler::Install(scheduler);

//	TrkIO = new CTrkIO;
	
//	if (!TrkIO)
//		User::Panic(_L("TrkServer failed to allocate CTrkIO"), __LINE__);
	
	TRAPD(error, MainL());

//	if (error != KErrNone)
//	{
//		TrkIO->PrintToScreen(_L("\r\nUnexpected error %d"), error);
//		TrkIO->Terminating();
//	}
	
//	SafeDelete(TrkIO);
	SafeDelete(scheduler);
	SafeDelete(cleanupStack);

	__UHEAP_MARKEND;

	User::Exit(0);
	return 0;
}
