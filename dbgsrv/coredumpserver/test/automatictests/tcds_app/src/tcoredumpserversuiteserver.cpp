// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Example file/test code to demonstrate how to develop a TestExecute Server
// Developers should take this project as a template and substitute their own
// for (WINS && !EKA2) versions will be xxxServer.Dll and require a thread to be started
// in the process of the client. The client initialises the server by calling the
// one and only ordinal.
//



/**
 @file Te_coredumpserverSuiteServer.cpp
 @internalTechnology
*/

#include "tcoredumpserversuiteserver.h"
#include "DEXCusersidestep.h"
#include "liststep.h"
#include "parameterstep.h"
#include "SELFUsersidestep.h"
#include "SELFpluginstep.h"
#include "DEXCpluginstep.h"
#include "testformatterusersidestep.h"
#include "CDSconnectionstep.h"
#include "CDStokenstep.h"
#include "utracestep.h"
#include "SELFusersidetrace.h"
#include "testsignaling.h"

_LIT(KServerName,"Te_coredumpserverSuite");
CTe_coredumpserverSuite* CTe_coredumpserverSuite::NewL()
/**
 * @return - Instance of the test server
 * Same code for Secure and non-secure variants
 * Called inside the MainL() function to create and start the
 * CTestServer derived server.
 */
	{
	CTe_coredumpserverSuite * server = new (ELeave) CTe_coredumpserverSuite();
	CleanupStack::PushL(server);

	server->ConstructL(KServerName);
	CleanupStack::Pop(server);
	return server;
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
	CTe_coredumpserverSuite* server = NULL;
	// Create the CTestServer derived server
	TRAPD(err,server = CTe_coredumpserverSuite::NewL());
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


CTestStep* CTe_coredumpserverSuite::CreateTestStep(const TDesC& aStepName)
/**
 * @return - A CTestStep derived instance
 * Secure and non-secure variants
 * Implementation of CTestServer pure virtual
 */
	{	
	INFO_PRINTF1(aStepName);
	
	CTestStep* testStep = NULL;
              if(aStepName == KDEXECUserSideStep)   
                            testStep = new CDEXECUserSideStep();
              else if(aStepName == KListsStep)
                            testStep = new CListsStep();
              else if(aStepName == KParametersStep)
                            testStep = new CParametersStep();
              else if(aStepName == KSELFUserSideStep)
                            testStep = new CSELFUserSideStep();
              else if(aStepName == KSELFPluginStep)
            	  			testStep = new CSELFPluginStep();
              else if(aStepName == KDEXECPluginStep)
            	  			testStep = new CDEXECPluginStep();
              else if(aStepName == KTestFormatterUserSideStep)
            	  			testStep = new CTestFormatterUserSideStep();
              else if(aStepName == KCDSConnectionsStep)
            	  			testStep = new CCDSConnectionsStep();
              else if(aStepName == KTokenStep)
            	  			testStep = new CTokenStep();
              else if(aStepName == KUTraceUserSide)
            	  			testStep = new CUTraceStep();
              else if(aStepName == KSELFUserSideTrace)
            	  		testStep = new CSELFUserSideTrace();
              else if(aStepName == KTestSignaling)
                  testStep = new CTestSignaling();
              
	return testStep;
	}

void CTe_coredumpserverSuite::CleanupProcessList(TAny *aArray)
/**
 * Cleanup operation for RProcessPointerList
 */
	{
		RProcessPointerList *processList = static_cast<RProcessPointerList*> (aArray);
	    processList->ResetAndDestroy();
	    processList->Close();
	}
void CTe_coredumpserverSuite::CleanupPluginList(TAny *aArray)
/**
 * Cleanup operation for RProcessPointerList
 */
	{
		RPluginPointerList *pluginList = static_cast<RPluginPointerList*> (aArray);
		pluginList->ResetAndDestroy();
		pluginList->Close();
	}

void CTe_coredumpserverSuite::CleanupThreadList(TAny *aArray)
/**
 * Cleanup operation for RThreadPointerList
 */
	{
		RThreadPointerList *threadList = static_cast<RThreadPointerList*> (aArray);
		threadList->ResetAndDestroy();
		threadList->Close();
	}

CDir* CTe_coredumpserverSuite::DoesFileExistL(const TDesC &aExpectedFile, TFindFile &aFileFinder)
    {
    TParse fileParse;
    fileParse.Set(aExpectedFile, NULL, NULL);
    
    TBuf<63> filePattern;
    filePattern = fileParse.Name();
    filePattern.Append(_L("*"));

    CDir* fileList = NULL;
    aFileFinder.FindWildByDir(filePattern, fileParse.DriveAndPath(), fileList);
    
    if(fileList == NULL)
        {
        User::Leave(KErrNotFound);
        }

    return fileList;
    }

TBool CTe_coredumpserverSuite::FileStartsWith(const TDesC& aRoot, const TDesC& aFilename )
/*
 * @return - If the file starts with the root
 * Takes a descriptor and see if it starts with the root
 */
	{	
	if(aFilename.Length() < aRoot.Length())
		{
		return EFalse;
		}

	TInt rootLength = aRoot.Length();
	TInt cnt = 0;
	
	while(cnt < rootLength)
		{
		
		if(aRoot[cnt] != aFilename[cnt])
			{			
			return EFalse;
			}
		cnt++;	
		}
	LOG_MSG("leaving");
	return ETrue;
	}

void CTe_coredumpserverSuite::CleanupBufferArray(TAny *aArray)
	{
		CDesC16ArrayFlat* buffArray = static_cast<CDesC16ArrayFlat*> (aArray);
		TInt cntr = 0;
		for(cntr =0; cntr<buffArray->MdcaCount(); cntr++)
			{
			TPtrC16 ptr = (*buffArray)[cntr];
			}
	}


