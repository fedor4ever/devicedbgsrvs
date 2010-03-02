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
#include "utracestep.h"
#include "tcoredumpserversuitedefs.h"
#include "tcoredumpserverSuiteServer.h"
#include <crashdatasave.h>
#include <s32file.h>
#include <e32property.h>

CUTraceStep::~CUTraceStep()
/**
 * Destructor
 */
	{
	}

CUTraceStep::CUTraceStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName (KUTraceUserSide);
	}

TVerdict CUTraceStep::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	TInt ret = KErrNone;
	ret = iSess.Connect ();
	if ( ret != KErrNone)
		{
		SetTestStepResult ( EFail);
		INFO_PRINTF2 (_L ("Error %d from iSess->Connect()/n"), ret);
		}
	else
		{
		SetTestStepResult (EPass);
		}

	ret = iFs.Connect ();
	if ( ret != KErrNone)
		{
		SetTestStepResult ( EFail);
		INFO_PRINTF2 (_L ("Error %d from iFs->Connect()/n"), ret);
		}
	else
		{
		SetTestStepResult (EPass);
		}

	return TestStepResult ();
	}

void CUTraceStep::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1 (_L ("Starting UTrace User Side Test Suite"));
	TestTraceDataL ();

	}

void CUTraceStep::TestTraceDataL()
	{
	_LIT (KCrashFileName, "c:\\trace");
	HandleCrashL (KCrashFileName);
	}

void CUTraceStep::HandleCrashL(const TDesC& aFileName)
/**
 * @return void
 * Tests handling a crash
 */
	{

	if ( TestStepResult ()==EPass)
		{
		_LIT (KCrashAppParam, "-d1 -t");

		INFO_PRINTF3 (_L ("The CrashApp Param is %S, The Filename is is %S"),
				&KCrashAppParam, &aFileName);

		//Start the process that we intend to crash....
		RProcess crashProcess;
		CleanupClosePushL (crashProcess);

		TInt ret = crashProcess.Create ( KCrashAppFileName, KCrashAppParam);
		if ( ret != KErrNone)
			{
			INFO_PRINTF2 (
					_L ("Error %d from RProcess.Create(z:\\sys\\bin\\crashapp.exe)/n"),
					ret);
			SetTestStepResult (EFail);
			User::Leave (ret);
			}

		LoadUTraceFormatterL ();
		LoadFileWriterL ();

		//configure cds and writer
		_LIT ( KFilePathPrompt, "not_important");

		INFO_PRINTF1 (_L ("Configuring CDS"));
		DoConfigureL (2, CDS_UID.iUid, COptionConfig::ECoreDumpServer,
				COptionConfig::ETUInt, KPostCrashEventActionPrompt, 1,
				KNullDesC, 4, KNullDesC, 0);

		INFO_PRINTF1 (_L ("Configuring File Writer"));
		DoConfigureL ((TInt)(CCrashDataSave::ECoreFilePath), WRITER_UID.iUid,
				COptionConfig::EWriterPlugin, COptionConfig::ETFileName,
				KFilePathPrompt, 1, KNullDesC, 0, aFileName, 0);

		// Observe the process and wait for a crash
		INFO_PRINTF1 (_L ("Observing bad boy crash app"));
		TRAP (ret, iSess.ObservationRequestL ( KCrashAppFileName,
				KCrashAppFileName, ETrue));
		if ( ret != KErrNone)
			{
			INFO_PRINTF2 (
					_L ("Error %d iSess.ObservationRequestL(z:\\sys\\bin\\crashapp.exe)\n"),
					ret);
			SetTestStepResult (EFail);
			User::Leave ( ret);
			}

		//start the crash process
		crashProcess.Resume ();
		CleanupStack::PopAndDestroy (&crashProcess); //this is for crashProcess

		//Monitor the progress of the crash file being generated...
		MonitorProgressL ();
		User::After (5000000);

		//Check dump has been generated...and valid
		CDir *fileList = NULL;
		TFindFile fileFinder(iFs);
		TRAP (ret, fileList = CTe_coredumpserverSuite::DoesFileExistL (
				aFileName, fileFinder));

		if ( (ret != KErrNone) || (fileList == NULL))
			{
			INFO_PRINTF2 (_L ("Expected crash file was not generated:  %d"),
					ret);
			SetTestStepResult (EFail);
			User::Leave ( ret);
			}

		CleanupStack::PushL (fileList);
		for (TInt i = 0; i < fileList->Count (); i++)
			{
			TParse fullName;
			fullName.Set ((*fileList)[i].iName, &fileFinder.File(), NULL);
			VerifyTraceDataL (fullName.FullName ());
			INFO_PRINTF2 (_L ("Deleting %S"), &fullName.FullName ());
			User::LeaveIfError (iFs.Delete (fullName.FullName ()));
			}
		CleanupStack::PopAndDestroy (fileList);

		INFO_PRINTF1 (_L ("Expected trace crash file was generated and parsed successfully"));

		}

	}

