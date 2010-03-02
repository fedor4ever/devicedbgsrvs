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
// CTestStep derived implementation
//



/**
 @file ListsStep.cpp
 @internalTechnology
*/
#include "liststep.h"
#include "tcoredumpserversuitedefs.h"
#include "tcoredumpserversuiteserver.h"

CListsStep::~CListsStep()
/**
 * Destructor
 */
	{
	}

CListsStep::CListsStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KListsStep);
	}

TVerdict CListsStep::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	   
	TInt ret = KErrNone;
	ret = iSess.Connect(); 
	if(ret != KErrNone)
		{
		SetTestStepResult( EFail );
		ERR_PRINTF2(_L("Error %d from iSess->Connect()"), ret);
		}	
	else
		{
		SetTestStepResult(EPass);
		}

	return TestStepResult();
	}


TVerdict CListsStep::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 * Our implementation only gets called if the base class doTestStepPreambleL() did
 * not leave. That being the case, the current test result value will be EPass.
 */
	{
	  
	if (TestStepResult()==EPass)
		{	
		TInt ret = KErrNone;

		__UHEAP_MARK;

		TRAP(ret, ClientAppL());
		if(KErrNone != ret)
			{
			SetTestStepResult(EFail);
			ERR_PRINTF2(_L("Error %d from CListsStep->ClientAppL()"), ret);
			}
		
		__UHEAP_MARKEND;

		}
	return TestStepResult();  

	}



TVerdict CListsStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	
	iSess.Disconnect();
	User::After(10000000); //ensure we give enough time for session to close
	return EPass;	
	}


void CListsStep::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1(_L("Starting Lists Test Suite") );
	ListProcessesL();
	ListThreadsL();
	ListExecutables();
    ListPlugins();
	}


void CListsStep::ListProcessesL()
/**
 * @return void
 * This tests we can succesfully list the processes and that the returned values make sense
 */
	{
	INFO_PRINTF1(_L("Lists Test Suite: ListProcesses") );

	if (TestStepResult()==EPass)
		{

		RProcessPointerList procList;
		TCleanupItem cleanupProcList(CTe_coredumpserverSuite::CleanupProcessList, (TAny*)&procList);
		CleanupStack::PushL(cleanupProcList);

		TRAPD( ret, iSess.GetProcessesL( procList ) );

		if( KErrNone != ret )
			{
			SetTestStepResult(EFail);
			ERR_PRINTF2(_L("Error %d from iSess.GetProcessesL()"), ret);
			return;
			}

		if( procList.Count() == 0 )
			{
			SetTestStepResult(EFail);
			ERR_PRINTF1(_L("Error : GetProcessesL() returned empty list") );
			return;
			}
		else
			{
			RProcess thisProc;
			TInt i;
			for( i=0; i < procList.Count(); i++ )
				{
				if( procList[i]->Id() == thisProc.Id() )
					{
					break;
					}
				}

			if( i == procList.Count() )
				{
				SetTestStepResult(EFail);
				ERR_PRINTF1(_L("Error : This process could not be found in GetProcessesL() list") );
				return;
				}
			}

		CleanupStack::PopAndDestroy();
		}
	}

void CListsStep::ListThreadsL()
/**
 * @return void
 * This checks the threads are listed correctly and the returned values make sense
 */
	{
	INFO_PRINTF1(_L("Lists Test Suite: ListThreads") );

	if (TestStepResult()==EPass)
		{

		RThreadPointerList threadList;
		TCleanupItem cleanupThreadList(CTe_coredumpserverSuite::CleanupPluginList, (TAny*)&threadList);
		CleanupStack::PushL(cleanupThreadList);

		TRAPD( ret, iSess.GetThreadsL( threadList ) );
		if( KErrNone != ret )
			{
			SetTestStepResult(EFail);
			ERR_PRINTF2(_L("Error %d from iSess.GetThreadsL()"), ret);
			return;
			}

		if( threadList.Count() == 0 )
			{
			SetTestStepResult(EFail);
			ERR_PRINTF1(_L("Error : GetThreadsL() returned empty list") );
			return;
			}
		else
			{
			RThread thisThread;
			TInt i;
			for( i=0; i < threadList.Count(); i++ )
				{
				if( threadList[i]->Id() == thisThread.Id() )
					{
					break;
					}
				}

			if( i == threadList.Count() )
				{
				SetTestStepResult(EFail);
				ERR_PRINTF1(_L("Error : This thread could not be found in GetThreadsL() list/n") );
				return;
				}
			}

		CleanupStack::PopAndDestroy();		

		}
	}

void CListsStep::ListExecutables()
/**
 * @return void
 * This tests we can succesfully list the executables and that the returned values make sense
 */
	{
	INFO_PRINTF1(_L("Lists Test Suite: ListExecutables") );

	if (TestStepResult()==EPass)
		{

		RExecutablePointerList execList;

		TRAPD( err, iSess.GetExecutablesL( execList ) );

		if(err != KErrNone)
			{
			SetTestStepResult(EFail);
			INFO_PRINTF2(_L("Error %d from iSess.GetExecutablesL()/n"), err);
			return;
			}

		if( execList.Count() == 0 )
			{
			SetTestStepResult(EFail);
			INFO_PRINTF1(_L("Error : GetExecutablesL() returned empty list/n") );
			return;
			}
		else
			{
			RProcess thisProc;
			TInt i;
			for( i=0; i < execList.Count(); i++ )
				{
				if( execList[i]->Name() == thisProc.FileName() )
					{
					break;
					}
				}

			if( i == execList.Count() )
				{
				SetTestStepResult(EFail);
				INFO_PRINTF1(_L("Error : This executable could not be found in GetExecutablesL() list/n") );
				return;
				}
			}

		execList.ResetAndDestroy();
		execList.Close();

		}
	}

void CListsStep::ListPlugins()
/**
 * @return void
 * This tests we can succesfully list the executables and that the returned values make sense
 */
	{
	INFO_PRINTF1(_L("Lists Test Suite: ListPlugins") );

	if (TestStepResult()==EPass)
		{

		RPluginList pluginList;

		TRAPD( err, iSess.GetPluginListL( pluginList ) );

		if(err != KErrNone)
			{
			SetTestStepResult(EFail);
			INFO_PRINTF2(_L("Error %d from iSess.GetPluginsL()"), err);
			return;
			}

		if( pluginList.Count() == 0 )
			{
			SetTestStepResult(EFail);
			INFO_PRINTF1(_L("Error : GetPluginsL() returned empty list") );
			return;
			}
		else
			{
             
			TInt i;
			for( i=0; i < pluginList.Count(); i++ )
				{
				if( (pluginList[i].iUid == DEXEC_UID) || (pluginList[i].iUid == SELF_UID) )
					{
					break;
					}
				}

			if( i == pluginList.Count() )
				{
				SetTestStepResult(EFail);
				INFO_PRINTF1(_L("Error : DEXC nor SELF plugin could not be found in GetPluginsL() list") );
				return;
				}
			}

		pluginList.Reset();
		pluginList.Close();

		}
	}

