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
 @file tokenstep.h
 @internalTechnology
*/
#if (!defined __TOKEN_STEP_H__)
#define __TOKEN_STEP_H__
#include <TestExecuteStepBase.h>
#include "tcoredumpserversuitestepbase.h"
#include "tcoredumpcommon.h"


class CTokenStep : public CTe_coredumpserverSuiteStepBase
	{
public:
	CTokenStep();
	~CTokenStep();
	
	void ClientAppL();
	
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepL();
	virtual TVerdict doTestStepPostambleL();

// Please add/modify your class members here:
private:

	RCoreDumpSession iSess;

	void CheckAttachL();
	
	};

_LIT(KTokenStep,"TokenStep");

#endif
