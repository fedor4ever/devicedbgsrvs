// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "flashdump.h"
#include "debuglogging.h"
#include <e32base.h>
#include <e32std.h>
#include <e32cons.h>

_LIT(KTextConsoleTitle, "The CrashFlash Dumper");

LOCAL_D CConsoleBase* console; // write all messages to this


GLDEF_C TInt E32Main();

//  Local Functions

LOCAL_C HBufC* ReadCommandLineL()
	{
	TInt cmdSize = User::CommandLineLength();

	if(cmdSize > 0)
		{
		HBufC* cmd = HBufC::NewL(cmdSize);
		TPtr p(cmd->Des());

		User::CommandLine(p);
		
		return cmd;
		}
		
	console->Printf(_L("\nusage: flashdump <filename>\n"));
	console->Printf(_L("\nAny key to continue\n"));
	console->Getch();
	
	return NULL;
	}

LOCAL_C void MainL()
	{
	CCrashFlashDump* dumper = CCrashFlashDump::NewL();
	CleanupStack::PushL(dumper);
	
	HBufC* file = ReadCommandLineL();
	if(!file)
		{
		User::Leave(KErrArgument);
		}
	
	CleanupStack::PushL(file);
	
	LOG_MSG("Opening File");	
	dumper->OpenFileL(file->Des());
	
	LOG_MSG("Dumping file to flash");
	dumper->DumpFlashToFileL();
	
	CleanupStack::PopAndDestroy(file);
	CleanupStack::PopAndDestroy(dumper);	
	}

LOCAL_C void DoStartL()
	{
	MainL();
	}

//  Global Functions

GLDEF_C TInt E32Main()
	{
	// Create cleanup stack
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();

	// Create output console
	TRAPD(createError, console = Console::NewL(KTextConsoleTitle, TSize(KConsFullScreen,KConsFullScreen)));
	if (createError)
	return createError;

	// Run application code inside TRAP harness, wait keypress when terminated
	TRAPD(mainError, DoStartL());
	if(mainError == KErrNone)
		{
		LOG_MSG("Flash dump succesful!");
		}
	else
		{
		LOG_MSG2("Flash dump failed with [%d]", mainError);
		}
	
	delete console;
	delete cleanup;
	__UHEAP_MARKEND;
	return mainError;
	}


//eof


