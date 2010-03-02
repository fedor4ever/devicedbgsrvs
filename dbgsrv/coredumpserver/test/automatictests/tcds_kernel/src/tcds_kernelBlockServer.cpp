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

/**
 @file tcds_kernelBlockServer.cpp
 @internalTechnology
*/

#include "tcds_kernelBlockServer.h"

Ctcds_kernelBlockServer* Ctcds_kernelBlockServer::NewL()
	{
	Ctcds_kernelBlockServer* server = new (ELeave) Ctcds_kernelBlockServer();
	CleanupStack::PushL(server);
	server->ConstructL();
	CleanupStack::Pop(server);
	return server;
	}

CTestBlockController* Ctcds_kernelBlockServer::CreateTestBlock()
/**
 * @return - Instance of the test block controller
 * Called when Ctcds_kernelBlockServer initializing.
 */
	{
	CTestBlockController* controller = NULL;

	TRAPD(err, controller = Ctcds_kernelBlockController::NewL());
	if(KErrNone != err)
		{
		User::Panic(_L("Failed to create Ctcds_kernelBlockController"), err);
		}

	return controller;
	}

// Secure variants much simpler
// For EKA2, just an E32Main and a MainL()
LOCAL_C void MainL()
/**
 * Secure variant
 * Much simpler, uses the new Rendezvous() call to sync with the client
 */
	{
	// Leave the hooks in for platform security
#if (defined __DATA_CAGING__)
	RProcess().DataCaging(RProcess::EDataCagingOn);
	RProcess().DataCaging(RProcess::ESecureApiOn);
#endif
	CActiveScheduler* sched=NULL;
	sched=new(ELeave) CActiveScheduler;
	CActiveScheduler::Install(sched);
	Ctcds_kernelBlockServer* server = NULL;
	// Create the Ctcds_kernelBlockServer derived server
	TRAPD(err,server = Ctcds_kernelBlockServer::NewL());
	if(!err)
		{
		// Sync with the client and enter the active scheduler
		RProcess::Rendezvous(KErrNone);
		sched->Start();
		}
	delete server;
	delete sched;
	}

GLDEF_C TInt E32Main()
/**
 * @return - Standard Epoc error code on process exit
 * Secure variant only
 * Process entry point. Called by client using RProcess API
 */
	{
	__UHEAP_MARK;

	CTrapCleanup* cleanup = CTrapCleanup::New();
	if(cleanup == NULL)
		{
		return KErrNoMemory;
		}
	TRAPD(err,MainL());
	delete cleanup;

	__UHEAP_MARKEND;
	return err;
	}
