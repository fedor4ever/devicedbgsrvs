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
 @file CFlashDataSourceWrapper.cpp
 @internalTechnology 
*/
#include "CFlashDataSourceWrapper.h"

using namespace Debug;

//Names of functions to be tested - referenced from script file
_LIT(KNewL_Debug, "NewL");
_LIT(KReadCrashLog, "ReadCrashLog");
_LIT(KGetFlashBuffer, "GetFlashBuffer");
_LIT(KProcessCrashHeader1, "ProcessCrashHeader1");
_LIT(KProcessCrashHeader2, "ProcessCrashHeader2");
_LIT(KGetProcessListL1, "GetProcessListL1");
_LIT(KGetProcessListL2, "GetProcessListL2");
_LIT(KGetThreadListL1, "GetThreadListL1");
_LIT(KGetThreadListL2, "GetThreadListL2");
_LIT(KGetThreadListL3, "GetThreadListL3");
_LIT(KReadRegistersL1, "ReadRegistersL1");
_LIT(KReadMemoryL1, "ReadMemoryL1");
_LIT(KReadMemoryL2, "ReadMemoryL2");
_LIT(KGetCodeSegmentsL1, "GetCodeSegmentsL1");
_LIT(KGetCodeSegmentsL2, "GetCodeSegmentsL2");
_LIT(KGetCodeSegmentsL3, "GetCodeSegmentsL3");

_LIT(KReadTraceBufferL1, "ReadTraceBufferL1");
_LIT(KReadTraceBufferL2, "ReadTraceBufferL2");
_LIT(KCalcChecksum, "CalcChecksum");
_LIT(KTraceDataSize, "TraceDataSize");
_LIT(KTraceDataSizeNotFound, "TraceDataSizeNotFound");

_LIT8(KCrashDummyData, "This is a sample write");

const TInt FLASH_ALIGN = sizeof(TInt32);
const TInt START_OF_FLASH = 0;

/**
 * Constructor for test wrapper
 */
CFlashDataSourceWrapper::CFlashDataSourceWrapper()
	:iObject(NULL)
	{
	}

/**
 * Destructor
 */
CFlashDataSourceWrapper::~CFlashDataSourceWrapper()
	{
	}

/**
 * Two phase constructor for CFlashDataSourceWrapper
 * @return CFlashDataSourceWrapper object
 * @leave
 */
