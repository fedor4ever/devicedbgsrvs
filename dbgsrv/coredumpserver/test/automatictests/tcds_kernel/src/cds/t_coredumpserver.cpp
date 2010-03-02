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
 @file
 @internalTechnology
*/

#include "t_coredumpserver.h"
#include "flashdatasource.h"

//Names of functions to be tested - referenced from script file
_LIT(KDataViaDataSource, "DataViaDataSource");
_LIT(KTraceViaDataSource, "TraceViaDataSource");
_LIT(KUntrustedAccess, "UntrustedDataAccess");
/**
 * Constructor for test wrapper
 */
CCoreDumpServerWrapper::CCoreDumpServerWrapper()
	{
	}

/**
 * Destructor
 */
CCoreDumpServerWrapper::~CCoreDumpServerWrapper()
	{
	iCoreDumpSession.Close();
	iSecSess.Close();
	}

/**
 * Two phase constructor for CCoreDumpServerWrapper
 * @return CProcessCrashWrapper object
 * @leave
 */
CCoreDumpServerWrapper* CCoreDumpServerWrapper::NewL()
	{
	CCoreDumpServerWrapper* ret = new (ELeave) CCoreDumpServerWrapper();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CCoreDumpServerWrapper::ConstructL()
	{
	User::LeaveIfError(iCoreDumpSession.Connect());

	TVersion secVers(Debug::KDebugServMajorVersionNumber, Debug::KDebugServMinorVersionNumber, Debug::KDebugServPatchVersionNumber);
	User::LeaveIfError(iSecSess.Connect(secVers));
	}

/**
 * Assign the object
 * @param aObject TAny* to the object to test
 * @leave
 */
void CCoreDumpServerWrapper::SetObjectL(TAny* aObject)
	{}

/**
 * Runs a test preamble
 */
void CCoreDumpServerWrapper::PrepareTestL()
	{
	SetBlockResult(EPass);
	INFO_PRINTF1(_L("CCoreDumpMonitorWrapper::Ready to start test"));
	}

/**
 * Handle a command invoked from the script
 * @param aCommand Is the name of the command for a TBuf
 * @param aSection Is the .ini file section where parameters to the command are located
 * @param aAsyncErrorIndex Is used by the TEF Block framework to handle asynchronous commands.
 */
TBool CCoreDumpServerWrapper::DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
	{
	//__UHEAP_MARK;

	PrepareTestL();

	if (KDataViaDataSource() == aCommand)
		{
		TestDataSourceReturnsSystemCrashDataL();
		}
	else if(KTraceViaDataSource() == aCommand)
		{
		TestDataSourceReturnsTraceDataL();
		}
	else if(KUntrustedAccess() == aCommand)
		{
		TestUntrustedServerAccessL();
		}
	else
		{
		return EFalse;
		}

	//__UHEAP_MARKEND;

	return ETrue;
	}

/**
 * This makes sure a formatter can retrieve data via the flash data source
 */
void CCoreDumpServerWrapper::TestDataSourceReturnsSystemCrashDataL()
	{
	INFO_PRINTF1(_L("TestDataSourceReturnsSystemCrashData"));

	//Load Test Formatter
	CFlashDataSource* src = CFlashDataSource::NewL(iSecSess);
	CleanupStack::PushL(src);

	RCrashInfoPointerList* crashList = new(ELeave)RCrashInfoPointerList;
	TCleanupItem listCleanup(CCoreDumpServerWrapper::CleanupCrashList, (TAny*)crashList);
	CleanupStack::PushL(listCleanup);

	//Get the crashes from the flash partition
	iCoreDumpSession.ListCrashesInFlashL(*crashList);

	if(crashList->Count() == 0)
		{
		ERR_PRINTF1(_L("Failed to find any crashes in the flash that we could check out"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}

	INFO_PRINTF2(_L("Found [%d] crash(es) on the partition"), crashList->Count());

	INFO_PRINTF1(_L("Analysing crash 0"));
	src->AnalyseCrashL(((*crashList)[0])->iCrashId);

	INFO_PRINTF1(_L("Analysis successful"));

	//Now we try and read back the core header via the flash data source
	const TCrashInfoHeader& header = src->GetCrashHeader();

	//There isn't much checking we can do here as a lot of the things are determinable.
	//Its only a smoke test anyway to make sure the data source is functioning
	INFO_PRINTF1(_L("Checking the process ID that crashed was 1"));
	if(header.iPid != 0x1)
		{
		ERR_PRINTF1(_L("Failed to retrieve proper crashed process ID"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}

	INFO_PRINTF1(_L("Checking a valid crashed process ID"));
	if(header.iTid <= 0x1)
		{
		ERR_PRINTF1(_L("Failed to retrieve proper crashed process ID"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}

	const TRmdArmExcInfo registers = src->GetCrashContext();

	const TUint32 cpsr = registers.iCpsr;

	INFO_PRINTF1(_L("Checking a valid CPSR and were in SVR mode when we crashed"));
	if(KCPSRUsrMode == (cpsr & 0x1F))
		{
		ERR_PRINTF2(_L("CPSR = 0x%X    --> Not in SVR mode"), cpsr);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	//Now, erase the partition

	TRAPD(err, iCoreDumpSession.DeleteCrashPartitionL());
	if(KErrNone != err)
		{
		RDebug::Printf("\ndeleteing failed with %d", err);
		}

	INFO_PRINTF1(_L("Negative test. Testing flash data source does as expected when there is no data in partition"));

	TRAP(err, src->AnalyseCrashL(0));
	if(KErrNone == err)
		{
		ERR_PRINTF1(_L("Were able to analyse a non existant crash with KErrNone"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}

	INFO_PRINTF2(_L("Analysis gave %d as expected"), err);

	CleanupStack::PopAndDestroy(2);
	}

/**
 * Testing we can retrieve trace data when it is present
 */
void CCoreDumpServerWrapper::TestDataSourceReturnsTraceDataL()
	{
	INFO_PRINTF1(_L("TestDataSourceReturnsTraceDataL"));

	//Load Test Formatter
	CFlashDataSource* src = CFlashDataSource::NewL(iSecSess);
	CleanupStack::PushL(src);

	RCrashInfoPointerList* crashList = new(ELeave)RCrashInfoPointerList;
	TCleanupItem listCleanup(CCoreDumpServerWrapper::CleanupCrashList, (TAny*)crashList);
	CleanupStack::PushL(listCleanup);

	//Get the crashes from the flash partition
	iCoreDumpSession.ListCrashesInFlashL(*crashList);

	if(crashList->Count() == 0)
		{
		ERR_PRINTF1(_L("Failed to find any crashes in the flash that we could check out"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}

	INFO_PRINTF2(_L("Found [%d] crash(es) on the partition"), crashList->Count());

	INFO_PRINTF1(_L("Analysing crash 0"));
	src->AnalyseCrashL(((*crashList)[0])->iCrashId);

	INFO_PRINTF1(_L("Analysis successful"));

	TUint sizeToRead = 0x100;
	RBuf8 traceData;
	traceData.CreateL(sizeToRead);
	traceData.CleanupClosePushL();

	INFO_PRINTF1(_L("Reading the trace buffer"));

	TRAPD(err, src->ReadTraceBufferL(traceData));
	if(err != KErrNone)
		{
		ERR_PRINTF1(_L("Failed to get trace data"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}

	INFO_PRINTF1(_L("Making sure the trace data is correct"));

	//Can't analyse this data, just make sure its there
	if(sizeToRead != traceData.Length())
		{
		ERR_PRINTF1(_L("Didnt read all the trace data there"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}

	INFO_PRINTF1(_L("Trying to read with a zero sized buffer"));

	//See what happens when we put in a zero size buffer
	RBuf8 buf;
	buf.CreateL(0);
	buf.CleanupClosePushL();

	TRAP(err, src->ReadTraceBufferL(buf));
	if(err != KErrNone)
		{
		ERR_PRINTF1(_L("Failed to get trace data with a zero size buffer"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(4);
		return;
		}

	CleanupStack::PopAndDestroy(4);
	}

/**
 * This tests that untrusted clients cannot do things with the CDS that they shouldnt
 */
void CCoreDumpServerWrapper::TestUntrustedServerAccessL()
	{
	INFO_PRINTF1(_L("TestUntrustedServerAccessL"));

	INFO_PRINTF1(_L("Testing we can't list crashes from the flash partition"));

	RCrashInfoPointerList* crashList = new(ELeave)RCrashInfoPointerList;
	TRAPD(err, iCoreDumpSession.ListCrashesInFlashL(*crashList));
	if(err != KErrPermissionDenied)
		{
		delete crashList;
		ERR_PRINTF1(_L("Allowed to List crashes - shouldnt be. Are you sure the CDS token isn't in the ROM or on the device?"));
		SetBlockResult(EFail);
		return;
		}

	delete crashList;

	INFO_PRINTF1(_L("Testing we can't delete the flash partition"));
	TRAP(err, iCoreDumpSession.DeleteCrashPartitionL());
	if(err != KErrPermissionDenied)
		{
		ERR_PRINTF1(_L("Allowed to delete flash partition - shouldnt be. Are you sure the CDS token isn't in the ROM or on the device?"));
		SetBlockResult(EFail);
		return;
		}

	INFO_PRINTF1(_L("Testing we can't process crash logs"));
	TRAP(err, iCoreDumpSession.ProcessCrashLogL(0));
	if(err != KErrPermissionDenied)
		{
		ERR_PRINTF2(_L("Allowed to process crash logs (%d) - shouldn't be. Are you sure the CDS token isn't in the ROM or on the device?"), err);
		SetBlockResult(EFail);
		return;
		}

	INFO_PRINTF1(_L("All is good. Move along, nothing to see here"));
	}

/**
 * Cleanup item implementation for RCrashInfoPointerList
 * @param aArray pointer to the list that is supposed to be freed
*/
void CCoreDumpServerWrapper::CleanupCrashList(TAny *aArray)
	{
	RCrashInfoPointerList* crashList = static_cast<RCrashInfoPointerList*> (aArray);
	crashList->ResetAndDestroy();
	crashList->Close();
	delete crashList;
	}

//eof
