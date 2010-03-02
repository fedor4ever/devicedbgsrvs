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
 @file DEXECPluginStep.h
 @internalTechnology
*/
#if (!defined __DEXECPLUGINS_STEP_H__)
#define __DEXECPLUGINS_STEP_H__
#include <TestExecuteStepBase.h>
#include "tcoredumpserversuitestepbase.h"
#include "tcoredumpcommon.h"

class CDEXECPluginStep : public CTe_coredumpserverSuiteStepBase
	{
    enum { DexcMultipleTries = 11 };
public:
	CDEXECPluginStep();
	~CDEXECPluginStep();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepL();
	virtual TVerdict doTestStepPostambleL();

// Please add/modify your class members here:
private:

	RCoreDumpSession iSess;
	
	void ClientAppL();
	void LoadPluginL();
    void UnloadPluginL();
    void UnloadPluginL(const TInt aCounter);
	void LoadMultiplePluginsL(const TInt aCounter);
	void UnloadMultiplePluginsL(const TInt aCounter);
	};

_LIT(KDEXECPluginStep,"DEXECPluginStep");

#endif
