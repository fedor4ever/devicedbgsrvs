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
#include "TrkTcbCmdCodes.h"
#include "TrkTcbSrvScheduler.h"
#include "TrkTcbSrvServer.h"


void RunServerL();

//
// RunServerL()
// Creates tcb server, signals the client and starts the server
//

void RunServerL()
{
	// Create and install the active scheduler we need
	CTrkTcbSrvScheduler* scheduler = CTrkTcbSrvScheduler::NewLC();
	if (scheduler == 0)
		User::Leave(KErrNoMemory);
		
	CActiveScheduler::Install(scheduler);

	// Create server
	CTrkTcbSrvServer::NewLC();

	// Initialisation complete, now signal the client
#ifdef EKA2
	User::LeaveIfError(User::RenameThread(KTrkTcbServerName));
#else
	User::LeaveIfError(RThread().RenameMe(KTrkTcbServerName));
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
		User::Panic(_L("Trk Tcb Server failed to allocate CTrapCleanup"), __LINE__);
	
	TInt error = KErrNone;
	
	if (cleanup)
	{
		TRAP(error, RunServerL());
		delete cleanup;
	}
	
	if (error != KErrNone)
	{
		User::Panic(_L("Trk Tcb Server failed to start"), __LINE__);
	}
	
	__UHEAP_MARKEND;

	return error;
}

