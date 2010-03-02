/**
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/



/**
 @file ParametersStep.h
 @internalTechnology
*/
#if (!defined __PARAMETERS_STEP_H__)
#define __PARAMETERS_STEP_H__
#include <TestExecuteStepBase.h>
#include "tcoredumpserversuitestepbase.h"
#include "tcoredumpserversuiteserver.h"
#include "optionconfig.h"
#include "tcoredumpcommon.h"
#include <rm_debug_api.h>

using namespace Debug;

_LIT(KParametersStep,"ParametersStep");

const TUid KCrashAppUid = { 0x102831E5 };

class CParametersStep : public CTe_coredumpserverSuiteStepBase
	{
public:
	CParametersStep();
	~CParametersStep();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepL();
	virtual TVerdict doTestStepPostambleL();

// Please add/modify your class members here:
private:

	RCoreDumpSession iSess;
	RSecuritySvrSession iSecSess;
	RFs iFs;

	void ClientAppL();	
	
	//Tests
	void CrashChildThreadAndTestPostProcL();
	void CrashMainThreadAndTestPostProcL();
	void CrashChildThreadAndTestPreProcL();
	
	//Utilities
	void KillProcessAfterCrashL();	
	void GenerateAndHandleCrashL(const TDesC& aCrashAppParameters, 
								 const TUint& aCDSPostProcParam,
								 const TUint& aCDSPreProcParam);

	void DoConfigureL(const TUint32& aIndex, 
	 	     		  const TUint32& aUID, 
	 	     		  const COptionConfig::TParameterSource& aSource, 
	 	     		  const COptionConfig::TOptionType& aType, 
	 	     		  const TDesC& aPrompt, 
	 	     		  const TUint32& aNumOptions,
	 	     		  const TDesC& aOptions,
	 	     		  const TInt32& aVal, 
	 	     		  const TDesC& aStrValue );	
	void MonitorProgressL();
	TProcessId IsCrashAppAliveL();
	void KillCrashAppL(const TDesC& aProcessName);
	
	void ResumeProcessL(TUint64 aPid);
	TInt GetNumberOfProccessesL();
	TInt GetNumberThreadsL();
	
	TBool IsCrashAppMainThreadRunningL();	
	
	};



#endif
