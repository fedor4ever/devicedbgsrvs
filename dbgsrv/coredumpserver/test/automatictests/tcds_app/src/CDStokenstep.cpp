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
 @file tokenstep.cpp
 @internalTechnology
*/
#include "CDStokenstep.h"
#include "tcoredumpserversuitedefs.h"
#include "tcoredumpserversuiteserver.h"

CTokenStep::~CTokenStep()
/**
 * Destructor
 */
	{
	}

CTokenStep::CTokenStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KTokenStep);
	}

TVerdict CTokenStep::doTestStepPreambleL()
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


TVerdict CTokenStep::doTestStepL()
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
			ERR_PRINTF2(_L("Error %d from CTokenStep->ClientAppL()"), ret);
			}
		
		__UHEAP_MARKEND;

		}
	return TestStepResult();  

	}



TVerdict CTokenStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	
	iSess.Disconnect();
	return EPass;	
	}


void CTokenStep::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1(_L("Starting Lists Test Suite") );
    CheckAttachL();
	}


void CTokenStep::CheckAttachL()
/**
 * @return void
 * This tests we can succesfully list the processes and that the returned values make sense
 */
	{
	INFO_PRINTF1(_L("Token Test Suite: Attach") );

	if (TestStepResult()==EPass)
		{

	    INFO_PRINTF1(_L("Attach to crashapp.exe") );
        TRAPD(err, iSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, ETrue) );
        if(err != KErrNone)
            {
            INFO_PRINTF2(_L("attach iSess.ObservationRequestL(z:\\sys\\bin\\crashapp.exe) failed:%d\n"), err);
            SetTestStepResult(EFail);
            User::Leave( err );
            }

	    INFO_PRINTF1(_L("Detach from crashapp.exe") );
        TRAP(err, iSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, EFalse) );
        if(err != KErrNone)
            {
            INFO_PRINTF2(_L("detach iSess.ObservationRequestL(z:\\sys\\bin\\crashapp.exe) failed:%d\n"), err);
            SetTestStepResult(EFail);
            User::Leave( err );
            }

	    INFO_PRINTF1(_L("Attach to crashappoem.exe") );
        TRAPD(oem, iSess.ObservationRequestL( KCrashAppFileNameNonDebugBit, KCrashAppFileNameNonDebugBit, ETrue) );
        if(oem != KErrNone)
            {
            INFO_PRINTF2(_L("attach iSess.ObservationRequestL(z:\\sys\\bin\\crashappoem.exe) failed:%d\n"), oem);
            SetTestStepResult(EFail);
            User::Leave( oem );
            }

	    INFO_PRINTF1(_L("Detach from crashappoem.exe") );
        TRAP(oem, iSess.ObservationRequestL( KCrashAppFileNameNonDebugBit, KCrashAppFileNameNonDebugBit, EFalse) );
        if(oem != KErrNone)
            {
            INFO_PRINTF2(_L("detach iSess.ObservationRequestL(z:\\sys\\bin\\crashappoem.exe) failed:%d\n"), oem);
            SetTestStepResult(EFail);
            User::Leave( oem );
            }
		}
	}
