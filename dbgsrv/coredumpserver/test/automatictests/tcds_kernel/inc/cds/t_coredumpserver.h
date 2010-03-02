// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

/**
 @file
 @internalTechnology
*/

#ifndef __COREDUMP_SERVER_H__
#define __COREDUMP_SERVER_H__

#include <datawrapper.h>
#include <e32base.h>
#include <rm_debug_api.h>
#include <coredumpinterface.h>
#include <coredumpserverapi.h>

#include "t_common_defs.h"

/**
 * This class is responsible for the core dump server tests relating to system crash,
 * and specifically, the flash data source
 */
class CCoreDumpServerWrapper : public CDataWrapper
	{
	public:

		~CCoreDumpServerWrapper();

		static CCoreDumpServerWrapper* NewL();

		virtual TBool DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);
		virtual TAny* GetObject() { }
		virtual void SetObjectL(TAny* aObject);
		static void CleanupCrashList(TAny *aArray);

	protected:
		void ConstructL();

	private:

		CCoreDumpServerWrapper();

		void PrepareTestL();
		void TestDataSourceReturnsSystemCrashDataL();
		void TestDataSourceReturnsTraceDataL();
		void TestUntrustedServerAccessL();

	private:

		RCoreDumpSession iCoreDumpSession;
		RSecuritySvrSession iSecSess;

	};
#endif __COREDUMP_SERVER_H__

//eof
