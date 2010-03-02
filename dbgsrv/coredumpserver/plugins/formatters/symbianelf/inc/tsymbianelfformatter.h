// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @released
*/

#ifndef T_SYMBIAN_ELF_FORMATTER_H__
#define T_SYMBIAN_ELF_FORMATTER_H__
#include <e32std.h>
#include <e32test.h>
#include <crashdata.h>
#include "testcrashdatasource.h"
#include "testcrashdatasave.h"
#include "symbianelfstringinfo.h"

class CSymbianElfFormatter;

const TUid KSymbianElfFormatterUid = { 0x10282fe3 };
const TUint MaxCrashDataSize = 4096;
const TUint MaxPluginDescription = 63;

_LIT(KTestTitle, "SYMBIAN_ELF_FORMATTER_TEST");
_LIT(KTestName, "test suite: ");

class CSymbianElfFormatterTest : public CBase
{
public:
       enum TSuite {
                    ENone,
                    EConfigData,
                    EFindImplementation,
                    EConfigParams,
                    EGetDescription,
                    EWriteSymbianInfo,
                    EWriteCodeSegments,
                    EWriteDataSegments,
                    EWriteThreadsInfo,
                    EWriteProcessInfo,
                    EWriteExecutableInfo,
                    EWriteRegistersInfo,
                    ECrash
                    };

	static CSymbianElfFormatterTest* NewL(RTest &aTest);
	static CSymbianElfFormatterTest* NewLC(RTest &aTest);
    void ConstructL();
	CSymbianElfFormatterTest(RTest &aTest);
	~CSymbianElfFormatterTest();
    void TestSuiteL(TSuite aSuite);

private:
    static void CleanupEComArray(TAny* aArray);
	RTest &itest;
    CTestCrashDataSource	* server;
    CSymbianElfFormatter	* formatter;
	CStringInfoTable		* iStrInfoTbl;
    CTestDataSave			* writer;
	TCrashInfo				iCrashInfo;
};

#endif // T_SYMBIAN_ELF_FORMATTER_H__
