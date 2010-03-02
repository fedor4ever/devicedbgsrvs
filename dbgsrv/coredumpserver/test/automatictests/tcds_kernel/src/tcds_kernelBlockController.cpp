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
 @file tcds_unitBlockController.cpp
 @internalTechnology
*/

#include "tcds_kernelBlockController.h"

_LIT(KCrashBoardWrapper, "CCrashBoardWrapper");
_LIT(KProcessCrashWrapper, "CProcessCrashWrapper");
_LIT(KCoreDumpMonitorWrapper, "CCoreDumpMonitor");
_LIT(KCoreDumpServerWrapper, "CCoreDumpServer");
_LIT(KSCMConfiguration, "CSCMConfig");
_LIT(KCSelfWrapper, "CSymbianElfWrapper");

/**
 * @return - Instance of the test block controller
 */
Ctcds_kernelBlockController* Ctcds_kernelBlockController::NewL()
	{
	Ctcds_kernelBlockController* block = new (ELeave) Ctcds_kernelBlockController();
	return block;
	}

/**
 * Called to create test wrapper. The argument is the wrapper to create. This argument comes
 * from the script file
 * @param aData Wrapper to create
 * @return TestWrapper
 */
CDataWrapper* Ctcds_kernelBlockController::CreateDataL(const TDesC& aData)
	{
	CDataWrapper* wrapper = NULL;
	if (KCrashBoardWrapper() == aData)
		{
		wrapper = CCrashBoardWrapper::NewL();
		}
	else if(KProcessCrashWrapper() == aData)
		{
		wrapper = CProcessCrashWrapper::NewL();
		}
	else if(KCoreDumpServerWrapper() == aData)
		{
		wrapper = CCoreDumpServerWrapper::NewL();
		}
	else if(KCoreDumpMonitorWrapper() == aData)
		{
		wrapper = CCoreDumpMonitorWrapper::NewL();
		}
	else if(KSCMConfiguration() == aData)
		{
		wrapper = CSCMConfigurationWrapper::NewL();
		}
	else if(KCSelfWrapper() == aData)
		{
		wrapper = CSymbianElfWrapper::NewL();
		}
	else
		{
		_LIT(KPanic, "%S");
		TBuf<128> buf;
		buf.Format(KPanic, &aData);
		User::Panic(buf, KErrNotFound);
		}

	return wrapper;
	}