CFlashDataSourceWrapper* CFlashDataSourceWrapper::NewL()
	{
	CFlashDataSourceWrapper* ret = new (ELeave) CFlashDataSourceWrapper();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CFlashDataSourceWrapper::ConstructL()
	{
	
	}

/** 
 * Assign the object
 *  
 * @param aObject TAny* to the object to test
 * @leave
 */
void CFlashDataSourceWrapper::SetObjectL(TAny* aObject)
	{
	delete iObject;
	iObject = NULL;
	iObject = static_cast<CFlashDataSource*> (aObject);	
	}

/**
 * Runs a test preamble
 */
void CFlashDataSourceWrapper::PrepareTestL()
	{
	SetBlockResult(EPass);		
	
	INFO_PRINTF1(_L("Connecting to DSS"));
	//get a session to the security server
	User::LeaveIfError(iSecSess.Connect(securityServerVersion));
	
	INFO_PRINTF1(_L("Erasing crash log"));
	User::LeaveIfError(iSecSess.EraseCrashLog(0, 1));
	
	INFO_PRINTF1(_L("Creating data source"));
	
	//Hackage: TEF doesnt support preamble/postamble and destructs iObject each time. Until they sort it out
	//or document how it is done the hackage shall have to continue. Oh for JUnit....
	delete iObject;		//to be sure
	iObject = CFlashDataSource::NewL(iSecSess);
	//end of hackage
	
	INFO_PRINTF1(_L("Ready to start test"));
	}

/**
 * Process command to see what test to run 
 */
TBool CFlashDataSourceWrapper::DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
	{
	__UHEAP_MARK;	
	
	PrepareTestL();
	
	if (KNewL_Debug() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::NewL()");
		TRAPD(err , DoCmdCrashFlashDataSource_NewL_TestL());	
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed!");
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if (KReadCrashLog() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::ReadCrashLog()");
		TRAPD(err , DoCmd_ReadCrashLog_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if (KGetFlashBuffer() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::GetFlashBuffer()");
		TRAPD(err , DoCmd_GetFlashBuffer_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if (KProcessCrashHeader1() == aCommand)
		{
		TRAPD(err , DoCmd_ProcessCrashHeader1_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if (KProcessCrashHeader2() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::ProcessCrashHeader() - 2nd test");
		TRAPD(err , DoCmd_ProcessCrashHeader2_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}	
	else if( KGetProcessListL1() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::GetProcessListL() - 1st test");
		TRAPD(err , DoCmd_GetProcessListL1_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if ( KGetProcessListL2() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::GetProcessListL() - 2nd test");
		TRAPD(err , DoCmd_GetProcessListL2_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if (KGetThreadListL1() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::GetThreadListL() - 1st test");
		TRAPD(err , DoCmd_GetThreadListL1_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if( KGetThreadListL2() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::GetThreadListL() - 2nd test");
		TRAPD(err , DoCmd_GetThreadListL2_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if( KGetThreadListL3() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::GetThreadListL() - 3rd test");
		TRAPD(err , DoCmd_GetThreadListL3_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}	
	else if( KReadRegistersL1() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::ReadRegistersL() - 1st test");
		TRAPD(err , DoCmd_ReadRegistersL1_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if( KReadMemoryL1() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::ReadMemoryL() - 1st test");
		TRAPD(err , DoCmd_ReadMemoryL1_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if( KReadMemoryL2() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::ReadMemoryL() - 2nd test");
		TRAPD(err , DoCmd_ReadMemoryL2_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if( KGetCodeSegmentsL1() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource::GetCodeSegmentsL() - 1st test");
		TRAPD(err , DoCmd_GetCodeSegmentsL1_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
	else if( KGetCodeSegmentsL2() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource - Code Segment Analysis - 2nd test");
		TRAPD(err , DoCmd_GetCodeSegmentsL2_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}	
	else if( KGetCodeSegmentsL3() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource - Code Segment Analysis - 3rd test");
		TRAPD(err , DoCmd_GetCodeSegmentsL3_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}	
	else if( KReadTraceBufferL1() == aCommand)
		{
		RDebug::Printf("Looking at CFlashDataSource - Trace Buffer - 1st test");
		TRAPD(err , DoCmd_ReadTraceBufferL1_Test_L());
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed! [%d]", err);
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
			}
		}
  	else if(KCalcChecksum() == aCommand)
  		{ 
  		TRAPD(err , DoCmd_CalculateChecksum_TestL());
  		if(BlockResult() != EPass || KErrNone != err)
  			{
  			RDebug::Printf("\tFailed! [%d]", err);
  			}		
  		}
  	else if(KTraceDataSizeNotFound() == aCommand)
  		{ 
  		TRAPD(err , DoCmd_TraceDataSizeNotFoundL_TestL());
  		if(BlockResult() != EPass || KErrNone != err)
  			{
  			RDebug::Printf("\tFailed! [%d]", err);
  			}		
  		}
  	else if(KTraceDataSize() == aCommand)
  	  		{ 
  	  		TRAPD(err , DoCmd_TraceDataSizeL_TestL());
  	  		if(BlockResult() != EPass || KErrNone != err)
  	  			{
  	  			RDebug::Printf("\tFailed! [%d]", err);
  	  			}		
  	  		}
  	else if(KReadTraceBufferL2() == aCommand)
			{ 
			TRAPD(err , DoCmd_ReadTraceBufferL2_Test_L());
			if(BlockResult() != EPass || KErrNone != err)
				{
				RDebug::Printf("\tFailed! [%d]", err);
				}		
			}	
	else 		
		{
		RDebug::Printf("Not found");
		
		delete iObject;	
		iSecSess.Close();
		
		__UHEAP_MARKEND;
		
		return EFalse;
		}
	
	delete iObject;	
	iSecSess.Close();	
	
	__UHEAP_MARKEND;
	
	return ETrue;
	}

/**
 * This tests that we can succesfully construct 
 * @leave
 */
void CFlashDataSourceWrapper::DoCmdCrashFlashDataSource_NewL_TestL()
	{
	//first we delete the CFlashDataSource created by our post amble to ensure we are testing a fresh object
	delete iObject;
	
	SetBlockResult(EPass);
	
	INFO_PRINTF1(_L("Testing: CFlashDataSource::NewL()"));		
	
	//Try to create a flash data source object
	TRAPD(err, iObject = CFlashDataSource::NewL(iSecSess));
	
	//Check errors
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Leave when creating CFlashDataSource: [%d]"),err);
		SetBlockResult(EFail);
		}
	
	//make sure its not null
	if(!iObject)
		{
		ERR_PRINTF1(_L("Problems with creating CFlashDataSource object"));
		SetBlockResult(EFail);
		}	
	
	INFO_PRINTF1(_L("CFlashDataSource object constructed succesfully"));	
	}

/**
 * Tests the ReadCrashLog method on the flash data source
 */
void CFlashDataSourceWrapper::DoCmd_ReadCrashLog_L()
	{	
	INFO_PRINTF1(_L("Testing CFlashDataSource::ReadCrashLog()"));	
	
	//Write some expected data to the flash partition
	TUint32 size = 0;
	
	INFO_PRINTF1(_L("Writing dummy data to crash log"));
	iSecSess.WriteCrashConfig(START_OF_FLASH, KCrashDummyData, size);
	
	INFO_PRINTF1(_L("Reading dummy data back from crash log"));
	TInt err = iObject->ReadCrashLog(START_OF_FLASH, size);
	
	//make sure we got the right data back
	if( KErrNone != err || 0 != iObject->GetFlashBuffer().Compare(KCrashDummyData) )
		{
		ERR_PRINTF2(_L("Failed to read the correct data back via flash data source: Err = [%d]"), err);
		SetBlockResult(EFail);
		return;
		}
	
	//test the parameters	
	INFO_PRINTF1(_L("Testing that reading from a negative position will fail"));
	
	TInt32 negative = -2;
	err = iObject->ReadCrashLog(negative, size);
	
	if(err != KErrArgument)
		{
		ERR_PRINTF1(_L("We were able to read data from a negative position"));
		SetBlockResult(EFail);
		return;
		}
	}

/**
 * Tests the flash data source get flash buffer
 */
void CFlashDataSourceWrapper::DoCmd_GetFlashBuffer_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::GetFlashBuffer()"));
		
	//Ensure that the buffer has been created
	INFO_PRINTF1(_L("Ensuring the buffer has been succesfully created"));

	TDes8& buf = iObject->GetFlashBuffer();
	
	if(KInitialBufferSize != buf.Size())
		{
		ERR_PRINTF1(_L("Buffer was not created"));
		SetBlockResult(EFail);
		return;
		}
	}

/**
 * Tests that we correctly process the crash header
 */
void CFlashDataSourceWrapper::DoCmd_ProcessCrashHeader1_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::ProcessCrashHeader() - 1st Test"));
	
	TInt64 tid = 22;	
	
	iContextHdr.iNumRegisters = 0;
	iInfHdr.iFlashAlign = FLASH_ALIGN;
	iInfHdr.iTid = tid;
	iInfHdr.iCrashId = CRASH_ID;
	
	TInt logSz = iInfHdr.GetSize() + iContextHdr.GetSize() + iOffsetsHdr.GetSize();
	iInfHdr.iLogSize = logSz;
	
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + TRegisterSet::KSCMRegisterSetMaxSize + KMaxCacheSize;
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);	

	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	
	INFO_PRINTF1(_L("Writing header to flash"));
	
	TUint32 size = 0;		
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data, size));	
	
	CleanupStack::PopAndDestroy();
	
	INFO_PRINTF1(_L("Processing crash header"));
	
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to process crash log: [%d]"), err);
		SetBlockResult(EFail);
		return;
		}
	
	//Read the buffer back
	TDes8& buf = iObject->GetFlashBuffer();
	
	INFO_PRINTF1(_L("Ensuring crash header read back is correct"));	
	
	//Test our getters work correctly too
	if(tid != iObject->GetCrashedThreadId())
		{
		ERR_PRINTF1(_L("Unable to Get Crashed Thread ID from header"));
		SetBlockResult(EFail);
		return;
		}
	
	if(FLASH_ALIGN != iObject->GetFlashAlignment())
		{
		ERR_PRINTF1(_L("Unable to Get Correct flash alignment from header"));
		SetBlockResult(EFail);
		return;
		}
	
	if(logSz != iObject->GetCrashLogSize())
		{
		ERR_PRINTF1(_L("Unable to Get correct log size from header"));
		SetBlockResult(EFail);
		return;
		}	
	
	INFO_PRINTF1(_L("All is good. Move along now."));	
	}

/**
 * Negative test to make sure if there is no header we get told so
 */
void CFlashDataSourceWrapper::DoCmd_ProcessCrashHeader2_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::ProcessCrashHeader() - 2nd Test"));
	
	//write a struct that isnt TCrashInfoHeader
	TProcessData dummy;	
	TPtr8 data((TUint8*)&dummy, sizeof(TProcessData), sizeof(TProcessData));
	
	TUint32 size = 0;
	
	INFO_PRINTF1(_L("Writing header to flash"));	
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data, size));
	
	INFO_PRINTF1(_L("Processing crash header"));	
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));	
	if(KErrNotFound != err)
		{
		ERR_PRINTF2(_L("Process header should have been corrupt but wasnt [%d]"), err);
		SetBlockResult(EFail);
		return;
		}
	}

/**
 * Tests we can succesfully read back known processes from flash
 */
void CFlashDataSourceWrapper::DoCmd_GetProcessListL1_Test_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::GetProcessListL() - 1st test"));
	
	//We need to write some process data (will just do 2) to the flash, and update the header with its location
	TInt64 p1id = 100;
	TInt32 p1prior = 4;
	
	TInt64 p2id = 200;
	TInt32 p2prior = 5;
	
	//Process names
	_LIT8(KProc1, "t_proc1");
	_LIT8(KProc2, "tproc2");
	
	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + TRegisterSet::KSCMRegisterSetMaxSize + 2 * TProcessData::KSCMProcessDataMaxSize + KMaxCacheSize;
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);			
	
	TCrashOffsetsHeader hdr;
	hdr.iPLstOffset = sizeof(TCrashInfoHeader) + sizeof(TCrashOffsetsHeader); //starts right after headers 
	
	//The first process data
	TProcessData proc1;
	proc1.iPid = p1id;
	proc1.iPriority = p1prior;
	proc1.iNamesize = KProc1().Size();
	proc1.iName = KProc1;
	
	//The second process data
	TProcessData proc2;
	proc2.iPid = p2id;
	proc2.iPriority = p2prior;
	proc2.iNamesize = KProc2().Size();
	proc2.iName = KProc2;
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	iOffsetsHdr.iPLstOffset = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize(); //starts right after headers
	iInfHdr.iLogSize = iOffsetsHdr.iPLstOffset + proc1.GetSize() + proc2.GetSize();		
	
	INFO_PRINTF1(_L("Writing known processes to flash"));
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	proc1.Serialize(writer);
	proc2.Serialize(writer);
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));
	
	//Now we try get these processes back via GetProcessListL
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to process crash header: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy();
		return;
		}
	
	RProcessPointerList *processList = new(ELeave)RProcessPointerList;
	TCleanupItem processCleanup(CFlashDataSource::CleanupProcessList, (TAny*)processList);
	CleanupStack::PushL(processCleanup);
		
	TUint wr = 0;
	
	INFO_PRINTF1(_L("Getting process list"));
	TRAP(err, iObject->GetProcessListL(*processList, wr));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to get process list: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	if(processList->Count() != 2)
		{
		ERR_PRINTF2(_L("Didnt find the expected 2 processes. There were [%d] instead"), processList->Count());
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	CProcessInfo* p1 = (*processList)[0];
	CProcessInfo* p2 = (*processList)[1];
	
	INFO_PRINTF1(_L("Retrieveing processes"));
	if(!p1 || !p2)
		{
		ERR_PRINTF1(_L("Failed to retrieve non null processes"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	INFO_PRINTF1(_L("Making sure processes are as expected"));
	
	//Check ID's
	if(p1->Id() != proc1.iPid  ||  p2->Id() != proc2.iPid)
		{
		ERR_PRINTF1(_L("Failed to retrieve correct process ID's:"));
		ERR_PRINTF3(_L("P1 expected [%d] but found [%d]"), proc1.iPid, p1->Id());
		ERR_PRINTF3(_L("P2 expected [%d] but found [%d]"), proc2.iPid, p2->Id());
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}	
	
	//check names
	RBuf wide;	
	wide.CreateL(KProc1().Size());
	wide.CleanupClosePushL();	
	wide.Copy(KProc1);
	
	INFO_PRINTF1(_L("Checking process 1 name is ok"));
	if( p1->Name().Compare(wide) != 0 )
		{
		ERR_PRINTF3(_L("Wrong name retrieved, expected [%S] but got [%S]"), &KProc1, &wide);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}
	
	wide.Copy(KProc2);
	
	INFO_PRINTF1(_L("Checking process 2 name is ok"));
	if( p2->Name().Compare(wide) != 0 )
		{
		ERR_PRINTF3(_L("Wrong name retrieved, expected [%S] but got [%S]"), &KProc2, &wide);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}	

	INFO_PRINTF1(_L("All looks good"));
	
	CleanupStack::PopAndDestroy(3);

	}

/**
 * Negative tests should the flash contain corrupt data
 */
void CFlashDataSourceWrapper::DoCmd_GetProcessListL2_Test_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::GetProcessList() - 2nd test"));
	
	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 3 * TRegisterSet::KSCMRegisterSetMaxSize + KMaxCacheSize;
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);				
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	iOffsetsHdr.iPLstOffset = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize(); //starts right after headers
	iInfHdr.iLogSize = iOffsetsHdr.iPLstOffset + 2 * iContextHdr.GetSize();		
	
	INFO_PRINTF1(_L("Writing known processes to flash"));
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	iContextHdr.Serialize(writer);	
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));
	
	RProcessPointerList *processList = new(ELeave)RProcessPointerList;
	TCleanupItem processCleanup(CFlashDataSource::CleanupProcessList, (TAny*)processList);
	CleanupStack::PushL(processCleanup);
	
		
	INFO_PRINTF1(_L("Getting process list"));
	
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to process header [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	
	TUint wr = 0;
	TRAP(err, iObject->GetProcessListL( *processList, wr));	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Should have retrieved zero sized process list"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	
	if(processList->Count() != 0)
		{
		ERR_PRINTF1(_L("Should have retrieved zero sized process list"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	INFO_PRINTF1(_L("Everything is as expected"));
	
	CleanupStack::PopAndDestroy(2);
	}

/**
 * Tests to make sure we can get back a thread list (two here) from the flash
 */
void CFlashDataSourceWrapper::DoCmd_GetThreadListL1_Test_L()
	{	
	INFO_PRINTF1(_L("Testing CFlashDataSource::GetThreadListL() 1st Test - System wide thread list"));
	
	//We need to write some thread data (will just do 2) to the flash, and update the header with its location
	TInt64 t1id = 100;
	TInt32 t1prior = 4;
	TInt32 t1SvcSp = 20;
	TInt32 t1UsrStkSize = 40;
	
	TInt64 t2id = 200;
	TInt32 t2prior = 5;
	TInt32 t2SvcSp = 25;
	TInt32 t2UsrStkSize = 47;	
	
	//thread names
	_LIT8(KThread1, "t_thread1");
	_LIT8(KThread2, "thread2");	

	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 
						TRegisterSet::KSCMRegisterSetMaxSize + 2 * TThreadData::KSCMThreadDataMaxSize + 
						KMaxCacheSize;
	
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);		
	
	TInt owner1 = 100;
	TInt owner2 = 200;
	
	//The first thread data
	TThreadData t1;
	t1.iTid = t1id;
	t1.iPriority = t1prior;
	t1.iSvcSP = t1SvcSp;
	t1.iUsrStacksize = t1UsrStkSize;
	t1.iNamesize = KThread1().Size();
	t1.iName = KThread1;
	t1.iOwnerId = owner1;
	
	//The second thread data
	TThreadData t2;
	t2.iTid = t2id;
	t2.iPriority = t2prior;
	t2.iSvcSP = t2SvcSp;
	t2.iUsrStacksize = t2UsrStkSize;
	t2.iNamesize = KThread2().Size();
	t2.iName = KThread2;
	t2.iOwnerId = owner2;
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	iOffsetsHdr.iTLstOffset = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize(); //starts right after headers
	iInfHdr.iLogSize = iOffsetsHdr.iTLstOffset + t1.GetSize() + t2.GetSize();		
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	t1.Serialize(writer);
	t2.Serialize(writer);	
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));

	//Now we try get these threads back via GetThreadListL
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to process crash header: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy();
		return;
		}
	
	RThreadPointerList *threadList = new(ELeave)RThreadPointerList;
	TCleanupItem threadCleanup(CFlashDataSource::CleanupThreadList, (TAny*)threadList);
	CleanupStack::PushL(threadCleanup);
	
	TUint wr = 0;
	
	INFO_PRINTF1(_L("Getting thread list"));
	TRAP(err, iObject->GetThreadListL(((TUint64)-1), *threadList, wr));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to get thread list: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	if(threadList->Count() != 2)
		{
		ERR_PRINTF1(_L("Retrieved wrong number of threads"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
		
	CThreadInfo* ti1 = (*threadList)[0];
	CThreadInfo* ti2 = (*threadList)[1];
	
	INFO_PRINTF1(_L("Retrieveing threads"));
	if(!ti1 || !ti2)
		{
		ERR_PRINTF1(_L("Failed to retrieve non null threads"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	INFO_PRINTF1(_L("Making sure processes are as expected"));
	
	//Check ID's
	if(ti1->Id() != t1.iTid ||  ti2->Id() != t2.iTid)
		{
		ERR_PRINTF1(_L("Failed to retrieve correct thread ID's:"));
		ERR_PRINTF3(_L("T1 expected [%d] but found [%d]"), t1.iTid, ti1->Id());
		ERR_PRINTF3(_L("T2 expected [%d] but found [%d]"), t2.iTid, ti2->Id());
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}	
	
	//check names
	RBuf wide;	
	wide.CreateL(KThread1().Size());
	wide.CleanupClosePushL();	
	wide.Copy(KThread1);
	
	INFO_PRINTF1(_L("Checking thread 1 name is ok"));
	if( ti1->Name().Compare(wide) != 0 )
		{
		ERR_PRINTF3(_L("Wrong name retrieved, expected [%S] but got [%S]"), &KThread1, &wide);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}
	
	if(KThread2().Size() > wide.Size())
		{
		wide.ReAllocL(KThread2().Size());
		}
	wide.Copy(KThread2);
	
	INFO_PRINTF1(_L("Checking thread 2 name is ok"));
	if( ti2->Name().Compare(wide) != 0 )
		{
		ERR_PRINTF3(_L("Wrong name retrieved, expected [%S] but got [%S]"), &KThread2, &wide);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}	

	INFO_PRINTF1(_L("All looks good"));
	
	CleanupStack::PopAndDestroy(3);	
	
	}

/**
 * Looks at a process specific thread list
 */
void CFlashDataSourceWrapper::DoCmd_GetThreadListL3_Test_L()
	{	
	INFO_PRINTF1(_L("Testing CFlashDataSource::GetThreadListL() 3rd Test - Process specific thread list"));
	
	//We need to write some thread data (will just do 2) to the flash, and update the header with its location
	TInt64 t1id = 100;
	TInt32 t1prior = 4;
	TInt32 t1SvcSp = 20;
	TInt32 t1UsrStkSize = 40;
	
	TInt64 t2id = 200;
	TInt32 t2prior = 5;
	TInt32 t2SvcSp = 25;
	TInt32 t2UsrStkSize = 47;	
	
	//thread names
	_LIT8(KThread1, "t_thread1");
	_LIT8(KThread2, "thread2");	

	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 
						TRegisterSet::KSCMRegisterSetMaxSize + 2 * TThreadData::KSCMThreadDataMaxSize + 
						KMaxCacheSize;
	
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);		
	
	TInt owner1 = 100;
	TInt owner2 = 200;
	
	//The first thread data
	TThreadData t1;
	t1.iTid = t1id;
	t1.iPriority = t1prior;
	t1.iSvcSP = t1SvcSp;
	t1.iUsrStacksize = t1UsrStkSize;
	t1.iNamesize = KThread1().Size();
	t1.iName = KThread1;
	t1.iOwnerId = owner1;
	
	//The second thread data
	TThreadData t2;
	t2.iTid = t2id;
	t2.iPriority = t2prior;
	t2.iSvcSP = t2SvcSp;
	t2.iUsrStacksize = t2UsrStkSize;
	t2.iNamesize = KThread2().Size();
	t2.iName = KThread2;
	t2.iOwnerId = owner2;
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	iOffsetsHdr.iTLstOffset = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize(); //starts right after headers
	iInfHdr.iLogSize = iOffsetsHdr.iTLstOffset + t1.GetSize() + t2.GetSize();		
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	t1.Serialize(writer);
	t2.Serialize(writer);	
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));

	//Now we try get these threads back via GetThreadListL
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to process crash header: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy();
		return;
		}
	
	RThreadPointerList *threadList = new(ELeave)RThreadPointerList;
	TCleanupItem threadCleanup(CFlashDataSource::CleanupThreadList, (TAny*)threadList);
	CleanupStack::PushL(threadCleanup);
	
	TUint wr = 0;
	
	INFO_PRINTF1(_L("Getting thread list"));
	TRAP(err, iObject->GetThreadListL(owner1, *threadList, wr));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to get thread list: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	if(threadList->Count() != 1)
		{
		ERR_PRINTF1(_L("Retrieved wrong number of threads"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
		
	CThreadInfo* ti1 = (*threadList)[0];
	
	INFO_PRINTF1(_L("Retrieveing threads"));
	if(!ti1)
		{
		ERR_PRINTF1(_L("Failed to retrieve non null threads"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	INFO_PRINTF1(_L("Making sure processes are as expected"));
	
	//Check ID's
	if(ti1->Id() != t1.iTid)
		{
		ERR_PRINTF1(_L("Failed to retrieve correct thread ID's:"));
		ERR_PRINTF3(_L("T1 expected [%d] but found [%d]"), t1.iTid, ti1->Id());
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}	
	
	//check names
	RBuf wide;	
	wide.CreateL(KThread1().Size());
	wide.CleanupClosePushL();	
	wide.Copy(KThread1);
	
	INFO_PRINTF1(_L("Checking thread 1 name is ok"));
	if( ti1->Name().Compare(wide) != 0 )
		{
		ERR_PRINTF3(_L("Wrong name retrieved, expected [%S] but got [%S]"), &KThread1, &wide);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}

	INFO_PRINTF1(_L("All looks good"));
	
	CleanupStack::PopAndDestroy(3);	
	
	}


/**
 * Negative tests should the flash contain corrupt data
 */
void CFlashDataSourceWrapper::DoCmd_GetThreadListL2_Test_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::GetThreadList() - 2nd test"));
	
	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 3 * TRegisterSet::KSCMRegisterSetMaxSize + KMaxCacheSize;
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);				
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	iOffsetsHdr.iTLstOffset = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize(); //starts right after headers
	iInfHdr.iLogSize = iOffsetsHdr.iTLstOffset + 2 * iContextHdr.GetSize();		
	
	INFO_PRINTF1(_L("Writing known processes to flash"));
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	iContextHdr.Serialize(writer);	
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));
	
	RThreadPointerList *threadList = new(ELeave)RThreadPointerList;
	TCleanupItem threadCleanup(CFlashDataSource::CleanupThreadList, (TAny*)threadList);
	CleanupStack::PushL(threadCleanup);
	
		
	INFO_PRINTF1(_L("Getting thread list"));
	
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to process header [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}	
	
	TUint wr = 0;	
	TRAP(err, iObject->GetThreadListL(START_OF_FLASH, *threadList, wr));
	
	if(threadList->Count() != 0)
		{
		ERR_PRINTF1(_L("Should have retrieved zero sized thread list"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	INFO_PRINTF1(_L("Everything is as expected"));
	
	CleanupStack::PopAndDestroy(2);
	
	}

/**
 * Tests we can read registers back from the flash succesfully
 */
void CFlashDataSourceWrapper::DoCmd_ReadRegistersL1_Test_L()
	{
	
	/**
	 * The 3 different register sets corrospond to the 3 different register related configuration
	 * options:
	 * 			ECrashedThreadFullRegisters starts from iCTFullReg
	 * 			EThreadsUsrRegisters starts from iSysSvrReg
	 * 			EThreadsSvrRegisters starts from iSysUsrReg
	 * 
	 * Are going to write the following data to flash:
	 * 
	 * |--TCrashHeader--||--TRegisterValue--| ... |--TRegisterValue--|      at the ECrashedThreadFullRegisters point
	 * 					 <--               For a crashed thread                      -->    
	 * 					 |--TRegisterValue--| ... |--TRegisterValue--|      at the EThreadsUsrRegisters point
	 * 					 <--    For a system thread (at user registers point)        -->  
	 * 					 |--TRegisterValue--| ... |--TRegisterValue--|      at the EThreadsSvrRegisters point
	 * 					 <--    For a system thread  (at svr register point)      -->        
	 */
	
	INFO_PRINTF1(_L("Testing CFlashDataSource::ReadRegistersL() Test1"));
	
	TThreadId crashedThread = TThreadId(139); //arbitrary vals
	TThreadId otherThread = TThreadId(15);		//arbitrary vals

	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 6 * TRegisterValue::KSCMRegisterValueMaxSize + KMaxCacheSize;
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);	
	
	//Headers - as much as we need	
	iInfHdr.iTid = crashedThread.Id();
	iInfHdr.iCrashId = CRASH_ID;	
	iOffsetsHdr.iCTFullRegOffset = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize(); //starts right after headers	
	iContextHdr.iNumRegisters = 0; //no context
	
	//For this test we will put in 2 registers for each config option
	TRegisterValue r1;
	r1.iValue32 = 0xDEADDEAA;
	r1.iOwnId = crashedThread.Id();
	
	TRegisterValue r2;
	r2.iValue32 = 0xDEADEADB;
	r2.iOwnId = crashedThread.Id();
	
	TRegisterValue r3;
	r3.iValue32 = 0xDEADEADC;
	r3.iOwnId = otherThread.Id();
	
	TRegisterValue r4;
	r4.iValue32 = 0xDEADEADD;
	r4.iOwnId = otherThread.Id();
	
	TRegisterValue r5;
	r5.iValue32 = 0xDEADEADE;
	r5.iOwnId = otherThread.Id();
	
	TRegisterValue r6;
	r6.iValue32 = 0xDEADEADF;
	r6.iOwnId = otherThread.Id();

	iOffsetsHdr.iSysSvrRegOffset = iOffsetsHdr.iCTFullRegOffset + r1.GetSize() + r2.GetSize();
	iOffsetsHdr.iSysUsrRegOffset = iOffsetsHdr.iSysSvrRegOffset + r3.GetSize() + r4.GetSize();	
	iInfHdr.iLogSize = iOffsetsHdr.iSysUsrRegOffset + r6.GetSize() + r5.GetSize();	
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	r1.Serialize(writer);
	r2.Serialize(writer);
	r3.Serialize(writer);
	r4.Serialize(writer);
	r5.Serialize(writer);
	r6.Serialize(writer);
	
	INFO_PRINTF1(_L("Writing known registers to flash"));
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(0, data ,written));
	
	//Now we try and get them back via ReadRegistersL
	RRegisterList regList;
	CleanupClosePushL(regList);
	
	INFO_PRINTF1(_L("Reading back registers"));
	
	TRAPD(err, iObject->AnalyseCrashL(1));	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to analyse crash header: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	//Read for crashed thread first (hard coded in the absence of TSymbianInfo for now)
	TRAP(err, iObject->ReadRegistersL(crashedThread.Id(), regList));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to read registers: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	if(regList.Count() != 2)
		{
		ERR_PRINTF2(_L("Got wrong amount of registers back: [%d]"), regList.Count());
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	if(regList[0].iValue32 != r1.iValue32 || regList[1].iValue32 != r2.iValue32)
		{
		ERR_PRINTF1(_L("Got wrong register values back"));
		ERR_PRINTF3(_L("\tExpected Expected [0x%X] but got [0x%X]"),r1.iValue32,regList[0].iValue32);
		ERR_PRINTF3(_L("\tExpected Expected [0x%X] but got [0x%X]"),r2.iValue32,regList[1].iValue32);
		
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	//and for "system" thread
	TRAP(err, iObject->ReadRegistersL(otherThread.Id(), regList));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to read registers: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}

	if(regList.Count() != 4)
		{
		ERR_PRINTF2(_L("Got wrong amount of registers back: [%d]"), regList.Count());
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	if(regList[0].iValue32 != r3.iValue32 
			|| regList[1].iValue32 != r4.iValue32
			|| regList[2].iValue32 != r5.iValue32
			|| regList[3].iValue32 != r6.iValue32)
		{
		ERR_PRINTF1(_L("Got wrong register values back"));
		ERR_PRINTF3(_L("\tExpected Expected [0x%X] but got [0x%X]"),r3.iValue32,regList[0].iValue32);
		ERR_PRINTF3(_L("\tExpected Expected [0x%X] but got [0x%X]"),r4.iValue32,regList[1].iValue32);
		ERR_PRINTF3(_L("\tExpected Expected [0x%X] but got [0x%X]"),r5.iValue32,regList[2].iValue32);
		ERR_PRINTF3(_L("\tExpected Expected [0x%X] but got [0x%X]"),r6.iValue32,regList[3].iValue32);

		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	INFO_PRINTF1(_L("Got back all registers as expected"));
	
	CleanupStack::PopAndDestroy(2);	
	}

/** 
 * Tests that we can read memory from the crash log
 */
void CFlashDataSourceWrapper::DoCmd_ReadMemoryL1_Test_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::ReadMemoryL() Test1"));
	
	//buffer to hold memory
	TInt32 numBytes = 8; //we will dump 8 bytes of memory
	TUint8 memoryToDump = 0xAB;		
	
	RBuf8 mem;
	mem.CreateL(numBytes * sizeof(TUint8));	
	mem.CleanupClosePushL();
	
	for(TInt cnt = numBytes -1; cnt >= 0; cnt--)
		{
		mem.Append(&memoryToDump, sizeof(TUint8));
		}
	
	//buffer to hold log
	TInt dataLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + TRegisterSet::KSCMRegisterSetMaxSize + numBytes + TMemoryDump::KSCMMemDumpMaxSize + TRawData::KSCMRawDataMaxSize;
	
	RBuf8 data;	
	data.CreateL(dataLength);
	data.SetLength(dataLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);		
	
	TUint32 startAdd = 0x10000000;
	TUint64 procId = 100;
	
	TMemoryDump memDump;
	memDump.iStartAddress = startAdd;
	memDump.iPid = procId;
	memDump.iLength = mem.Length();
	
	TRawData rawData;
	rawData.iLength = mem.Length();	
	rawData.iData.Set(mem.MidTPtr(0));
	
	TInt32 crashSize = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize() + memDump.GetSize() + rawData.GetSize();
	iInfHdr.iLogSize = crashSize;
	iInfHdr.iCrashId = CRASH_ID;
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	memDump.Serialize(writer);
	rawData.Serialize(writer);
	
	INFO_PRINTF1(_L("Writing known memory to flash"));
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));
	
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to analyse crash: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	RBuf8 found;
	found.CreateL(mem.Length());
	found.CleanupClosePushL();
	
	
	//We read memory by thread ID. So we will create an arbitrary thread and assign it to belong to the
	//process ID we have dumped memory for
	TInt64 threadId = 23;
	iObject->AssignOwner(procId, threadId);
	
	TRAP(err, iObject->ReadMemoryL(threadId, startAdd, mem.Length(), found));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to read memory: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}
	
	for(TInt cnt = found.Length() -1; cnt >= 0; cnt--)
		{
		if(found[cnt] != memoryToDump)
			{
			ERR_PRINTF2(_L("Wrong memory returned = [0x%X]"),found[cnt]);
			SetBlockResult(EFail);
			CleanupStack::PopAndDestroy(3);
			return;
			}
		}	
	
	INFO_PRINTF1(_L("Got back all memory as expected"));
		
	CleanupStack::PopAndDestroy(3);
	}

/** 
 * Negative Tests to see we can handle dodgy params
 */
void CFlashDataSourceWrapper::DoCmd_ReadMemoryL2_Test_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::ReadMemoryL() Test2"));
	
	//buffer to hold memory
	TInt32 numBytes = 8; //we will dump 8 bytes of memory
	TUint8 memoryToDump = 0xAB;		
	
	RBuf8 mem;
	mem.CreateL(numBytes * sizeof(TUint8));	
	mem.CleanupClosePushL();
	
	for(TInt cnt = numBytes -1; cnt >= 0; cnt--)
		{
		mem.Append(&memoryToDump, sizeof(TUint8));
		}
	
	//buffer to hold log
	TInt dataLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + TRegisterSet::KSCMRegisterSetMaxSize + numBytes + TMemoryDump::KSCMMemDumpMaxSize + TRawData::KSCMRawDataMaxSize;
	
	RBuf8 data;	
	data.CreateL(dataLength);
	data.SetLength(dataLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);		
	
	TUint32 startAdd = 0x10000000;
	TUint64 procId = 100;
	
	TMemoryDump memDump;
	memDump.iStartAddress = startAdd;
	memDump.iPid = procId;
	memDump.iLength = mem.Length();
	TRawData rawData;
	rawData.iLength = mem.Length();	
	rawData.iData.Set(mem.MidTPtr(0));
	
	TInt32 crashSize = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize() + memDump.GetSize() + rawData.GetSize();
	iInfHdr.iLogSize = crashSize;
	iInfHdr.iCrashId = CRASH_ID;
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	memDump.Serialize(writer);
	rawData.Serialize(writer);
	
	INFO_PRINTF1(_L("Writing known memory to flash"));
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));
	
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to analyse crash: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	RBuf8 found;
	found.CreateL(mem.Length());
	found.CleanupClosePushL();
	
	//reading from address not dumped
	TRAP(err, iObject->ReadMemoryL(procId, 0xDEDEADAD, mem.Length(), found));
	if(KErrNotFound != err)
		{
		ERR_PRINTF2(_L("Able to read memory not dumped: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}
	
	//read from a thread not dumped
	TRAP(err, iObject->ReadMemoryL(50, startAdd, mem.Length(), found));
	if(KErrNotFound != err)
		{
		ERR_PRINTF2(_L("able to read memory from thread not dumped: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}
	
	INFO_PRINTF1(_L("Got back all memory as expected"));
		
	CleanupStack::PopAndDestroy(3);
	}

/** 
 * 	Tests CFlashDataSource::GetCodeSegmentsL
 *  Writes known code segments to flash and ensures we can get them back
 *  
 *  Description:
 *  We will pretend we have a process ID of 100. We will also have a thread ID of 101.
 *  We will assign ownership between these two - ie. thread ID 101 exists in process ID 100.
 *  We will write a mini crash log to flash:
 *  <---Crash Header---><---Code Segment Set---><---Code Segment 1---><---Code Segment 2--->
 *  
 *  The code segments will exist for process ID 100. We will then try to read the code segments 
 *  for thread ID 101 and we should retrieve code seg 1 and 2.
 *  
 */
void CFlashDataSourceWrapper::DoCmd_GetCodeSegmentsL1_Test_L()
	{		
	INFO_PRINTF1(_L("Testing CFlashDataSource::GetCodeSegmentsL() - 1st test"));	
	
	TUint64 procId = 100; //arbitrary	
	TInt numSegs = 2;
	
	TCodeSegmentSet segSet;
	segSet.iNumSegs = 2;
	segSet.iPid = procId;
	segSet.iNumSegs = numSegs;
	
	//seg names
	_LIT8(KSeg1, "seg1");
	_LIT8(KSeg2, "cseg2");
	

	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 
						TRegisterSet::KSCMRegisterSetMaxSize + 2 * TCodeSegment::KMaxSegmentNameSize + 
						KMaxCacheSize;
	
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);		
	
	TCodeSegment seg1;
	seg1.iNameLength = KSeg1().Size();
	seg1.iName = KSeg1;
	seg1.iCodeSegType = EExeCodeSegType;
	
	TCodeSegment seg2;
	seg2.iNameLength = KSeg2().Size();
	seg2.iName = KSeg2;
	seg2.iCodeSegType = EExeCodeSegType;
	
	//For code segments to make sense we need to assign thread to process ownership
	TInt64 threadId = 101;
	iObject->AssignOwner(procId, threadId); 
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	iInfHdr.iLogSize = iInfHdr.GetSize() + iContextHdr.GetSize() + iOffsetsHdr.GetSize() + segSet.GetSize() + seg1.GetSize() + seg2.GetSize();		
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	segSet.Serialize(writer);
	seg1.Serialize(writer);
	seg2.Serialize(writer);
	
	INFO_PRINTF1(_L("Writing known Code Segs to flash"));
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));	

	//Now we try get these threads back via GetCodeSegmentsL
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to process crash header: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy();
		return;
		}
	
	RCodeSegPointerList* list = new(ELeave)RCodeSegPointerList;
	TCleanupItem segCleanup(CFlashDataSource::CleanupCodeSegList, (TAny*)list);
	CleanupStack::PushL(segCleanup);
	
	TUint sz = 0;
	TRAP(err, iObject->GetCodeSegmentsL(threadId, *list, sz));
	
	if(list->Count() != numSegs)
		{
		ERR_PRINTF3(_L("Failed to retrieve right amount of code segs. Expected [%d] got [%d]"),numSegs, list->Count());
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	TCodeSegInfo* cs1 = (*list)[0];
	TCodeSegInfo* cs2 = (*list)[1];
	
	INFO_PRINTF1(_L("Retrieveing code segs"));
	if(!cs1 || !cs2)
		{
		ERR_PRINTF1(_L("Failed to retrieve non null threads"));
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	//Check types
	INFO_PRINTF1(_L("Checking code seg types are ok"));
	if(cs1->iType != EExeCodeSegType ||  cs2->iType != EExeCodeSegType)
		{
		ERR_PRINTF1(_L("Failed to retrieve correct code seg types:"));
		ERR_PRINTF3(_L("CS1 expected [%d] but found [%d]"), EExeCodeSegType, cs1->iType);
		ERR_PRINTF3(_L("CS2 expected [%d] but found [%d]"), EExeCodeSegType, cs2->iType);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(2);
		return;
		}
	
	//check names
	RBuf wide;	
	wide.CreateL(KSeg1().Size());
	wide.CleanupClosePushL();	
	wide.Copy(KSeg1);
	
	INFO_PRINTF1(_L("Checking code seg name's are ok"));
	
	TBool passed = (cs1->iName.Compare(wide) == 0) || (cs2->iName.Compare(wide) == 0);
	
	if(!passed)
		{
		ERR_PRINTF4(_L("Wrong name retrieved, expected [%S] but got [%S] and [%S]"), &wide, &cs2->iName, &cs1->iName);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}
	
	if(2 *KSeg2().Size() > wide.Size())
		{
		wide.ReAllocL(2 * KSeg2().Size());
		}
	
	wide.Copy(KSeg2);
	
	INFO_PRINTF1(_L("Checking second code seg name is ok"));
	
	passed = (cs1->iName.Compare(wide) == 0) || (cs2->iName.Compare(wide) == 0);
	
	if(!passed)
		{
		ERR_PRINTF4(_L("Wrong name retrieved, expected [%S] but got [%S] and [%S]"), &wide, &cs2->iName, &cs1->iName);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}
	
	CleanupStack::PopAndDestroy(3);
	
	}

/**
 *  Tests CFlashDataSource::GetCodeSegmentsL
 *  Ensure we recognise corrupt segments (ie. no segment set to describe it)
 */
void CFlashDataSourceWrapper::DoCmd_GetCodeSegmentsL2_Test_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource() - Code Segment Analysis - 2nd test"));
	
	//seg names
	_LIT8(KSeg1, "seg1");
	_LIT8(KSeg2, "cseg2");	

	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 
						TRegisterSet::KSCMRegisterSetMaxSize + 2 * TCodeSegment::KMaxSegmentNameSize + 
						KMaxCacheSize;
	
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);		
	
	TCodeSegment seg1;
	seg1.iNameLength = KSeg1().Size();
	seg1.iName = KSeg1;
	seg1.iCodeSegType = EExeCodeSegType;
	
	TCodeSegment seg2;
	seg2.iNameLength = KSeg2().Size();
	seg2.iName = KSeg2;
	seg2.iCodeSegType = EExeCodeSegType;
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	iInfHdr.iLogSize = iInfHdr.GetSize() + iContextHdr.GetSize() + iOffsetsHdr.GetSize()+ seg1.GetSize() + seg2.GetSize();		
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	seg1.Serialize(writer);
	seg2.Serialize(writer);
	
	INFO_PRINTF1(_L("Writing known corrupt Code Segs to flash"));
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));	

	//Now we try get these threads back via GetCodeSegmentsL
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));	
	if(KErrCorrupt != err)
		{
		ERR_PRINTF2(_L("Failed to recognise corrupt data: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy();
		return;
		}
	
	CleanupStack::PopAndDestroy();
	
	INFO_PRINTF1(_L("Corrupt data recognised"));
	}



/**
 *  Tests CFlashDataSource::GetCodeSegmentsL
 *  Ensure we do not reciev duplicate segments 
 * even after we place then in flash
 */
void CFlashDataSourceWrapper::DoCmd_GetCodeSegmentsL3_Test_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource() - Code Segment Analysis - 3rd test"));
	
	TUint64 procId = 100; //arbitrary	
	const TInt KNumRepeatSegs = 3;
	
	TCodeSegmentSet segSet;
	segSet.iNumSegs = KNumRepeatSegs;
	segSet.iPid = procId;
	
	//seg names
	_LIT8(KSeg1, "seg1");
	
	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 
						TRegisterSet::KSCMRegisterSetMaxSize + 2 * TCodeSegment::KMaxSegmentNameSize + 
						KMaxCacheSize;
	
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);		
	
	TCodeSegment seg1;
	seg1.iNameLength = KSeg1().Size();
	seg1.iName = KSeg1;
	seg1.iCodeSegType = EExeCodeSegType;
	
	
	//For code segments to make sense we need to assign thread to process ownership
	TInt64 threadId = 101;
	iObject->AssignOwner(procId, threadId); 
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	
	
	iInfHdr.iLogSize = 
		iInfHdr.GetSize() + iContextHdr.GetSize() + iOffsetsHdr.GetSize() 
		+ segSet.GetSize() + ( seg1.GetSize() * KNumRepeatSegs);		
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	segSet.Serialize(writer);
	
	// serialize the same segment a number of times
	for(TInt i=0;i<segSet.iNumSegs;i++)
		{
		seg1.Serialize(writer);
		}
	
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));	

	//Now we try get these threads back via GetCodeSegmentsL
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to process crash header: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy();
		return;
		}
	
	RCodeSegPointerList* list = new(ELeave)RCodeSegPointerList;
	TCleanupItem segCleanup(CFlashDataSource::CleanupCodeSegList, (TAny*)list);
	CleanupStack::PushL(segCleanup);
	
	TUint sz = 0;
	TRAP(err, iObject->GetCodeSegmentsL(threadId, *list, sz));
	
	
	// loop through the code seg list check that none are the same
	for(TInt i=0;i<list->Count();i++)
		{
		TCodeSegInfo* csinf1 = (*list)[i];			
		for(TInt j=1;j<list->Count();j++)
			{
			TCodeSegInfo* csinf2 = (*list)[j];
			
			// check not checking same item
			if( csinf1 != csinf2 )
				{
				if(csinf1->iDataRunAddr == csinf2->iDataRunAddr && 
				   csinf1->iDataSize == csinf2->iDataSize )
					{
					ERR_PRINTF1(_L("DUPLICATE code segs 1") );
					SetBlockResult(EFail);
					CleanupStack::PopAndDestroy(2);
					return;
					
					}
				}
			}	
		}
	
	CleanupStack::PopAndDestroy(2);  // data , list
	}

/**
 * Tests we can get ALL expected trace data back
 */
void CFlashDataSourceWrapper::DoCmd_ReadTraceBufferL1_Test_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::ReadTraceBufferL() Test1"));
	
	//buffer to hold trace - will do in 2 parts to simulate the circular buffer
	TInt32 numBytes = 8; //we will dump 8 bytes of trace
	TUint8 traceToDump1 = 0xAB;
	TUint8 traceToDump2 = 0xCD;
	
	RBuf8 trace1;
	trace1.CreateL(numBytes * sizeof(TUint8));	
	trace1.CleanupClosePushL();
		
	for(TInt cnt = numBytes -1; cnt >= 0; cnt--)
		{
		trace1.Append(&traceToDump1, sizeof(TUint8));
		}
	
	RBuf8 trace2;
	trace2.CreateL(numBytes * sizeof(TUint8));	
	trace2.CleanupClosePushL();
	
	for(TInt cnt = numBytes -1; cnt >= 0; cnt--)
		{
		trace2.Append(&traceToDump2, sizeof(TUint8));
		}
	
	//buffer to hold log
	TInt dataLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + 
							TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 
							TRegisterSet::KSCMRegisterSetMaxSize +							 
							TTraceDump::KSCMTraceDumpMaxSize + 
							2 * (TRawData::KSCMRawDataMaxSize + numBytes);
	
	RBuf8 data;	
	data.CreateL(dataLength);
	data.SetLength(dataLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);	
	
	TTraceDump traceDump;
	traceDump.iSizeOfMemory = 2 * numBytes;
	traceDump.iNumberOfParts = 2;
	
	TRawData rawData1;
	rawData1.iLength = trace1.Length();	
	rawData1.iData.Set(trace1.MidTPtr(0));
	
	TRawData rawData2;
	rawData2.iLength = trace2.Length();	
	rawData2.iData.Set(trace2.MidTPtr(0));
	
	TInt32 crashSize = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize() + traceDump.GetSize() + rawData1.GetSize() + rawData2.GetSize();
	iInfHdr.iLogSize = crashSize;
	iInfHdr.iCrashId = CRASH_ID;
	iOffsetsHdr.iTraceOffset = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize();
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	traceDump.Serialize(writer);
	rawData1.Serialize(writer);
	rawData2.Serialize(writer);
	
	INFO_PRINTF1(_L("Writing known trace to flash"));
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));
	
	//Now we try get the trace data back
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));
	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to process crash header: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}
		
	TUint traceBufferAvailable = 0;
	
	TRAP(err, traceBufferAvailable = iObject->GetAvailableTraceSizeL());
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to read trace data size: [%d]"), err);
		SetBlockResult(EFail);		
		CleanupStack::PopAndDestroy(3);
		return;
		}
	
	RBuf8 found;
	found.CreateL(traceBufferAvailable);
	found.CleanupClosePushL();
	
	//reading from address not dumped	
	TRAP(err, iObject->ReadTraceBufferL(found));
	
	RDebug::Printf("Printing trace found");
	for(TInt x = 0; x<found.Length(); x++)
		{			
		RDebug::Printf("[0x%X]", found[x]);
		}
	
	RBuf8 totalTrace;
	totalTrace.CreateL(trace1.Length() + trace2.Length());
	totalTrace.CleanupClosePushL();
	
	totalTrace.Append(trace1);
	totalTrace.Append(trace2);
	
	if(totalTrace.Compare(found) != 0)
		{
		ERR_PRINTF1(_L("Found trace did not match expected trace"));
		SetBlockResult(EFail);		
		CleanupStack::PopAndDestroy(5);
		return;
		}
	
	CleanupStack::PopAndDestroy(5);
	}

