// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
//  Block server for the sELF lib unit tests

/**
 * @file tcds_unit_sELFlib_block.cpp
 * @internalTechnology
 */

#include "tcds_unit_selflib_block.h"

CSELFLibBlockController* CSELFLibBlockController::NewL()
    {
    CSELFLibBlockController* block = new (ELeave) CSELFLibBlockController();
    return block;
    }

/**
 * Creates the Wrapper object with which we test
 * @param Descriptor of object to create
 * @return Test data wrapper
 * @leaves One of the system wide error codes
 */
CDataWrapper* CSELFLibBlockController::CreateDataL(const TDesC& aData)
    {
    CDataWrapper* wrapper = NULL;

	TBuf8<300> x;
	x.SetLength(aData.Length() * 2);
	x.Copy(aData);

	if(KCSELFLibWrapper() == aData)
		{
		wrapper = CSELFLibWrapper::NewL();
		}
	else
		{
    	User::Panic(_L("Testwrapper Not Found"), KErrNotFound);
		}

    return wrapper;
    }

CSELFLibBlockServer* CSELFLibBlockServer::NewL()
    {
	CSELFLibBlockServer* server = new (ELeave) CSELFLibBlockServer();
	CleanupStack::PushL(server);
	server->ConstructL();
	CleanupStack::Pop(server);
	return server;
	}

CTestBlockController* CSELFLibBlockServer::CreateTestBlock()
    {
    CTestBlockController* controller = NULL;
    TRAPD(err, controller = CSELFLibBlockController::NewL());
    if(KErrNone != err)
		{
    	User::Panic(_L("Failed to create CSELFLibBlockServer"), err);
		}

    return controller;
    }

LOCAL_C void MainL()
	{
	// Leave the hooks in for platform security
#if (defined __DATA_CAGING__)
	RProcess().DataCaging(RProcess::EDataCagingOn);
	RProcess().DataCaging(RProcess::ESecureApiOn);
#endif
	CActiveScheduler* sched=NULL;
	sched=new(ELeave) CActiveScheduler;
	CActiveScheduler::Install(sched);
	CSELFLibBlockServer* server = NULL;
	// Create the CSELFLibBlockServer derived server
	TRAPD(err,server = CSELFLibBlockServer::NewL());
	if(KErrNone == err)
		{
		// Sync with the client and enter the active scheduler
		RDebug::Printf("\nStarting the SELF Library Test Suite\n");
		RProcess::Rendezvous(KErrNone);
		sched->Start();
		}
	delete server;
	delete sched;
	}

GLDEF_C TInt E32Main()
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
