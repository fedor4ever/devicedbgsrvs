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

#include <e32std.h>
#include <e32debug.h>
#include <f32file.h>
// User includes

#include "toolssrvscheduler.h"
#include "toolssrvserver.h"


#include "ToolsCmdCodes.h"

void RunServerL();

//
// RunServerL()
// Creates Tools server, signals the client and starts the server
//

void RunServerL()
{
	// Create and install the active scheduler we need
	CToolsSrvScheduler* scheduler = CToolsSrvScheduler::NewLC();
	if (scheduler == 0)
		User::Leave(KErrNoMemory);
		
	CActiveScheduler::Install(scheduler);

	// Create server
	CToolsSrvServer::NewLC();

	// Initialisation complete, now signal the client
#ifdef EKA2
	User::LeaveIfError(User::RenameThread(KToolsServerName));
#else
	User::LeaveIfError(RThread().RenameMe(KToolsServerName));
#endif
	RProcess::Rendezvous(KErrNone);
	
	// Ready to run
	CActiveScheduler::Start();

	// Cleanup the server and scheduler
	CleanupStack::PopAndDestroy(2, scheduler);
	
}



//
// Main Entry Point
//

TInt E32Main()
{
	__UHEAP_MARK;
	
	CTrapCleanup* cleanup = CTrapCleanup::New();
	if (cleanup == 0)
		User::Panic(_L("Tools Server failed to allocate CTrapCleanup"), __LINE__);
	
	TInt error = KErrNone;
	
	if (cleanup)
	{
		TRAP(error, RunServerL());
		delete cleanup;
	}
	
	if (error != KErrNone)
	{
		User::Panic(_L("Tools Server failed to start"), __LINE__);
	}
	
	__UHEAP_MARKEND;

	return error;
}

