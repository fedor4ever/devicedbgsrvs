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
// Example CTestStep derived implementation
//



/**
 @file filewriterteststeps.cpp
*/

#include "filewriterteststeps.h"
#include <crashdatasave.h>

TInt getFileWriter(CTestStep* logger, CCrashDataSave **dataSave)
{
	TUid dataSaveUid = { 0x102831e4 };
	TRAPD(err, *dataSave = CCrashDataSave::NewL(dataSaveUid));
	switch(err)
	{
		case KErrNoMemory:
		logger->Logger().LogExtra(((TText8*)__FILE__), __LINE__, ESevrErr, (_L("KErrNoMemory\n")));
		RDebug::Print(_L("KErrNoMemory\n"));
		break;
 
		case KErrNotFound:
		logger->Logger().LogExtra(((TText8*)__FILE__), __LINE__, ESevrErr, (_L("KErrNotFound\n")));
		RDebug::Print(_L("KErrNotFound\n"));
		break;
 		
		case KErrArgument:
		logger->Logger().LogExtra(((TText8*)__FILE__), __LINE__, ESevrErr, (_L("KErrArgument\n")));
 		RDebug::Print(_L("KErrArgument\n"));
 		break;
	
		case KErrPermissionDenied:
		logger->Logger().LogExtra(((TText8*)__FILE__), __LINE__, ESevrErr, (_L("KErrPermissionDenied\n")));
		RDebug::Print(_L("KErrPermissionDenied\n"));
 		break;
	}
	
	return err;
}

CFileWriterTestStep1::~CFileWriterTestStep1()
/**
 * Destructor
 */
	{
	}

CFileWriterTestStep1::CFileWriterTestStep1()
/**
 * Constructor
 */
	{
	// Call base class method to set up the human readable name for logging
	SetTestStepName(KFileWriterTestStep1);
	SetTestStepResult(EPass);
	}

TVerdict CFileWriterTestStep1::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}

TVerdict CFileWriterTestStep1::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 */
	{
    /*
	INFO_PRINTF1(_L("Testing GetFormattingPromptL"));
	RBuf format;
	CleanupClosePushL(format);

	format.Create(255);
	
	CCrashDataSave *writer = 0;
	testBooleanTrueL(getFileWriter(this, &writer) == KErrNone, (TText8*)__FILE__, __LINE__);
	CleanupStack::PushL(writer);
	INFO_PRINTF1(_L("File writer created"));
	
	TRAPD(err, writer->GetFormattingPromptL(format));
	
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
	
	INFO_PRINTF1(_L("Got the format from GetFormattingPromptL"));
	INFO_PRINTF2(_L("The format is: %S"), &format);
	
	_LIT(KPluginDescription, "Symbian Elf Core Dump Writer");
	testBooleanTrue(format == KPluginDescription, (TText8*)__FILE__, __LINE__);
	
	CleanupStack::PopAndDestroy(writer);
	INFO_PRINTF1(_L("File writer destroyed and popped"));

	CleanupStack::PopAndDestroy();
	
    */
	return TestStepResult();
	}

TVerdict CFileWriterTestStep1::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}

//

CFileWriterTestStep2::~CFileWriterTestStep2()
/**
 * Destructor
 */
	{
	}
CFileWriterTestStep2::CFileWriterTestStep2()
/**
 * Constructor
 */
	{
	// Call base class method to set up the human readable name for logging
	SetTestStepName(KFileWriterTestStep2);
	SetTestStepResult(EPass);
	}

TVerdict CFileWriterTestStep2::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}

TVerdict CFileWriterTestStep2::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 */
	{
    /*
	INFO_PRINTF1(_L("Testing FormatL"));

	CCrashDataSave *writer = 0;
	testBooleanTrueL(getFileWriter(this, &writer) == KErrNone, (TText8*)__FILE__, __LINE__);
	CleanupStack::PushL(writer);
	INFO_PRINTF1(_L("File writer created"));
	
	_LIT(KNewFormat, "New format");
	
	TRAPD(err, writer->FormatL(KNewFormat));
	INFO_PRINTF1(_L("Set new format"));
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
	
	RBuf format;
	format.Create(255);
	CleanupClosePushL(format);
	
	TRAP(err, writer->GetFormattingPromptL(format));
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
	testBooleanTrue(format.Compare(KNewFormat) == 0, (TText8*)__FILE__, __LINE__);
	
	CleanupStack::PopAndDestroy();
	CleanupStack::PopAndDestroy(writer);
	
    */
	return TestStepResult();
	}

TVerdict CFileWriterTestStep2::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}
//

CFileWriterTestStep3::~CFileWriterTestStep3()
/**
 * Destructor
 */
	{
	}

CFileWriterTestStep3::CFileWriterTestStep3()
/**
 * Constructor
 */
	{
	// Call base class method to set up the human readable name for logging
	SetTestStepName(KFileWriterTestStep3);
	SetTestStepResult(EPass);
	}

