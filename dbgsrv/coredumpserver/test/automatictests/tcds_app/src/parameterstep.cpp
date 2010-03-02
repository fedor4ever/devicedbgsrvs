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
// Example CTestStep derived implementation
//



/**
 @file ParametersStep.cpp
 @internalTechnology
*/
#include "parameterstep.h"
#include "tcoredumpserversuitedefs.h"
#include <e32property.h>
#include <crashdatasave.h>
#include <e32std.h>
#include <coredumpserverapi.h>

CParametersStep::~CParametersStep()
/**
 * Destructor
 */
	{
	}

CParametersStep::CParametersStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KParametersStep);
	}

TVerdict CParametersStep::doTestStepPreambleL()
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
	
	INFO_PRINTF1(_L("Successfully connected session to the Core Dump Server"));
	
	ret = iSecSess.Connect(securityServerVersion); 
	if(ret != KErrNone)
		{
		SetTestStepResult( EFail );
		ERR_PRINTF2(_L("Error %d from iSecSess->Connect()"), ret);
		}	
	else
		{
		SetTestStepResult(EPass);
		}
	
	INFO_PRINTF1(_L("Successfully connected session to the debug security server"));
	
	ret = iFs.Connect();
	if(ret != KErrNone)
		{
		SetTestStepResult( EFail );
		ERR_PRINTF2(_L("Error %d from iFs->Connect()"), ret);
		}	
	else
		{
		SetTestStepResult(EPass);
		}
	
	INFO_PRINTF1(_L("Successfully connected session to the file server"));
	
	return TestStepResult();
	}

TVerdict CParametersStep::doTestStepL()
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
			INFO_PRINTF2(_L("Error %d from CParametersStep->ClientAppL()"), ret);
			}		
		
		//Make sure its dead
		_LIT(KCrashApp, "crash");	
		KillCrashAppL(KCrashApp);

		__UHEAP_MARKEND;

		}
	return TestStepResult(); 
	}

void CParametersStep::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1(_L("Starting CDS Parameters Test Suite") );
	INFO_PRINTF1(_L("This tests two scenarios: When the main thread crashes and when a child thread crashes"));
	INFO_PRINTF1(_L("In each case we test all possible CDS config values to make sure that the correct things are happening \n\n"));
	
	//Here we are testing the post processing conditions when a main thread crashes and when a child thread crashes
	CrashChildThreadAndTestPostProcL();
	CrashMainThreadAndTestPostProcL();
	
	//Now we test the preprocessing values
	CrashChildThreadAndTestPreProcL();	
	}
void CParametersStep::CrashChildThreadAndTestPreProcL()
/**
 * @return void
 * Tests the pre processing values do the right thing when we crash a child thread
 */
	{
	if (TestStepResult()==EPass)
		{
		INFO_PRINTF1(_L("Testing all pre proccessing options for when child thread crashes"));
		
		_LIT(KCrashAppParams, "-m1 -d2 -c2"); //Will have one child thread that crashes after 2 sec
	
		//1 = suspend thread, 2 = suspend process
		for(TInt preVal = 1; preVal <3; preVal++ )
			{
			INFO_PRINTF3(_L("************* New Crash (Main thread) CDS PreProc val = %d Post Proc val = %d ************* "), preVal, 0);
			//Call crash app
			GenerateAndHandleCrashL(KCrashAppParams, 0, preVal);
			TProcessId pid = IsCrashAppAliveL();
	
			if(preVal == 1)
				{
				//Here the child should be frozen and main thread still running
				if(IsCrashAppMainThreadRunningL())
					{
					INFO_PRINTF1(_L("Main thread running as expected"));
					}
				else
					{
					SetTestStepError(EFail);
					ERR_PRINTF1(_L("Main thread is expected to be running and it is not"));
					User::Leave(KErrGeneral);
					}
				}
			else if(preVal == 2)
				{
				if(!IsCrashAppMainThreadRunningL())
					{
					INFO_PRINTF1(_L("Main thread frozen as expected"));
					}
				else
					{
					SetTestStepError(EFail);
					ERR_PRINTF1(_L("Main thread is expected to be frozen and is not"))	;
					User::Leave(KErrGeneral);
					}
				}
			
			
			//Disconnect the CDS' handle on this exe
			if(pid)
				{
				iSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, EFalse);
				INFO_PRINTF1(_L("Attaching DSS to executable"));
				User::LeaveIfError(iSecSess.AttachExecutable(KCrashAppFileName, EFalse));
				ResumeProcessL(pid);	
				User::LeaveIfError(iSecSess.DetachExecutable(KCrashAppFileName));
				
				_LIT(KCrashApp, "crash");
				KillCrashAppL(KCrashApp);
				User::After(2000000);
				
				if(IsCrashAppAliveL())
					{
					SetTestStepResult( EFail );
					ERR_PRINTF1(_L("Failed to kill app"));
					User::Leave(KErrGeneral);
					}
				}
			
			
			}
		}
	
	}

