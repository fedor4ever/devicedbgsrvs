/**
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @file tcds_unitBlockController.h
 @internalTechnology
*/

#if (!defined __TCDS_UNIT_BLOCK_CONTROLLER_H__)
#define __TCDS_UNIT_BLOCK_CONTROLLER_H__
#include <TestBlockController.h>
#include "CFlashDataSourceWrapper.h"
#include "CServerDataSourceWrapper.h"


class Ctcds_unitBlockController : public CTestBlockController
	{
public:
	static Ctcds_unitBlockController* NewL();
	// Base class pure virtual override
	virtual CDataWrapper* CreateDataL(const TDesC& aData);

// Please Add/modify your class members
private:
	};

#endif
