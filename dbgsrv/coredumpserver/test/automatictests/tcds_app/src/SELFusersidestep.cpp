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
 @file SELFStep.cpp
 @internalTechnology
*/
#include "SELFUserSideStep.h"
#include "tcoredumpserverSuiteDefs.h"
#include "tcoredumpserverSuiteServer.h"
#include "optionconfig.h"
#include <coredumpserverapi.h>
#include <e32property.h>
#include <crashdatasave.h>

CSELFUserSideStep::~CSELFUserSideStep()
/**
 * Destructor
 */
	{
	}

CSELFUserSideStep::CSELFUserSideStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KSELFUserSideStep);
	}

TVerdict CSELFUserSideStep::doTestStepPreambleL()
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
		INFO_PRINTF2(_L("Error %d from iSess->Connect()/n"), ret);
		}	
	else
		{
		SetTestStepResult(EPass);
		}
	
	ret = iFs.Connect();
	if(ret != KErrNone)
		{
		SetTestStepResult( EFail );
		INFO_PRINTF2(_L("Error %d from iFs->Connect()/n"), ret);
		}	
	else
		{
		SetTestStepResult(EPass);
		}

	return TestStepResult();
	}


TVerdict CSELFUserSideStep::doTestStepL()
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
			INFO_PRINTF2(_L("Error %d from CSELFStep->ClientAppL()"), ret);
			}		

		__UHEAP_MARKEND;

		}
	return TestStepResult();  
	}

void CSELFUserSideStep::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1(_L("Starting SELF Test Suite") );
	HandleCrashL();
    //CheckParametersL();
	}

void CSELFUserSideStep::HandleCrashL()
/**
 * @return void
 * Tests handling a crash
 */
	{

	if(TestStepResult()==EPass)
		{

		
		TPtrC crashAppParam;
		TPtrC crashFileName;
		TPtrC writerToUse;
		
		if(!GetStringFromConfig(ConfigSection(), KTe_CrashAppParam, crashAppParam) ||
		   !GetStringFromConfig(ConfigSection(), KTe_CrashFileName, crashFileName) ||
		   !GetStringFromConfig(ConfigSection(), KWriterToUse, writerToUse))
			{
			INFO_PRINTF1(_L("Failed to get data from ini file"));
			SetTestStepResult(EFail);
			User::Leave(KErrGeneral);
			}
		
		INFO_PRINTF4(_L("The CrashApp Param is %S, The Filename is is %S, The writer is %S"), &crashAppParam, &crashFileName,&writerToUse); // Block end
				
		//Convert writer config to hex
		TLex luther(writerToUse);
		TUint32 writer;
		luther.Val(writer, EHex);
		
		//Start the process that we intend to crash....
		RProcess crashProcess;
		CleanupClosePushL(crashProcess);
		
		TInt ret = crashProcess.Create( KCrashAppFileName, crashAppParam);
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Error %d from RProcess .Create(z:\\sys\\bin\\crashapp.exe)/n"), ret);
			SetTestStepResult(EFail);
			User::Leave(ret);
			}
		
		INFO_PRINTF1(_L("Started userside crash app"));
		
		//Load SELF Formatter
		LoadSELFFormatterL();
		
		TPluginRequest req;
		TUid writerUid = TUid::Uid(writer);
		req.iUid = writerUid;
		req.iPluginType = TPluginRequest::EWriter;
		req.iLoad = ETrue;
		
		TRAP(ret, iSess.PluginRequestL( req ));
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed to load writer plugin, error = %d"), ret);
			SetTestStepResult(EFail);
			User::Leave(ret);
			}
		
		INFO_PRINTF1(_L("Loaded writer plugin successfully"));
		
		//Now we configure CDS and writer		
		_LIT( KFilePathPrompt, "not_important" );
		
		DoConfigureL(2, CDS_UID.iUid, COptionConfig::ECoreDumpServer, COptionConfig::ETUInt, KPostCrashEventActionPrompt,
					 1, KNullDesC, 4, KNullDesC, 0);	

		DoConfigureL((TInt)(CCrashDataSave::ECoreFilePath), writerUid.iUid, COptionConfig::EWriterPlugin, COptionConfig::ETFileName,
						    KFilePathPrompt, 1, KNullDesC, 0, crashFileName, 0);		
		
		// Observe the process and wait for a crash
		TRAP(ret, iSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, ETrue) );
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Error %d iSess.ObservationRequestL(z:\\sys\\bin\\crashapp.exe)\n"), ret);
			SetTestStepResult(EFail);
			User::Leave( ret );
			}
		
		//start the crash process
		crashProcess.Resume();
		CleanupStack::PopAndDestroy(&crashProcess); //this is for crashProcess
	
		//Monitor the progress of the crash file being generated...
		MonitorProgressL();
        User::After(5000000);

		//Check dump has been generated...and valid
        CDir *fileList = NULL;
        TFindFile fileFinder(iFs);
		TRAP(ret, fileList = CTe_coredumpserverSuite::DoesFileExistL(crashFileName, fileFinder));
		
		if((ret != KErrNone) || (fileList == NULL))
			{
			INFO_PRINTF2(_L("Expected crash file was not generated:  %d"), ret);
			SetTestStepResult(EFail);
			User::Leave( ret );
			}

        CleanupStack::PushL(fileList);
        for(TInt i = 0; i < fileList->Count(); i++)
            {
            TParse fullName;
            fullName.Set((*fileList)[i].iName, &fileFinder.File(), NULL);
            User::LeaveIfError(iFs.Delete(fullName.FullName()));
            }
        CleanupStack::PopAndDestroy(fileList);
		
		INFO_PRINTF1(_L("Expected crash file was generated successfully"));
		
		RProcessPointerList procList;
		TCleanupItem cleanupProcesses(CTe_coredumpserverSuite::CleanupProcessList, (TAny*)&procList);
		CleanupStack::PushL(cleanupProcesses);
	
		iSess.GetProcessesL( procList );			
		
		INFO_PRINTF1(_L("Looking through the procList"));
		for(TInt i=0; i < procList.Count(); i++ )
			{				
			if(procList[i]->Name() == KCrashAppFileName)
				{
				SetTestStepResult( EFail );
				INFO_PRINTF1(_L("Crash application was not killed as expected after crash"));
				User::Leave(KErrGeneral);
				}
			}
		CleanupStack::PopAndDestroy(&procList);
		}
	}