void CParametersStep::CrashMainThreadAndTestPostProcL()
/**
 * @return void
 * Tests the CDS post processing when the main thread of the crash app dies. 
 * In all cases the app should be dead except when we do not resume the process (option 0)
 */
	{
	if (TestStepResult()==EPass)
		{
		TInt cdsPreConfigVal = 2; //Suspend process (this is irrelevant really, we will check this seperately)
		
		INFO_PRINTF1(_L("Testing all post proccessing options for when main thread crashes (in all cases crash app should be gone)"));
		INFO_PRINTF1(KPostCrashEventActionPrompt);	
		
		_LIT(KCrashAppParams, "-s -d1 -c2"); //Will have a second thread to main, and main will crash after 1 second
	
		TInt cdsPostConfigVal = 8;
		do
			{
			//Right shift 1 to cycle 4,2,1,0
			cdsPostConfigVal>>=1;
		
			INFO_PRINTF3(_L("************* New Crash (Main thread) CDS PreProc val = %d Post Proc val = %d ************* "), cdsPreConfigVal, cdsPostConfigVal);
	
			//Call crash app and handling with range of post processing values (0,1,2,4 inc)
			GenerateAndHandleCrashL(KCrashAppParams, cdsPostConfigVal, cdsPreConfigVal);	
			//Let the process be resumed or killed or whatever
			User::After( 4500000 );				
						
			//Now we validate the post processing values
			TProcessId pid = IsCrashAppAliveL();
			if(pid)
				{
				//If post proc = 0 (suspend) it should still be alive. we need to resume it all so we can kill it
				if(cdsPostConfigVal == 0)
					{
					INFO_PRINTF1(_L("CDS configured not to resume process and it is still alive as expected"));		
					
					//Disconnect the CDS' handle on this exe
					iSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, EFalse);
					INFO_PRINTF1(_L("Attaching DSS to executable"));
					User::LeaveIfError(iSecSess.AttachExecutable(KCrashAppFileName, EFalse));
					ResumeProcessL(pid);	
					User::LeaveIfError(iSecSess.DetachExecutable(KCrashAppFileName));			
					}
				else 
					{
					RDebug::Printf("Error is here ****************");
					SetTestStepError(EFail);
					ERR_PRINTF2(_L("Expected crash app to be dead and it wasn't. Crash was in main thread and CDS post processing was configured at %d"), cdsPostConfigVal);
					User::Leave(KErrGeneral);
					}
				}
			else if(cdsPostConfigVal ==0)
				{
				SetTestStepError(EFail);
				ERR_PRINTF2(_L("Expected crash app to be alive and it wasn't. Crash was in main thread and CDS post processing was configured at %d"), cdsPostConfigVal);
				User::Leave(KErrGeneral);
				}
			else
				{
				INFO_PRINTF2(_L("Crash app died as expected with post proc param %d"), cdsPostConfigVal);
				}		
			
			_LIT(KCrashApp, "crash");
			KillCrashAppL(KCrashApp);
			User::After(3000000 );
			
			if(IsCrashAppAliveL())
				{
				SetTestStepResult( EFail );
				ERR_PRINTF1(_L("Failed to kill app"));
				User::Leave(KErrGeneral);
				}
			}
		while(cdsPostConfigVal > 0);
		}
	}

