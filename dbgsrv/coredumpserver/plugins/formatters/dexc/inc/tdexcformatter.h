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

#ifndef T_D_EXC_FORMATTER_H
#define T_D_EXC_FORMATTER_H
#include <e32std.h>
#include <e32test.h>
#include <crashdata.h>
#include "testcrashdatasource.h"
#include "testcrashdatasave.h"

const TUid KDexcFormatterUid = { 0x102832c5 };
class CDexcFormatter;

const TUint MaxCrashDataSize = 4096;
const TUint MaxPluginDescription = 63;

_LIT(KTestTitle, "D_EXC_FORMATTER_TEST");
_LIT(KTestName, "test suite: ");

class CDexcFormatterTest : public CBase
{
public:
        enum TSuite {
                    ENone,
                    EConfigData,
                    EFindImplementation,
                    EConfigParams,
                    EGetDescription,
                    EDumpLog,
                    EDumpPanicInfo,
                    EDumpExcInfo,
                    EDumpRegisters,
                    EDumpCodeSegs,
                    EDumpStack,
                    ECrash
                    };

        static CDexcFormatterTest * NewL(RTest &aTest);
        static CDexcFormatterTest * NewLC(RTest &aTest);
        void ConstructL();
        CDexcFormatterTest(RTest &aTest);
        ~CDexcFormatterTest();
        void TestSuiteL(TSuite aSuite) const;
private:
        static void CleanupEComArray(TAny* aArray);
        RTest &itest;
        CDexcFormatter *formatter;
        CTestCrashDataSource *server;
        CTestDataSave *writer;
};

#endif
