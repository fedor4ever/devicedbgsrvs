// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
//  Server header for unit test of sELFlib

/**
 * @file tcds_unit_sELFlib_blockserver.h
 * @internalTechnology
 */

#ifndef __TCDS_UNIT_SELFLIB_BLOCK_SERVER_H__
#define __TCDS_UNIT_SELFLIB_BLOCK_SERVER_H__

#include <TestBlockController.h>
#include <TestServer2.h>

#include "CSELFLibWrapper.h"

_LIT(KCSELFLibWrapper, "CSELFLibWrapper");

/**
 * This is the test test block server for the sELF lib tests
 */
class CSELFLibBlockServer : public CTestServer2
	{
	public:
		static CSELFLibBlockServer* NewL();
		virtual CTestBlockController* CreateTestBlock();
	};

/**
 * This is the test block controller for the sELFlib tests
 */
class CSELFLibBlockController : public CTestBlockController
	{
	public:
		static CSELFLibBlockController* NewL();
		virtual CDataWrapper* CreateDataL(const TDesC& aData);
	};

#endif