void CParametersStep::CrashChildThreadAndTestPostProcL()
/**
 * @return void 
 * Tests the CDS post processing when the child thread of the crash app dies. The pre processing
 * step is suspend process
 */
	{
	if (TestStepResult()==EPass)
		{
		TInt cdsPreConfigVal = 2; //Suspend process (this is irrelevant really, we will check this seperately)
		
		INFO_PRINTF1(_L("Testing all post proccessing options for when main thread crashes (in all cases crash app should be gone)"));
		INFO_PRINTF1(KPostCrashEventActionPrompt);	
		
		_LIT(KCrashAppParams, "-m1 -d1 -c2"); //Will have one child thread that crashes after 1 sec
	
		TInt cdsPostConfigVal = 8;
		do
			{
			//Right shift 1 to cycle 4,2,1,0
			cdsPostConfigVal>>=1;
			
			INFO_PRINTF3(_L("************* New Crash (Child thread) CDS PreProc val = %d Post Proc val = %d ************* "),cdsPreConfigVal, cdsPostConfigVal);
			
			//Call crash app and handling with range of post processing values (0,1,2,4 inc)
			GenerateAndHandleCrashL(KCrashAppParams, cdsPostConfigVal, cdsPreConfigVal);	
			//Wait for crashapp to be resumed or killed or whatever
			User::After( 1000000 );				
			
			TProcessId pid = IsCrashAppAliveL();
			
			if(cdsPostConfigVal == 0)//do nothing
				{
				//In this case the whole process is suspended
				if(pid && !IsCrashAppMainThreadRunningL())
					{
					INFO_PRINTF1(_L("Crash app alive and main thread suspended as expected"));				
					}						
				else
					{
					SetTestStepResult( EFail );
					ERR_PRINTF1(_L("Crash app was dead - not expected"));
					User::Leave(KErrGeneral);
					}
				}
			else if(cdsPostConfigVal == 1)//resume thread
				{
				//Only thread is resumed, not whole process
				if(pid && !IsCrashAppMainThreadRunningL())
					{
					INFO_PRINTF1(_L("Crash app alive and main thread suspended as expected"));				
					}						
				else
					{
					SetTestStepResult( EFail );
					ERR_PRINTF1(_L("Crash app was dead - not expected"));
					User::Leave(KErrGeneral);
					}
				}
			else if(cdsPostConfigVal == 2)//resume proc
				{
				//Only thread is resumed, not whole process
				if(pid && IsCrashAppMainThreadRunningL())
					{
					INFO_PRINTF1(_L("Crash app alive and main thread running as expected"));				
					}						
				else
					{
					SetTestStepResult( EFail );
					ERR_PRINTF1(_L("Crash app main thread not running - not expected"));
					User::Leave(KErrGeneral);
					}
				}
			else if(cdsPostConfigVal == 4)//kill proc
				{
				if(pid)				
					{
					SetTestStepResult( EFail );
					ERR_PRINTF1(_L("Process is still alive but should be dead"));	
					User::Leave(KErrGeneral);
					}
				else
					{
					INFO_PRINTF1(_L("Process dead as expected"));
					}				
				}
				
			//Kill the process if its still running
			if(pid)
				{
				
				//Disconnect the CDS' handle on this exe
				iSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, EFalse);
				INFO_PRINTF1(_L("Attaching DSS to executable"));
				User::LeaveIfError(iSecSess.AttachExecutable(KCrashAppFileName, EFalse));
				ResumeProcessL(pid);	
				User::LeaveIfError(iSecSess.DetachExecutable(KCrashAppFileName));
				
				_LIT(KCrashApp, "crash");
				KillCrashAppL(KCrashApp);
				User::After(2000000);
				
				if(IsCrashAppAliveL())
					{
					SetTestStepResult( EFail );
					ERR_PRINTF1(_L("Failed to kill app"));
					User::Leave(KErrGeneral);
					}
				}		
			}
		while(cdsPostConfigVal > 0);
		}
	
	}



void CParametersStep::ResumeProcessL(TUint64 aPid)
/**
Called to resume all the threads of the specified process. CCoreCrashHandler executes it as a post-processing action.
@param aPid ID of process to be resumed
@leave err one of the system wide error codes
iSecSess must be attached to the executable in question
*/
	{
	INFO_PRINTF1(_L("Resuming process"));
	
	//Get the threads we wish to resume
	RThreadPointerList threadList;
    TCleanupItem cleanupProcesses(CTe_coredumpserverSuite::CleanupThreadList, (TAny*)&threadList);
    CleanupStack::PushL(cleanupProcesses);	
	
	iSess.GetThreadsL(threadList, aPid);	

    TInt err = KErrNone;
    //first resume child threads
    TInt looperr = KErrNone;
    for(TInt j = 1; j < threadList.Count(); j++)
        {   
        looperr = iSecSess.ResumeThread( threadList[j]->Id() );
        if( (looperr != KErrNone) && (looperr != KErrNotFound) )
            {
            err = looperr;
            ERR_PRINTF2(_L("CParametersStep::ResumeProcessL - unable to resume thread! err:%d"), err);
            }
        }

    looperr = KErrNone;
    //then resume the main thread
    if(threadList.Count())
        {
        INFO_PRINTF2(_L("resuming main thread:%Ld\n"), threadList[0]->Id());
        looperr = iSecSess.ResumeThread(threadList[0]->Id());
        }

    if( (looperr != KErrNone) && (looperr != KErrNotFound) )
        {
        err = looperr;
        ERR_PRINTF2(_L("CCoreDumpSession::ResumeProcessL - unable to resume main thread! err:%d"), err); 
        }
    CleanupStack::PopAndDestroy(&threadList);
    User::LeaveIfError(err);
	}

