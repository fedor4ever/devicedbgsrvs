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
 @file CServerDataSourceWrapper.cpp
 @internalTechnology 
*/

#include <d32btrace.h>

#include "CServerDataSourceWrapper.h"

using namespace Debug;

//Names of functions to be tested - referenced from script file
_LIT(KGetAvailableTraceSizeL, "GetAvailableTraceSizeL");

/**
 * Constructor for test wrapper
 */
CServerDataSourceWrapper::CServerDataSourceWrapper()
	:iObject(NULL)
	{
	}

/**
 * Destructor
 */
CServerDataSourceWrapper::~CServerDataSourceWrapper()
	{
	}

/**
 * Two phase constructor for CFlashDataSourceWrapper
 * @return CFlashDataSourceWrapper object
 * @leave
 */
CServerDataSourceWrapper* CServerDataSourceWrapper::NewL()
	{
	CServerDataSourceWrapper* ret = new (ELeave) CServerDataSourceWrapper();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CServerDataSourceWrapper::ConstructL()
	{
	
	}

/** 
 * Assign the object
 *  
 * @param aObject TAny* to the object to test
 * @leave
 */
void CServerDataSourceWrapper::SetObjectL(TAny* aObject)
	{
	delete iObject;
	iObject = NULL;
	iObject = static_cast<CServerCrashDataSource*> (aObject);	
	}

/**
 * Runs a test preamble
 */
void CServerDataSourceWrapper::PrepareTestL()
	{
	SetBlockResult(EPass);		
	
	INFO_PRINTF1(_L("Connecting to DSS"));
	//get a session to the security server
	User::LeaveIfError(iSecSess.Connect(dssVersion));
	
	INFO_PRINTF1(_L("Creating data source"));
	
	//Hackage: TEF doesnt support preamble/postamble and destructs iObject each time. Until they sort it out
	//or document how it is done the hackage shall have to continue. Oh for JUnit....
	delete iObject;		//to be sure
	iObject = CServerCrashDataSource::NewL(iSecSess);
	//end of hackage
	
	INFO_PRINTF1(_L("Ready to start test"));
	}

/**
 * Process command to see what test to run 
 */
TBool CServerDataSourceWrapper::DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
	{
	__UHEAP_MARK;	
	
	PrepareTestL();
	
	if (KGetAvailableTraceSizeL() == aCommand)
		{
		RDebug::Printf("Looking at CServerDataSource::GetAvailableTraceSizeL()");
		TRAPD(err , DoCmdGetAvailableTraceSizeL_TestL());	
		if(BlockResult() != EPass || KErrNone != err)
			{
			RDebug::Printf("\tFailed!");
			INFO_PRINTF2(_L("Failed with err = [%d]"), err);
			SetBlockResult(EFail);
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

void CServerDataSourceWrapper::DoCmdGetAvailableTraceSizeL_TestL()
	{	
	//Step 1: Empty trace buffer
	INFO_PRINTF1(_L("Emptying the trace buffer"));
	RBTrace trace;
	trace.Open();	
	trace.Empty();
	trace.Close();
	
	//step 1: Put our data in
	INFO_PRINTF1(_L("Generating trace data"));
	RProcess crashapp;
	User::LeaveIfError(crashapp.Create(KCrashAppFileName, _L("-t -d1 -c13")));
	
	TRequestStatus stat;
	crashapp.Rendezvous(stat);
	crashapp.Resume();
	
	User::WaitForRequest(stat);	
	
	crashapp.Close();
	
	//step 2: Read it
	INFO_PRINTF1(_L("Getting size of trace data"));
	
	TUint bytesFound = iObject->GetAvailableTraceSizeL();
	
	if(bytesFound != KTraceSizeFromCrashApp)
		{
		ERR_PRINTF3(_L("Read the wrong amount of trace. Expected [0x%X] bytes but got [0x%X] bytes"), KTraceSizeFromCrashApp, bytesFound);
		SetBlockResult(EFail);
		return;
		}		
	}

//eof


