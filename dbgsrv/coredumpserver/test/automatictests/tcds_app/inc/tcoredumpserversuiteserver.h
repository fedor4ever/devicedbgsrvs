/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @file Te_coredumpserverSuiteServer.h
 @internalTechnology
*/

#if (!defined __TE_COREDUMPSERVER_SERVER_H__)
#define __TE_COREDUMPSERVER_SERVER_H__
#include <TestExecuteServerBase.h>
#include <BADESCA.H>

class CTe_coredumpserverSuite : public CTestServer
	{
public:
	static CTe_coredumpserverSuite* NewL();
	// Base class pure virtual override
	virtual CTestStep* CreateTestStep(const TDesC& aStepName);
	static void CleanupProcessList(TAny *aArray);
	static void CleanupThreadList(TAny *aArray);
	static void CleanupPluginList(TAny *aArray);
	static void CleanupBufferArray(TAny *aArray);
	static CDir* DoesFileExistL(const TDesC& aExpectedFile, TFindFile &aFileFinder);
	static TBool FileStartsWith(const TDesC& aRoot, const TDesC& aFilename );

// Please Add/modify your class members
private:
	};

#endif