TProcessId CParametersStep::IsCrashAppAliveL()
/**
 * @return TInt PID of process if alive, null if not
 * Scans the process list looking for crash app. Returns true if its found
 */
	{
	TProcessId pid = NULL; 
		
	RProcessPointerList procList;
	TCleanupItem cleanupProcesses(CTe_coredumpserverSuite::CleanupProcessList, (TAny*)&procList);
	CleanupStack::PushL(cleanupProcesses);
	
	iSess.GetProcessesL( procList );			

	for(TInt i=0; i < procList.Count(); i++ )
	{				
		if(procList[i]->Name() == KCrashAppFileName)
		{
		INFO_PRINTF1(_L("Found the crash app"));
		pid = procList[i]->Id();	
		}
	}
	CleanupStack::PopAndDestroy(&procList);
	
	return pid;
	
	}

void CParametersStep::KillCrashAppL(const TDesC& aProcessName)
/**
 * @return void
 * Kills specified process
 */
	{	
	_LIT(KAsterisk, "*");
	HBufC* buf = HBufC::NewLC(1+aProcessName.Length());
	TPtr bufPtr = buf->Des();
	bufPtr.Append(aProcessName);
	bufPtr.Append(KAsterisk);
	
	TFindProcess finder(bufPtr);
	TFullName fullname;
	RProcess process;
	while(finder.Next(fullname) == KErrNone)
		{
		TInt err = process.Open(finder);
		if(err == KErrNone)
			{
			INFO_PRINTF1(_L("Killing process"));
			INFO_PRINTF1(process.Name());
			process.Kill(KErrNone);
			process.Close();	        
			}
		else
			{
			ERR_PRINTF2(_L("Failed to kill process. Error = %d "), err);
			}
		}
	CleanupStack::PopAndDestroy(buf); 
	}

void CParametersStep::GenerateAndHandleCrashL(const TDesC& aCrashAppParameters, 
											 const TUint& aCDSPostProcParam,
											 const TUint& aCDSPreProcParam)
/**
 * @return void
 * This will:
 * 1. Load Formatter
 * 2. Load writer
 * 3. Configure CDS to post process after crash		
 * 4. Observe the crash process
 * 5. Start the crash process
 * 6. Monitor the crash progress until its dead
 */
	{		
	//Start the process that we intend to crash....
	RProcess crashProcess;
	CleanupClosePushL(crashProcess);
	
	TInt ret = crashProcess.Create( KCrashAppFileName, aCrashAppParameters);
	if(ret != KErrNone)
		{
		ERR_PRINTF2(_L("Error %d from RProcess .Create(z:\\sys\\bin\\crashapp.exe)/n"), ret);
		SetTestStepResult(EFail);
		User::Leave(ret);
		}

	//Now we configure CDS and writer			
	INFO_PRINTF2(_L("Configuring pre processing of CDS with %d"), aCDSPreProcParam);
	DoConfigureL(EPreCrashEventAction, CDS_UID.iUid, COptionConfig::ECoreDumpServer, COptionConfig::ETUInt, KNullDesC,
				 1, KNullDesC, aCDSPreProcParam, KNullDesC);	
	
	INFO_PRINTF2(_L("Configuring post processing of CDS with %d"), aCDSPostProcParam);	
	DoConfigureL(EPostCrashEventAction, CDS_UID.iUid, COptionConfig::ECoreDumpServer, COptionConfig::ETUInt, KNullDesC,
				 1, KNullDesC, aCDSPostProcParam, KNullDesC );	

   	INFO_PRINTF1(_L("Observing process"));
	iSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, ETrue);
	
	crashProcess.Resume();
	CleanupStack::PopAndDestroy(&crashProcess);	
	
	//This waits for the formatting to complete
	MonitorProgressL();	
	}

