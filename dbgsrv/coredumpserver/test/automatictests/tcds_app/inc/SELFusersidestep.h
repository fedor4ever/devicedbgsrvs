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
 @file SELFStep.h
 @internalTechnology
*/
#if (!defined __SELF_STEP_H__)
#define __SELF_STEP_H__
#include <TestExecuteStepBase.h>


#include "tcoredumpserversuitestepbase.h"
#include "tcoredumpcommon.h"

_LIT(KSELFUserSideStep,"SELFUserSideStep");

class CSELFUserSideStep : public CTe_coredumpserverSuiteStepBase
	{
public:
	CSELFUserSideStep();
	~CSELFUserSideStep();
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
	void LoadSELFFormatterL();
    void LoadFileWriterL();
    void BindPluginsL(const TInt aCoung);

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



#endif
