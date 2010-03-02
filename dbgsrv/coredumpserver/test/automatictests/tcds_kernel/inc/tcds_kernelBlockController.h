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
 @file tcds_kernelBlockController.h
 @internalTechnology
*/

#ifndef __TCDS_kernel_BLOCK_CONTROLLER_H__
#define __TCDS_kernel_BLOCK_CONTROLLER_H__
#include <TestBlockController.h>

//wrapper includes here
#include "t_crash_board.h"
#include "t_process_crash.h"
#include "t_coredumpserver.h"
#include "t_coredumpmonitor.h"
#include "t_scm_config.h"
#include "t_self.h"

class Ctcds_kernelBlockController : public CTestBlockController
	{
public:
	static Ctcds_kernelBlockController* NewL();
	virtual CDataWrapper* CreateDataL(const TDesC& aData);
	};

#endif
