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
 @file tcds_kernelBlockServer.h
 @internalTechnology
*/

#if (!defined __TCDS_kernel_BLOCK_SERVER_H__)
#define __TCDS_kernel_BLOCK_SERVER_H__
#include <TestServer2.h>
#include "tcds_kernelBlockController.h"

class Ctcds_kernelBlockServer : public CTestServer2
	{
public:
	static Ctcds_kernelBlockServer* NewL();
	// Base class pure virtual override
	virtual CTestBlockController* CreateTestBlock();

// Please Add/modify your class members
private:
	};

#endif
