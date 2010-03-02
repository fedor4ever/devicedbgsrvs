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
 @file DEXECStep.h
 @internalTechnology
*/
#if (!defined __DEXEC_STEP_H__)
#define __DEXEC_STEP_H__
#include <TestExecuteStepBase.h>
#include "tcoredumpserversuitestepbase.h"
#include "tcoredumpcommon.h"

class CDEXECUserSideStep : public CTe_coredumpserverSuiteStepBase
	{
public:
	CDEXECUserSideStep();
	~CDEXECUserSideStep();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepL();
	virtual TVerdict doTestStepPostambleL();

// Please add/modify your class members here:
private:


	RCoreDumpSession iSess;
	RFs iFs;
	
	void ClientAppL();
	
	//Tests
	void HandleCrashL();
	
	
	//Utility functions
	void LoadDEXECFormatterL();
    void LoadFileWriterL();
    void BindPluginsL(const TInt aCount);
	
	void MonitorProgressL();
	void DoConfigureL( const TUint32& aIndex, 
					  const TUint32& aUID, 
					  const COptionConfig::TParameterSource& aSource, 
					  const COptionConfig::TOptionType& aType, 
					  const TDesC& aPrompt, 
					  const TUint32& aNumOptions,
					  const TDesC& aOptions,
					  const TInt32& aVal, 
					  const TDesC& aStrValue,
					  const TUint aInstance );

    void CheckParametersL();
	};

_LIT(KDEXECUserSideStep,"DEXECUserSideStep");

#endif
