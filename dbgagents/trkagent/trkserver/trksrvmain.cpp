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
// User includes
#include "trksrvcmdcodes.h"
#include "trksrvserver.h"
#include "trkdebugmanager.h"
#include "TrkConnData.h"
#include "TrkEngine.h"

TTrkConnType GetCmdLineConnTypeL();
void RunServerL();

TTrkConnType GetCmdLineConnTypeL()
{
    TTrkConnType connType = ETrkConnInvalid;
    
    // Get command line argument
    if (User::CommandLineLength() > 0)
    {
        TBuf<512> commandLineArguments;
        User::CommandLine( commandLineArguments );

        // Check command line argument
        if (0 == commandLineArguments.Compare(KCmdLineConnTypeUsb))
        {
            connType = ETrkUsbDbgTrc;
        }
        else if (0 == commandLineArguments.Compare(KCmdLineConnTypeXti))
        {
            connType =  ETrkXti;
        }
    }
    
    return connType;  
}

//
// RunServerL()
// Creates trk server, signals the client and starts the server
//

void RunServerL()
{
	RProcess().SetPriority(EPriorityHigh);
	RThread().SetPriority(EPriorityAbsoluteHigh);

	// Create and install the active scheduler we need
	CActiveScheduler* scheduler = new CActiveScheduler;
	if (scheduler == 0)
		User::Leave(KErrNoMemory);
	// now push the scheduler onto the cleanup stack
	CleanupStack::PushL(scheduler);
		
	CActiveScheduler::Install(scheduler);

	CTrkDebugManager* trkDebugMgr = CTrkDebugManager::NewLC();
	// Create server
	CTrkSrvServer::NewLC(trkDebugMgr);
	TTrkConnType connType = GetCmdLineConnTypeL();
	if (connType != ETrkConnInvalid)
    {
        TRAPD(error, trkDebugMgr->StartDebuggingL(connType));
        if (error != KErrNone)
        {
            // We just ignore the error here since the error is managed by the debug manager
        }    
    }
	else
    {
        TRAPD(error, trkDebugMgr->StartDebuggingL());
        if (error != KErrNone)
        {
            // We just ignore the error here since the error is managed by the debug manager
        }
    } 

	// Initialisation complete, now signal the client
#ifdef EKA2
	User::LeaveIfError(User::RenameThread(KTrkServerName));
#else
	User::LeaveIfError(RThread().RenameMe(KTrkServerName));
#endif
	RProcess::Rendezvous(KErrNone);

	// Ready to run
	CActiveScheduler::Start();

	trkDebugMgr->StopDebugging();
	
	// Cleanup the server and scheduler
	CleanupStack::PopAndDestroy(3, scheduler);
}



//
// Main Entry Point
//

TInt E32Main()
{
	__UHEAP_MARK;
	
	CTrapCleanup* cleanup = CTrapCleanup::New();
	if (cleanup == 0)
		User::Panic(_L("Trk  Server failed to allocate CTrapCleanup"), __LINE__);
	
	TInt error = KErrNone;
	
	if (cleanup)
	{
		TRAP(error, RunServerL());
		delete cleanup;
	}
	
	if (error != KErrNone)
	{
		User::Panic(_L("Trk Server failed to start"), __LINE__);
	}
	
	__UHEAP_MARKEND;

	return error;
}