void CFlashDataSourceWrapper::DoCmd_CalculateChecksum_TestL()
  	{  	
  	// check sum is commutative - so it should add up to same value
  	// however blocks were requested
  	
	INFO_PRINTF1(_L("DoCmd_CalculateChecksum_TestL called"));
  	
  	TScmChecksum chksm1, chksm2;  	
  
  	iObject->CalculateChecksumL(0, 10, chksm1);
  	iObject->CalculateChecksumL(10, 10, chksm1);  	  	
  	iObject->CalculateChecksumL(0, 20, chksm2);  	
  	
  	SetBlockResult((chksm1 == chksm2) ? EPass : EFail );  	  	
  	}

/**
 * Tests we can get back the correct trace buffer size
 */
void CFlashDataSourceWrapper::DoCmd_TraceDataSizeL_TestL()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource() - Trace Size Test"));

	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 
						TRegisterSet::KSCMRegisterSetMaxSize + 2 * TCodeSegment::KMaxSegmentNameSize + 
						KMaxCacheSize;
	
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);		
	
	TUint memSize = 1243;
	TTraceDump trace;
	trace.iSizeOfMemory = memSize;
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	iInfHdr.iLogSize = iInfHdr.GetSize() + iContextHdr.GetSize() + iOffsetsHdr.GetSize()+ trace.GetSize();		
	iOffsetsHdr.iTraceOffset = iInfHdr.GetSize() + iContextHdr.GetSize() + iOffsetsHdr.GetSize();
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	trace.Serialize(writer);
	
	INFO_PRINTF1(_L("Writing known trace struct to flash"));
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));	

	//Now we try get these threads back via GetCodeSegmentsL
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to analyse crash: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy();
		return;
		}
	
	CleanupStack::PopAndDestroy();
	
	TUint recoveredMemSize = 0;
	TRAP(err, recoveredMemSize = iObject->GetAvailableTraceSizeL());
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to get back trace size: [%d]"), err);
		SetBlockResult(EFail);
		return;
		}
	
	if(recoveredMemSize != memSize)
		{
		ERR_PRINTF3(_L("Failed to get back correct trace size: Got [%d] Expected [%d]"), recoveredMemSize, memSize);
		SetBlockResult(EFail);
		return;
		}
	
	INFO_PRINTF1(_L("Trace size looks good"));
	}

