// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// for (WINS && !EKA2) versions will be xxxServer.Dll and require a thread to be started
// in the process of the client. The client initialises the server by calling the
// one and only ordinal.
//



/**
 @file filewritertestserver.cpp
*/
#include "filewritertestserver.h"
#include "filewriterteststeps.h"

// __EDIT_ME__ - Substitute the name of your test server 
_LIT(KServerName,"filewritertestserver");
// __EDIT_ME__ - Use your own server class name
CFileWriterTestServer* CFileWriterTestServer::NewL()
/**
 * @return - Instance of the test server
 * Called inside the MainL() function to create and start the
 * CTestServer derived server.
 */
	{
	// __EDIT_ME__ new your server class here
	CFileWriterTestServer * server = new (ELeave) CFileWriterTestServer ();
	CleanupStack::PushL(server);
	
	// Either use a StartL or ConstructL, the latter will permit
	// Server Logging.

	//server->StartL(KServerName); 
	server->ConstructL(KServerName);
	CleanupStack::Pop(server);
	return server;
	}

// EKA2 much simpler
// Just an E32Main and a MainL()
LOCAL_C void MainL()
/**
 * Much simpler, uses the new Rendezvous() call to sync with the client
 */
	{
	// Leave the hooks in for platform security
#if (defined __DATA_CAGING__)
	RProcess().DataCaging(RProcess::EDataCagingOn);
	RProcess().SecureApi(RProcess::ESecureApiOn);
#endif
	CActiveScheduler* sched=NULL;
	sched=new(ELeave) CActiveScheduler;
	CActiveScheduler::Install(sched);
	// __EDIT_ME__ Your server name
	CFileWriterTestServer * server = NULL;
	// Create the CTestServer derived server
	// __EDIT_ME__ Your server name
	TRAPD(err,server = CFileWriterTestServer::NewL());
	if(!err)
		{
		// Sync with the client and enter the active scheduler
		RProcess::Rendezvous(KErrNone);
		sched->Start();
		}
	delete server;
	delete sched;
	}

// Only a DLL on emulator for typhoon and earlier

GLDEF_C TInt E32Main()
/**
 * @return - Standard Epoc error code on exit
 */
	{
	CTrapCleanup* cleanup = CTrapCleanup::New();
	if(cleanup == NULL)
		{
		return KErrNoMemory;
		}
	TRAP_IGNORE(MainL());
	delete cleanup;
	return KErrNone;
    }

// Create a thread in the calling process
// Emulator typhoon and earlier

// __EDIT_ME__ - Use your own server class name
CTestStep* CFileWriterTestServer::CreateTestStep(const TDesC& aStepName)
/**
 * @return - A CTestStep derived instance
 * Implementation of CTestServer pure virtual
 */
	{
	CTestStep* testStep = NULL;
	// __EDIT_ME__ - Create your own test steps here
	// This server creates just one step but create as many as you want
	// They are created "just in time" when the worker thread is created
	TInt err = KErrNone;

	if(aStepName == KFileWriterTestStep1)
		{
		TRAP(err, testStep = new(ELeave) CFileWriterTestStep1());
		}
	else if(aStepName == KFileWriterTestStep2)
		{
		TRAP(err, testStep = new(ELeave) CFileWriterTestStep2());
		}
	else if(aStepName == KFileWriterTestStep3)
		{
		TRAP(err, testStep = new(ELeave) CFileWriterTestStep3());
		}
	else if(aStepName == KFileWriterTestStep4)
		{
		TRAP(err, testStep = new(ELeave) CFileWriterTestStep4());
		}
	else if(aStepName == KFileWriterTestStep5)
		{
		TRAP(err, testStep = new(ELeave) CFileWriterTestStep5());
		}

	if(err != KErrNone)
		{
		return NULL;
		}
	
	return testStep;
	}

