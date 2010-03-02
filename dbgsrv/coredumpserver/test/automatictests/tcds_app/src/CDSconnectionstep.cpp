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
 @file DEXECPlugin.cpp
 @internalTechnology
*/
#include "SELFPluginStep.h"
#include "tcoredumpserversuitedefs.h"
#include "CDSconnectionstep.h"

CCDSConnectionsStep::~CCDSConnectionsStep()
/**
 * Destructor
 */
	{
	}

CCDSConnectionsStep::CCDSConnectionsStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KCDSConnectionsStep);
	}

TVerdict CCDSConnectionsStep::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}

TVerdict CCDSConnectionsStep::doTestStepL()
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
			ERR_PRINTF2(_L("Error %d from CCDSConnectionsStep->ClientAppL()"), ret);
			}
		
		__UHEAP_MARKEND;

		}
	return TestStepResult();  
	}

void CCDSConnectionsStep::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1(_L("Starting CSDConnections Plugin Test Suite") );
	TestConnectionsL();
	}

void CCDSConnectionsStep::TestConnectionsL()
	{
	
	if (TestStepResult()==EPass)
		{
		RCoreDumpSession cdsSess;
		
		//Try connecting
		TInt ret = KErrNone;
		ret = cdsSess.Connect(); 
		CleanupClosePushL(cdsSess);
		
		if(ret != KErrNone)
			{
			SetTestStepResult( EFail );
			ERR_PRINTF2(_L("Error %d from cdsSess->Connect()"), ret);
			return;
			}
		INFO_PRINTF1(_L("Connected successfully"));
		
		//close the connection
		cdsSess.Disconnect();
		INFO_PRINTF1(_L("Disconnected successfully"));
		
		//Try opening another one straight away and using it...
		ret = cdsSess.Connect();
		if(ret != KErrNone)
			{
			SetTestStepResult( EFail );
			ERR_PRINTF2(_L("Error %d from cdsSess->Connect() the second time (following a connection just being closed)"), ret);
			return;
			}
		INFO_PRINTF1(_L("Connected successfully"));

        //now try to attach/detach
	    INFO_PRINTF1(_L("Attach to crashapp.exe") );
        TRAPD(err, cdsSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, ETrue) );
        if(err != KErrNone)
            {
            INFO_PRINTF2(_L("attach iSess.ObservationRequestL(z:\\sys\\bin\\crashapp.exe) failed:%d\n"), err);
            SetTestStepResult(EFail);
            User::Leave( err );
            }

	    INFO_PRINTF1(_L("Detach from crashapp.exe") );
        TRAP(err, cdsSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, EFalse) );
        if(err != KErrNone)
            {
            INFO_PRINTF2(_L("detach iSess.ObservationRequestL(z:\\sys\\bin\\crashapp.exe) failed:%d\n"), err);
            SetTestStepResult(EFail);
            User::Leave( err );
            }

	    INFO_PRINTF1(_L("Attach to crashappoem.exe") );
        TRAPD(oem, cdsSess.ObservationRequestL( KCrashAppFileNameNonDebugBit, KCrashAppFileNameNonDebugBit, ETrue) );
        if(oem != KErrPermissionDenied)
            {
            INFO_PRINTF2(_L("attach iSess.ObservationRequestL(z:\\sys\\bin\\crashappoem.exe) failed:%d\n"), oem);
            SetTestStepResult(EFail);
            User::Leave( oem );
            }

	    INFO_PRINTF1(_L("Detach from crashappoem.exe") );
        TRAP(oem, cdsSess.ObservationRequestL( KCrashAppFileNameNonDebugBit, KCrashAppFileNameNonDebugBit, EFalse) );
        if(oem != KErrNotFound)
            {
            INFO_PRINTF2(_L("detach iSess.ObservationRequestL(z:\\sys\\bin\\crashappoem.exe) failed:%d\n"), oem);
            SetTestStepResult(EFail);
            User::Leave( oem );
            }
		
		CleanupStack::PopAndDestroy(&cdsSess);
        }
	}

TVerdict CCDSConnectionsStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{	
	return EPass;	
	}