void CParametersStep::MonitorProgressL()
/**
 * @return void
 * This method monitors the RProperty for the crash progress. There are several states of this but we are
 * only interested in the start and finish of these properties
 */
	{
	RProperty crashProgress;
	User::LeaveIfError(crashProgress.Attach(KCoreDumpServUid, ECrashProgress));	
	
	TBuf<50> crashProg;			

	TRequestStatus status;
	crashProgress.Subscribe(status);
    
	INFO_PRINTF1(_L("CDS has started processing"));			
	//Now we wait until its finished
	do
		{			
		User::WaitForRequest(status);		
		crashProgress.Subscribe(status);

		User::LeaveIfError(crashProgress.Get(crashProg ));
		LOG_MSG("Found property and it was:");
		}
	while(crashProg != KEndOfProcessing);
	
	INFO_PRINTF1(_L("CDS has finished processing"));			
	
	}

TInt CParametersStep::GetNumberOfProccessesL()
/**
 * @return TInt
 * Utility function to get the number of proccesses running
 */
	{		
	
	RProcessPointerList procList;
    TCleanupItem cleanupProcesses(CTe_coredumpserverSuite::CleanupProcessList, (TAny*)&procList);
    CleanupStack::PushL(cleanupProcesses);

	iSess.GetProcessesL(procList);	
	TInt numProcs = procList.Count();	
	
	CleanupStack::PopAndDestroy();	

	return numProcs;	
	
	}

TInt CParametersStep::GetNumberThreadsL()
/**
 * @return TInt
 * Utility function to get the number of proccesses running
 */
	{	
	RThreadPointerList threadList;
    TCleanupItem cleanupProcesses(CTe_coredumpserverSuite::CleanupThreadList, (TAny*)&threadList);
    CleanupStack::PushL(cleanupProcesses);
	
	iSess.GetThreadsL(threadList);	
	TInt numThreads = threadList.Count();	
	
	CleanupStack::PopAndDestroy();	

	return numThreads;
	}

void CParametersStep::DoConfigureL(const TUint32& aIndex, 
 	    const TUint32& aUID, 
        const COptionConfig::TParameterSource& aSource, 
        const COptionConfig::TOptionType& aType, 
        const TDesC& aPrompt, 
        const TUint32& aNumOptions,
        const TDesC& aOptions,
        const TInt32& aVal, 
        const TDesC& aStrValue )
/**
* @return void
* @param aIndex Internal index to the component that owns the object
* @param aUID UID of the component that owns the object
* @param aSource Type of component that owns the object
* @param aType Type of parameter
* @param aPrompt Prompt to present to user 
* @param aNumOptions Number of options that the parameter can be set to. Only applies if type is ETMultiEntryEnum.
* @param aOptions Comma separated list of options. Applies to ETMultiEntryEnum and ETBool
* @param aVal Integer value. Applies to ETInt, ETUInt, ETBool
* @param aStrValue String value. Applies to ETString, ETFileName, ETMultiEntry, ETBool
*/
	{
	COptionConfig * config;	
	
	config = COptionConfig::NewL( aIndex,
		aUID,
		aSource,
		aType,
		aPrompt,
		aNumOptions,
		aOptions,
		aVal,
		aStrValue);
	
	CleanupStack::PushL(config);
	
	//Configure now...
	TRAPD(ret, iSess.SetConfigParameterL(*config) );
	if(ret != KErrNone)
		{
		ERR_PRINTF2(_L("Error %d changing param"), ret );
		SetTestStepResult(EFail);
		User::Leave(ret);
		}
	
	CleanupStack::PopAndDestroy(config);
	}

TBool CParametersStep::IsCrashAppMainThreadRunningL()
/**
 * Reads the main threads property to make sure its being updated
 */
	{
	TInt mainThreadCounter1 = 0;
	TInt mainThreadCounter2 = 0;
	User::LeaveIfError(RProperty::Get(KCrashAppUid, 0, mainThreadCounter1));
	User::After(1500000);
	User::LeaveIfError(RProperty::Get(KCrashAppUid, 0, mainThreadCounter2));
	return (mainThreadCounter2 > mainThreadCounter1);
	}

TVerdict CParametersStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	iSess.Disconnect();
	iSecSess.Close();
	iFs.Close();
	User::After(5000000); //ensure we give enough time for session to close
	return TestStepResult();
	}
