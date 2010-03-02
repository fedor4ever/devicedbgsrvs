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
#if (!defined __SELFPLUGINS_STEP_H__)
#define __SELFPLUGINS_STEP_H__
#include <TestExecuteStepBase.h>
#include "tcoredumpserversuitestepbase.h"
#include "tcoredumpcommon.h"

class CSELFPluginStep : public CTe_coredumpserverSuiteStepBase
	{
    enum {SelfMultipleTries = 11};
public:
	CSELFPluginStep();
	~CSELFPluginStep();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepL();
	virtual TVerdict doTestStepPostambleL();

// Please add/modify your class members here:
private:

	RCoreDumpSession iSess;
	
	void ClientAppL();
	void LoadPluginL();
	void UnloadPluginL();
	void UnloadPluginL(const TInt aIndex);
	void LoadMultiplePluginsL(const TInt aCount);
	void UnloadMultiplePluginsL(const TInt aCount);
	};

_LIT(KSELFPluginStep,"SELFPluginStep");

#endif