void CUTraceStep::MonitorProgressL()
	{
	RProperty crashProgress;
	User::LeaveIfError (crashProgress.Attach (KCoreDumpServUid, ECrashProgress));

	TBuf<50> crashProg;

	TRequestStatus status;
	crashProgress.Subscribe (status);
	User::WaitForRequest (status);
	//Subscribe for next one again...
	crashProgress.Subscribe (status);

	//First one should be the start string = "-"
	User::LeaveIfError (crashProgress.Get (crashProg));
	if ( crashProg != KStartOfUTraceProc)
		{
		INFO_PRINTF1 (_L ("UTRACE formatter has not started processing the data"));
		}

	INFO_PRINTF1 (_L ("UTRACE formatter has started processing the data"));
	INFO_PRINTF1 (_L ("Waiting to be notified of the timeout of the processing. A timeout here is a fail"));
	//Now we wait until its finished
	do
		{
		User::WaitForRequest (status);
		crashProgress.Subscribe (status);

		User::LeaveIfError (crashProgress.Get (crashProg));
		}
	while (crashProg != KEndOfProcessing);

	INFO_PRINTF1 (_L ("UTRACE formatter has finished processing the data"));
	}

void CUTraceStep::DoConfigureL(const TUint32& aIndex, const TUint32& aUID,
		const COptionConfig::TParameterSource& aSource,
		const COptionConfig::TOptionType& aType, const TDesC& aPrompt,
		const TUint32& aNumOptions, const TDesC& aOptions, const TInt32& aVal,
		const TDesC& aStrValue, const TUint aInstance)
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

	config = COptionConfig::NewL ( aIndex, aUID, aSource, aType, aPrompt,
			aNumOptions, aOptions, aVal, aStrValue);

	CleanupStack::PushL (config);

	config->Instance (aInstance);

	//Configure now...
	TRAPD (ret, iSess.SetConfigParameterL (*config));
	if ( ret != KErrNone)
		{
		INFO_PRINTF2 (_L ("Error %d changing param/n"), ret);
		SetTestStepResult (EFail);
		User::Leave (ret);
		}

	CleanupStack::PopAndDestroy (config);
	}

void CUTraceStep::LoadUTraceFormatterL()
/**
 * @return void
 * Utility function to load the UTRACE formatter
 */
	{

	INFO_PRINTF1 (_L ("Attempting to load UTRACE Plugin"));

	TPluginRequest req;
	req.iPluginType = TPluginRequest::EFormatter;
	req.iLoad = ETrue;
	req.iUid = UTRACE_UID;

	// Should be allowed to load utrace formatter
	TRAPD (ret, iSess.PluginRequestL ( req));
	if ( ret != KErrNone)
		{
		ERR_PRINTF2 (_L ("Failed to load writer plugin, error = %d"), ret);
		SetTestStepResult (EFail);
		User::Leave (ret);
		}

	}

void CUTraceStep::LoadFileWriterL()
	{
	TPluginRequest req;
	TUid writerUid = WRITER_UID;
	req.iUid = writerUid;
	req.iPluginType = TPluginRequest::EWriter;
	req.iLoad = ETrue;

	TRAPD (ret, iSess.PluginRequestL ( req));
	if ( ret != KErrNone)
		{
		ERR_PRINTF2 (_L ("Failed to load writer plugin, error = %d"), ret);
		SetTestStepResult (EFail);
		User::Leave (ret);
		}

	}

TVerdict CUTraceStep::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 * Our implementation only gets called if the base class doTestStepPreambleL() did
 * not leave. That being the case, the current test result value will be EPass.
 */
	{
	if ( TestStepResult ()==EPass)
		{
		TInt ret = KErrNone;

		__UHEAP_MARK;

		TRAP (ret, ClientAppL ());
		if ( KErrNone != ret)
			{
			SetTestStepResult (EFail);
			INFO_PRINTF2 (
					_L ("Error %d from CTestFormatterUserSideStep->ClientAppL()"),
					ret);
			}

		__UHEAP_MARKEND;

		}
	return TestStepResult ();
	}

TVerdict CUTraceStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	iSess.Disconnect ();
	User::After (10000000); //ensure we give enough time for session to close
	iFs.Close ();
	return EPass;
	}

void CUTraceStep::VerifyTraceDataL(const TDesC& aFullFilename)
	{
	INFO_PRINTF2 (_L ("VerifyTraceDataL (Not Implemented) The Filename is is %S"),
			 &aFullFilename);

	}

