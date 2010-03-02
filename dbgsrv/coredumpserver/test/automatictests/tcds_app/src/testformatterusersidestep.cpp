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
#include "testformatterusersidestep.h"
#include "tcoredumpserversuitedefs.h"
#include "tcoredumpserversuiteserver.h"
#include "optionconfig.h"

CTestFormatterUserSideStep::~CTestFormatterUserSideStep()
/**
 * Destructor
 */
	{
	}

CTestFormatterUserSideStep::CTestFormatterUserSideStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KTestFormatterUserSideStep);
	}

TVerdict CTestFormatterUserSideStep::doTestStepPreambleL()
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

	return TestStepResult();
	}

TVerdict CTestFormatterUserSideStep::doTestStepL()
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
			INFO_PRINTF2(_L("Error %d from CTestFormatterUserSideStep->ClientAppL()"), ret);
			}
		
		__UHEAP_MARKEND;

		}
	return TestStepResult();  
	}

void CTestFormatterUserSideStep::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1(_L("Starting TestFormatterUserSide Plugin Test Suite") );

	}

TVerdict CTestFormatterUserSideStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	iSess.Disconnect();
	return EPass;	
	}