/**
 * Tests we can get back the correct trace buffer size
 */
void CFlashDataSourceWrapper::DoCmd_TraceDataSizeNotFoundL_TestL()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource() - Trace Size Test - Negative"));

	//buffer for data
	TInt bufLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 
						TRegisterSet::KSCMRegisterSetMaxSize + 2 * TCodeSegment::KMaxSegmentNameSize + 
						KMaxCacheSize;
	
	RBuf8 data;	
	data.CreateL(bufLength);
	data.SetLength(bufLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);
	
	iInfHdr.iCrashId = CRASH_ID;	
	iContextHdr.iNumRegisters = 0; //no context
	iInfHdr.iLogSize = iInfHdr.GetSize() + iContextHdr.GetSize() + iOffsetsHdr.GetSize();		
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);	
	
	INFO_PRINTF1(_L("Writing known trace struct to flash"));
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));	

	//Now we try get these threads back via GetCodeSegmentsL
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to analyse crash: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy();
		return;
		}
	
	CleanupStack::PopAndDestroy();
		
	TRAP(err, TUint recoveredMemSize = iObject->GetAvailableTraceSizeL());
	if(KErrNotFound != err)
		{
		ERR_PRINTF2(_L("Failed to get back KErrNone for non available trace size: [%d]"), err);
		SetBlockResult(EFail);
		return;
		}
	
	INFO_PRINTF1(_L("All as expected."));
	}