void CSELFUserSideStep::CheckParametersL()
/**
 * This checks the SELF parameters - are we creating a stack file and text file?
 */
	{
	if(TestStepResult()==EPass)
		{
        //Start the process that we intend to crash....
        RProcess crashProcess;
        CleanupClosePushL(crashProcess);
        
        _LIT(KCrashAppParam, "-c4 -d1");
        TInt ret = crashProcess.Create( KCrashAppFileName, KCrashAppParam);
        if(ret != KErrNone)
            {
            INFO_PRINTF2(_L("Error %d from RProcess .Create(z:\\sys\\bin\\crashapp.exe)/n"), ret);
            SetTestStepResult(EFail);
            User::Leave(ret);
            }

        LoadSELFFormatterL();
        LoadSELFFormatterL();
        LoadSELFFormatterL();

        LoadFileWriterL();
        LoadFileWriterL();
        LoadFileWriterL();

        BindPluginsL(3);

        _LIT(KTrue, "True");
        _LIT(KFalse, "False");

        
        //enable system wide
        DoConfigureL(0, SELF_UID.iUid, COptionConfig::EFormatterPlugin, COptionConfig::ETString, 
                KNullDesC, 1, KNullDesC, 1, KFalse, 0);

        //disable process data
        DoConfigureL(1, SELF_UID.iUid, COptionConfig::EFormatterPlugin, COptionConfig::ETString, 
                KNullDesC, 1, KNullDesC, 0, KFalse, 1);

        //disable thread segs
        DoConfigureL(4, SELF_UID.iUid, COptionConfig::EFormatterPlugin, COptionConfig::ETString, 
                KNullDesC, 1, KNullDesC, 0, KTrue, 1);

        //disable process segs
        DoConfigureL(7, SELF_UID.iUid, COptionConfig::EFormatterPlugin, COptionConfig::ETString, 
                KNullDesC, 1, KNullDesC, 0, KTrue, 1);

        //disable data segs
        DoConfigureL(2, SELF_UID.iUid, COptionConfig::EFormatterPlugin, COptionConfig::ETString, 
                KNullDesC, 1, KNullDesC, 0, KTrue, 2);
        
        //disable code segs
        DoConfigureL(3, SELF_UID.iUid, COptionConfig::EFormatterPlugin, COptionConfig::ETString, 
                KNullDesC, 1, KNullDesC, 0, KTrue, 2);

        //disable register segs
        DoConfigureL(5, SELF_UID.iUid, COptionConfig::EFormatterPlugin, COptionConfig::ETString, 
                KNullDesC, 1, KNullDesC, 0, KTrue, 2);

        //disable executable segs
        DoConfigureL(6, SELF_UID.iUid, COptionConfig::EFormatterPlugin, COptionConfig::ETString, 
                KNullDesC, 1, KNullDesC, 0, KTrue, 2);


        TUid writerUid = WRITER_UID;//file writer
        _LIT( KCrashFileName, "c:\\self_chkparam");
        TBuf<63> fileName;
        for(TInt i = 0; i < 3; i++)
        {
            fileName = KCrashFileName;
            fileName.AppendFormat(_L("%d"), i);
            DoConfigureL(CCrashDataSave::ECoreFilePath, writerUid.iUid, COptionConfig::EWriterPlugin, COptionConfig::ETFileName,
                            KNullDesC, 1, KNullDesC, 0, fileName, i);	
        }

        // Observe the process and wait for a crash
        TRAP(ret, iSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, ETrue));
        if(ret != KErrNone)
            {
            ERR_PRINTF2(_L("Error %d iSess.ObservationRequestL(z:\\sys\\bin\\crashapp.exe)\n"), ret);
            SetTestStepResult(EFail);
            User::Leave( ret );
            }
        
        //start the crash process
        crashProcess.Resume();
        INFO_PRINTF1(_L("Started userside crash app"));
        CleanupStack::PopAndDestroy(&crashProcess);
    
        //Monitor the progress of the crash file being generated...
        MonitorProgressL();

        //Check dumps have been generated...		
        CDir *fileList = NULL;
        TFindFile fileFinder(iFs);
        TRAP(ret, fileList = CTe_coredumpserverSuite::DoesFileExistL(KCrashFileName, fileFinder));

        if( (ret != KErrNone) || (fileList == NULL) )
            {
            ERR_PRINTF2(_L("Expected crash files were not generated:  %d"), ret);
            SetTestStepResult(EFail);
            User::Leave( ret );
            }
        
        TInt validFiles = 0;
        CleanupStack::PushL(fileList);
        _LIT(KElfDump, "z:\\sys\\bin\\elfdump.exe");
        _LIT(KCoreFile, "c:\\core");
        for(TInt i = 0; i < fileList->Count(); i++)
            {
            TParse fullName;
            fullName.Set((*fileList)[i].iName, &fileFinder.File(), NULL);
            INFO_PRINTF2(_L("%S"), &fullName.FullName());

            RProcess elfdump;
            TInt err = elfdump.Create(KElfDump, fullName.FullName());
            if(err == KErrNone)
                {
                TRequestStatus status;
                elfdump.Rendezvous(status);
                elfdump.Resume();
                User::WaitForRequest(status);
                elfdump.Close();
                if(status.Int() == KErrNone)
                    {
                    validFiles++;
                    }
                else
                    {
                    INFO_PRINTF2(_L("elfdump postprocessing finished abnormally:%d"), status.Int());
                    }
                }
            else
                {
                INFO_PRINTF2(_L("unable to create elfdump process:%d"), err);
                }
            
            //Now we validate the trace data

            User::LeaveIfError(iFs.Delete(fullName.FullName()));
            }
        User::LeaveIfError(iFs.Delete(KCoreFile));

        if( fileList->Count() != 3 )
            {
            ERR_PRINTF1(_L("Expected crash files were not generated"));
            SetTestStepResult(EFail);
            User::Leave(KErrNotFound);
            }
        CleanupStack::PopAndDestroy(fileList);

        if( validFiles != 3 )
            {
            ERR_PRINTF1(_L("Expected crash files were generated but format was not valid"));
            SetTestStepResult(EFail);
            User::Leave(KErrCorrupt);
            }

		RProcessPointerList procList;
		TCleanupItem cleanupProcesses(CTe_coredumpserverSuite::CleanupProcessList, (TAny*)&procList);
		CleanupStack::PushL(cleanupProcesses);
		
		iSess.GetProcessesL( procList );			
		
		INFO_PRINTF1(_L("Looking through the procList"));
		for(TInt i=0; i < procList.Count(); i++ )
			{				
			if(procList[i]->Name() == KCrashAppFileName)
				{
				SetTestStepResult( EFail );
				ERR_PRINTF1(_L("Crash application was not killed as expected after crash"));
				User::Leave(KErrGeneral);
				}
			}
		CleanupStack::PopAndDestroy(&procList);
		}
	}

