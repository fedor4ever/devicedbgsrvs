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

#include <e32base.h>
#include <e32cons.h>
#include <ecom.h>
#include "symbianelfstringinfo.h"
#include "tsymbianelfstringinfo.h"

CSymbianStringInfoTest::CSymbianStringInfoTest(const TDesC& aTitle) : iTest(aTitle)
{
	iTest.SetLogged(ETrue);
	iTest.Title();
	iTest.Start(_L("SymbianStringInfo - start environment"));
}

CSymbianStringInfoTest* CSymbianStringInfoTest::NewL(const TDesC& aTitle)
{
	CSymbianStringInfoTest* self = new(ELeave) CSymbianStringInfoTest(aTitle);
	return self;
}

CSymbianStringInfoTest::~CSymbianStringInfoTest()
{
	iTest.Close();
}

void CSymbianStringInfoTest::TestSymbianStringInfoL()
{
	CStringInfoTable *strTbl = CStringInfoTable::NewL();
	CleanupStack::PushL(strTbl);
	iTest.Next(_L("GetSize"));
	TUint size = strTbl->GetSize();
	iTest(size>0, __LINE__);
	
	iTest.Next(_L("GetIndex on predefined string \"CORE.SYMBIAN.PROCESS\""));
	TUint index = strTbl->GetIndex(_L8("CORE.SYMBIAN.PROCESS"));
	iTest(index == 34, __LINE__);

	iTest.Next(_L("GetIndex on empty string"));
	index = strTbl->GetIndex(_L8(""));
	iTest(index == 0, __LINE__);

	iTest.Next(_L("AddStringL \"TEST.STRING\""));
	_LIT8(KTestString, "TEST.STRING");
	TRAPD(err, strTbl->AddStringL(KTestString));
	iTest(err == KErrNone, __LINE__);
	
	iTest.Next(_L("GetIndex on newly added string"));
	index = strTbl->GetIndex(KTestString);
	iTest(index > 0, __LINE__);

	iTest.Next(_L("GetAllStrings: Check that first position is 0"));
	TText8* ptr = strTbl->GetAllStrings();
	iTest(ptr[0] == 0, __LINE__);
	
	iTest.Next(_L("GetAllStrings: Check that last position is 0"));
	size = strTbl->GetSize();
	iTest(ptr[size-1] == 0, __LINE__);
	
	iTest.Next(_L("GetAllStrings: Check that next last position is 'G'"));
	iTest(ptr[size-2] == 'G', __LINE__);
	
	iTest.End();
	CleanupStack::PopAndDestroy(strTbl);
}

static void RunTestsL()
{
	CSymbianStringInfoTest* test = CSymbianStringInfoTest::NewL(_L("SymbianStringInfo - basic tests"));
	CleanupStack::PushL(test);

	test->TestSymbianStringInfoL();

	CleanupStack::PopAndDestroy(test);
}

TInt E32Main()
{
	__UHEAP_MARK;
	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	
	TRAPD(error, RunTestsL());
	
	__ASSERT_ALWAYS(!error, User::Panic(_L("RunTestsL was trapped with error: "), error));
	delete cleanupStack;
	__UHEAP_MARKEND;
	return 0;
}