TVerdict CFileWriterTestStep3::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}

TVerdict CFileWriterTestStep3::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 */
	{
	INFO_PRINTF1(_L("Testing OpenL"));
	CCrashDataSave *writer = 0;
	testBooleanTrueL(getFileWriter(this, &writer) == KErrNone, (TText8*)__FILE__, __LINE__);
	CleanupStack::PushL(writer);
	INFO_PRINTF1(_L("File writer created"));
	
	TRAPD(err, writer->OpenL(_L("c:\\dumpfiles\\file1")));
	INFO_PRINTF2(_L("Opened file c:\\dumpfiles\\file1 and the error code back was (should be 0): %d"), err);
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
	TRAP(err, writer->CloseL());

	User::After(500000);
	INFO_PRINTF1(_L("Create 10 more files"));

	for (TInt i = 0; i < 10; i++)
	{
		TBuf<255> filename;
		filename.Format(_L("C:\\dumpfiles\\file%d"), i);
		TRAP(err, writer->OpenL(filename));
		testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
		INFO_PRINTF2(_L("Opened file %d"), i );

		TRAP(err, writer->CloseL() );
		testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
		INFO_PRINTF2(_L("Closed file %d"), i);
	}
	
	CleanupStack::PopAndDestroy(writer);
	return TestStepResult();
	}

TVerdict CFileWriterTestStep3::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}


CFileWriterTestStep4::~CFileWriterTestStep4()
/**
 * Destructor
 */
	{
	}

CFileWriterTestStep4::CFileWriterTestStep4()
/**
 * Constructor
 */
	{
	// Call base class method to set up the human readable name for logging
	SetTestStepName(KFileWriterTestStep4);
	}

TVerdict CFileWriterTestStep4::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}

TVerdict CFileWriterTestStep4::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 */
	{
	INFO_PRINTF1(_L("Testing CloseL"));
	CCrashDataSave *writer = 0;
	testBooleanTrueL(getFileWriter(this, &writer) == KErrNone, (TText8*)__FILE__, __LINE__);
	CleanupStack::PushL(writer);
	INFO_PRINTF1(_L("File writer created"));
	
	
	TRAPD(err, writer->OpenL(_L("c:\\dumpfiles\\file1")));
	INFO_PRINTF2(_L("Opened file file 1 and the error code back was (should be 0): %d"), err);
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
	

	TRAP(err, writer->CloseL());
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
	INFO_PRINTF1(_L("Closed file"));

	CleanupStack::PopAndDestroy(writer);
	return TestStepResult();
	}

TVerdict CFileWriterTestStep4::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}


CFileWriterTestStep5::~CFileWriterTestStep5()
/**
 * Destructor
 */
	{
	}

CFileWriterTestStep5::CFileWriterTestStep5()
/**
 * Constructor
 */
	{
	// Call base class method to set up the human readable name for logging
	SetTestStepName(KFileWriterTestStep5);
	}

TVerdict CFileWriterTestStep5::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}

TVerdict CFileWriterTestStep5::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 */
	{
	INFO_PRINTF1(_L("Testing WriteL"));
	CCrashDataSave *writer = 0;
	testBooleanTrueL(getFileWriter(this, &writer) == KErrNone, (TText8*)__FILE__, __LINE__);
	CleanupStack::PushL(writer);
	INFO_PRINTF1(_L("File writer created"));

	TRAPD(err, writer->OpenL(_L("c:\\dumpfiles\\file1") ));
	INFO_PRINTF2(_L("Opened file file 1 and the error code back was (should be 0): %d"), err);
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);

	RBuf8 buf;
	CleanupClosePushL(buf);
	buf.Create(100);
	buf = _L8("1234567890123456789012345678901234567890123456789012345678901234567890");
	
    TAny* ptr = static_cast<TAny*>(const_cast<TUint8*>(buf.Ptr()));

	TRAP(err, writer->WriteL( 0, ptr, buf.Length()));
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
	INFO_PRINTF1(_L("Wrote 70 bytes"));
	
	TRAP(err, writer->WriteL( 0x100, ptr, buf.Length()));
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
	INFO_PRINTF1(_L("Wrote 70 bytes start at pos 100"));
	
	TRAP(err, writer->WriteL( -1, ptr, buf.Length()));
	testBooleanTrue(err != KErrNone, (TText8*)__FILE__, __LINE__);
	INFO_PRINTF2(_L("Negative test: Tried to write 70 bytes start at -1, leave code %d"), err);
	
	TRAP(err, writer->CloseL());
	testBooleanTrue(err == KErrNone, (TText8*)__FILE__, __LINE__);
	INFO_PRINTF1(_L("Closed file"));
	
	CleanupStack::PopAndDestroy(); // buf
	CleanupStack::PopAndDestroy(writer);
	return TestStepResult();
	}

TVerdict CFileWriterTestStep5::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	return TestStepResult();
	}