void CSELFUserSideStep::DoConfigureL(const TUint32& aIndex, 
		  	     const TUint32& aUID, 
                 const COptionConfig::TParameterSource& aSource, 
                 const COptionConfig::TOptionType& aType, 
                 const TDesC& aPrompt, 
                 const TUint32& aNumOptions,
                 const TDesC& aOptions,
                 const TInt32& aVal, 
                 const TDesC& aStrValue,
                 const TUint aInstance)
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
	
    config->Instance(aInstance);

	//Configure now...
	TRAPD(ret, iSess.SetConfigParameterL(*config) );
	if(ret != KErrNone)
		{
		INFO_PRINTF2(_L("Error %d changing param/n"), ret );
		SetTestStepResult(EFail);
		User::Leave(ret);
		}

	CleanupStack::PopAndDestroy(config);
	}

void CSELFUserSideStep::MonitorProgressL()
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
	User::WaitForRequest(status);
	//Subscribe for next one again...
	crashProgress.Subscribe(status);
	
	//First one should be the start string = "-"
	User::LeaveIfError(crashProgress.Get(crashProg ));		
	if(crashProg != KStartOfSELFProc)
		{
		INFO_PRINTF1(_L("SELF formatter has not started processing the data"));
		}
	
	INFO_PRINTF1(_L("SELF formatters have started processing the data"));
	INFO_PRINTF1(_L("Waiting to be notified of the timeout of the processing. A timeout here is a fail"));
	//Now we wait until its finished
	do
		{			
		User::WaitForRequest(status);		
		crashProgress.Subscribe(status);

		User::LeaveIfError(crashProgress.Get(crashProg ));		
		}
	while(crashProg != KEndOfProcessing);
	
	INFO_PRINTF1(_L("SELF formatter has finished processing the data"));			
	
	}