/**
 * Tests we can get ALL expected trace data back
 */
void CFlashDataSourceWrapper::DoCmd_ReadTraceBufferL2_Test_L()
	{
	INFO_PRINTF1(_L("Testing CFlashDataSource::ReadTraceBufferL() Test2"));
	
	//buffer to hold trace - will do in 2 parts to simulate the circular buffer
	TInt32 numBytes = 8; //we will dump 8 bytes of trace
	TUint8 traceToDump1 = 0xAB;
	TUint8 traceToDump2 = 0xCD;
	
	RBuf8 trace1;
	trace1.CreateL(numBytes * sizeof(TUint8));	
	trace1.CleanupClosePushL();
		
	for(TInt cnt = numBytes -1; cnt >= 0; cnt--)
		{
		trace1.Append(&traceToDump1, sizeof(TUint8));
		}
	
	RBuf8 trace2;
	trace2.CreateL(numBytes * sizeof(TUint8));	
	trace2.CleanupClosePushL();
	
	for(TInt cnt = numBytes -1; cnt >= 0; cnt--)
		{
		trace2.Append(&traceToDump2, sizeof(TUint8));
		}
	
	//buffer to hold log
	TInt dataLength = TCrashInfoHeader::KSCMCrashInfoMaxSize + 
							TCrashOffsetsHeader::KSCMCrashOffsetsMaxSize + 
							TRegisterSet::KSCMRegisterSetMaxSize +							 
							TTraceDump::KSCMTraceDumpMaxSize + 
							2 * (TRawData::KSCMRawDataMaxSize + numBytes);
	
	RBuf8 data;	
	data.CreateL(dataLength);
	data.SetLength(dataLength);
	data.CleanupClosePushL();
	
	TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);	
	
	TTraceDump traceDump;
	traceDump.iSizeOfMemory = 2 * numBytes;
	traceDump.iNumberOfParts = 2;
	
	TRawData rawData1;
	rawData1.iLength = trace1.Length();	
	rawData1.iData.Set(trace1.MidTPtr(0));
	
	TRawData rawData2;
	rawData2.iLength = trace2.Length();	
	rawData2.iData.Set(trace2.MidTPtr(0));
	
	TInt32 crashSize = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize() + traceDump.GetSize() + rawData1.GetSize() + rawData2.GetSize();
	iInfHdr.iLogSize = crashSize;
	iInfHdr.iCrashId = CRASH_ID;
	iOffsetsHdr.iTraceOffset = iInfHdr.GetSize() + iOffsetsHdr.GetSize() + iContextHdr.GetSize();
	
	iInfHdr.Serialize(writer);
	iOffsetsHdr.Serialize(writer);
	iContextHdr.Serialize(writer);
	traceDump.Serialize(writer);
	rawData1.Serialize(writer);
	rawData2.Serialize(writer);
	
	INFO_PRINTF1(_L("Writing known trace to flash"));
	
	TUint32 written = 0;
	User::LeaveIfError(iSecSess.WriteCrashConfig(START_OF_FLASH, data ,written));
	
	//Now we try get the trace data back
	TRAPD(err, iObject->AnalyseCrashL(CRASH_ID));
	
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Failed to process crash header: [%d]"), err);
		SetBlockResult(EFail);
		CleanupStack::PopAndDestroy(3);
		return;
		}
		
	TUint traceBufferAvailable = 0;
	
	TRAP(err, traceBufferAvailable = iObject->GetAvailableTraceSizeL());
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to read trace data size: [%d]"), err);
		SetBlockResult(EFail);		
		CleanupStack::PopAndDestroy(3);
		return;
		}
	
	RBuf8 totalTrace;
	totalTrace.CreateL(trace1.Length() + trace2.Length());
	totalTrace.CleanupClosePushL();
	
	totalTrace.Append(trace1);
	totalTrace.Append(trace2);
	
	//Read 1 byte at a time
	RBuf8 read;
	read.CreateL(1);
	read.CleanupClosePushL();
	
	for(TInt x = 0; x< traceBufferAvailable; x++)
		{
		TRAP(err, iObject->ReadTraceBufferL(read, x));
		if(err != KErrNone)
			{
			ERR_PRINTF2(_L("Unable to read trace data: [%d]"), err);
			SetBlockResult(EFail);		
			CleanupStack::PopAndDestroy(5);
			return;
			}
		
		RDebug::Printf("Found [0x%X] expected [0x%X]", read[0], totalTrace[x]);
		
		if(read[0] != totalTrace[x])
			{
			ERR_PRINTF1(_L("Didnt get back expected trace"));
			SetBlockResult(EFail);		
			CleanupStack::PopAndDestroy(5);
			return;
			}
		}
	
	CleanupStack::PopAndDestroy(5);
	}

//eof