void CSELFUserSideStep::LoadSELFFormatterL()
/**
 * @return void
 * Utility function to load the SELF formatter
 */
	{
	
	INFO_PRINTF1(_L("Attempting to load SELF Plugin"));
	
	TPluginRequest req;
	req.iPluginType = TPluginRequest::EFormatter;
	req.iLoad = ETrue;
	req.iUid = SELF_UID;

	// Should be allowed to load Symbian ELF
    TRAPD(ret, iSess.PluginRequestL( req ));
    if(ret != KErrNone)
        {
        ERR_PRINTF2(_L("Failed to load self formatter plugin, error = %d"), ret);
        SetTestStepResult(EFail);
        User::Leave(ret);
        }
	
	}

void CSELFUserSideStep::LoadFileWriterL()
    {
    TPluginRequest req;
    TUid writerUid = WRITER_UID; 
    req.iUid = writerUid;
    req.iPluginType = TPluginRequest::EWriter;
    req.iLoad = ETrue;
    
    TRAPD(ret, iSess.PluginRequestL( req ));
    if(ret != KErrNone)
        {
        ERR_PRINTF2(_L("Failed to load writer plugin, error = %d"), ret);
        SetTestStepResult(EFail);
        User::Leave(ret);
        }

    }

void CSELFUserSideStep::BindPluginsL(const TInt aCount)
/**
 * @return void
 * Utility function to load the SELF formatter
 */
	{
	INFO_PRINTF1(_L("Attempting to bind plugins"));
	
	TPluginRequest req;
	req.iPluginType = TPluginRequest::EFormatter;
	req.iUid = TUid::Uid(0);

    for(TInt i = 0; i < aCount; i++)
        {
        req.iIndex = req.iPair = i;
        TRAPD(ret, iSess.PluginRequestL( req ));

        if(ret != KErrNone)
            {
            ERR_PRINTF2(_L("Failed to bind plugin pair=%d"), i);
            SetTestStepResult(EFail);
            User::Leave(ret);
            }
        }
	}


TVerdict CSELFUserSideStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	iSess.Disconnect();
	User::After(7000000); //ensure we give enough time for session to close
	iFs.Close();
	return EPass;	
	}



