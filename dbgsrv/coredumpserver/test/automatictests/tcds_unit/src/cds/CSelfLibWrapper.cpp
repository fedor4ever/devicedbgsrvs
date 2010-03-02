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
//  Test wrapper for unit tests of the sELF lib

/**
 * @file CSELFLibWrapper.cpp
 * @internalTechnology
 */

#include <f32file.h>
#include <bautils.h>

#include "CSELFLibWrapper.h"

CSELFLibWrapper::CSELFLibWrapper()
	{}

CSELFLibWrapper::~CSELFLibWrapper()
	{
	iFs.Close();
	}

/**
 * Two phase constructor for CFlashDataSourceWrapper
 * @return CFlashDataSourceWrapper object
 * @leave
 */
CSELFLibWrapper* CSELFLibWrapper::NewL()
	{
	CSELFLibWrapper* ret = new (ELeave) CSELFLibWrapper();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CSELFLibWrapper::ConstructL()
	{
	User::LeaveIfError(iFs.Connect());
	}

/**
 * Process command to see what test to run
 */
TBool CSELFLibWrapper::DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
	{
	__UHEAP_MARK;

	(void)aSection;
	(void)aAsyncErrorIndex;

	RBuf8 name;
	name.CreateL(aCommand.Length());
	name.Copy(aCommand);
	RDebug::Printf("\nNext Test Case... [%S]", &name);
	name.Close();

	if(KSELFNewL() == aCommand)
		TestNewL_L();
	else if(KSELFNewLC() == aCommand)
		TestNewLC_L();
	else if(KSELF_InvalidSignature() == aCommand)
		TestConstruction_InvalidELF_SignatureL();
	else if(KSELF_InvalidSize() == aCommand)
		TestConstruction_InvalidELF_SizeL();
	else if(KSELF_NotSupported() == aCommand)
		TestConstruction_ValidELF_NotSupported();
	else if(KSELF_TestConstruction_Inuse() == aCommand)
		TestConstruction_Inuse();
	else if(KSELF_GetElfHeaderL() == aCommand)
		TestGetElfHeaderL();
	else if(KSELF_InsertVariantSpecificData() == aCommand)
		TestInsertDataL();
	else if(KSELF_InsertMultipleVariants() == aCommand)
		TestMultipleInsertionL();
	else if(KSELF_InsertNothingAndUpdate() == aCommand)
		TestInsertNothingL();
	else if(KSELF_TestPerformance() == aCommand)
		TestPerformanceL();
	else if(KSELF_TestStress() == aCommand)
		TestStressL();
	else if(KPrepareFiles() == aCommand)
		CopyFilesToWritablePlaceL();
	else if(KTearDownFiles() == aCommand)
		TearDownFilesL();

	__UHEAP_MARKEND;

	return ETrue;
	}

/**
 * Unit test for:
 * CSELFEditor* CSELFEditor::NewL()
 * @see CSELFEditor
 */
void CSELFLibWrapper::TestNewL_L()
	{
	INFO_PRINTF1(_L("Testing CSELFEditor::NewL"));

	//Try a file we know does not exist
	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewL(KSELFFileNameNonExistant()));

	INFO_PRINTF3(_L("CSELFEditor::NewL(%S) returns [%d]"), &KSELFFileNameNonExistant, err);

	T_SELFLIB_ASSERT_TRUE(KErrNotFound == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor == NULL, 0);

	//Try a file we know does exist
	TRAP(err, editor = CSELFEditor::NewL(KSELFFileName()));

	INFO_PRINTF3(_L("CSELFEditor::NewL(%S) returns [%d]"), &KSELFFileName, err);

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor != NULL, 0);

	delete editor;
	editor = NULL;

#ifdef _DEBUG		//__UHEAP_FAILNEXT won't work on urel

	//Try with an allocation fail
	__UHEAP_FAILNEXT(1);
	TRAP(err, editor = CSELFEditor::NewL(KSELFFileName()));

	INFO_PRINTF3(_L("CSELFEditor::NewL(%S) returns [%d] (with __UHEAP_FAILNEXT)"), &KSELFFileName, err);

	T_SELFLIB_ASSERT_TRUE(KErrNoMemory == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor == NULL, 0);

#endif
	}

/**
 * Unit test for:
 * CSELFEditor* CSELFEditor::NewLC()
 * @see CSELFEditor
 */
void CSELFLibWrapper::TestNewLC_L()
	{
 	INFO_PRINTF1(_L("Testing CSELFEditor::NewLC"));

	//Try a file we know does not exist
	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewLC(KSELFFileNameNonExistant()); CleanupStack::Pop(););

	INFO_PRINTF3(_L("CSELFEditor::NewLC(%S) returns [%d]"), &KSELFFileNameNonExistant, err);

	T_SELFLIB_ASSERT_TRUE(KErrNotFound == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor == NULL, 0);

	//Try a file we know does exist
	TRAP(err, editor = CSELFEditor::NewLC(KSELFFileName()); CleanupStack::Pop(););

	INFO_PRINTF3(_L("CSELFEditor::NewLC(%S) returns [%d]"), &KSELFFileName, err);

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor != NULL, 0);

	delete editor;
	editor = NULL;

#ifdef _DEBUG			//__UHEAP_FAILNEXT won't work on urel

	//Try with an allocation fail
	__UHEAP_FAILNEXT(1);
	TRAP(err, editor = CSELFEditor::NewLC(KSELFFileName()); CleanupStack::Pop());

	INFO_PRINTF3(_L("CSELFEditor::NewLC(%S) returns [%d] (with __UHEAP_FAILNEXT)"), &KSELFFileName, err);

	T_SELFLIB_ASSERT_TRUE(KErrNoMemory == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor == NULL, 0);

#endif
	}

/**
 * Unit test for:
 * CSELFEditor::InsertVariantSpecificDataL()
 * @see CSELFEditor
 */
void CSELFLibWrapper::TestInsertDataL()
	{
	INFO_PRINTF1(_L("Testing CSELFEditor Insert Variant Sepcific data"));

	//Get the number of variant data segments there before
	TInt prenumsegments = GetNumberofVariantSegmentsL(KSELFFileName());
	INFO_PRINTF2(_L("There are [0x%X] variant data segments before the test"), prenumsegments);

	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewL(KSELFFileName()));

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor != NULL, 0);

	CleanupStack::PushL(editor);

	INFO_PRINTF1(_L("Inserting the variant specific"));
	TRAP(err, editor->InsertVariantSpecificDataL(KTestData1()));
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);

	TRAP(err, editor->InsertVariantSpecificDataL(KTestData2()));
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);

	TRAP(err, editor->InsertVariantSpecificDataL(KTestData3()));
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);

	INFO_PRINTF1(_L("Updating the ELF file"));
	TRAP(err, editor->WriteELFUpdatesL());

	INFO_PRINTF2(_L("ELF File updating returned [%d]"), err);
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);

	CleanupStack::PopAndDestroy(editor);

	TInt postnumsegments = GetNumberofVariantSegmentsL(KSELFFileName());
	INFO_PRINTF2(_L("There are [0x%X] variant data segments after the test"),postnumsegments);

	T_SELFLIB_ASSERT_TRUE(prenumsegments + 3 == postnumsegments, 0);

	TInt bufferRequired = GetVariantSegmentSizeL(postnumsegments - 1, KSELFFileName());
	RBuf8 data;
	data.CreateL(bufferRequired);
	data.CleanupClosePushL();

	TRAP(err, GetVariantSegmentDataL(postnumsegments - 1, data, KSELFFileName()));

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 1);
	T_SELFLIB_ASSERT_TRUE(0 == KTestData3().Compare(data), 1);

	TRAP(err, GetVariantSegmentDataL(postnumsegments - 2, data, KSELFFileName()));

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 1);
	T_SELFLIB_ASSERT_TRUE(0 == KTestData2().Compare(data), 1);

	TRAP(err, GetVariantSegmentDataL(postnumsegments - 3, data, KSELFFileName()));

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 1);
	T_SELFLIB_ASSERT_TRUE(0 == KTestData1().Compare(data), 1);

	INFO_PRINTF1(_L("Data looks good"));

	CleanupStack::PopAndDestroy(&data);
	}

/**
 * Tests that you can't have mutliple CSELFEditors open on the same ELF file
 * @see CSELFEditor
 */
void CSELFLibWrapper::TestConstruction_Inuse()
	{
	INFO_PRINTF1(_L("Testing CSELFEditor multiple handles"));

	CSELFEditor* handle1 = NULL;
	CSELFEditor* handle2 = NULL;

	//Open one handle
	TRAPD(err, handle1 = CSELFEditor::NewL(KSELFFileName()));

	INFO_PRINTF3(_L("CSELFEditor::NewL(%S) returns [%d]"), &KSELFFileName, err);

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 0);
	T_SELFLIB_ASSERT_TRUE(handle1 != NULL, 0);

	//Try handle two
	TRAP(err, handle2 = CSELFEditor::NewL(KSELFFileName()));

	INFO_PRINTF3(_L("CSELFEditor::NewL(%S) returns [%d] (this is the second handle)"), &KSELFFileName, err);
	T_SELFLIB_ASSERT_TRUE(KErrInUse == err, 0);
	T_SELFLIB_ASSERT_TRUE(handle2 == NULL, 0);

	delete handle1;
	}

/**
 * Unit test for:
 * CSELFEditor::NewL
 * Tests with an invalid ELF file. This is invalid due to its signature
 * @see CSELFEditor
 */
void CSELFLibWrapper::TestConstruction_InvalidELF_SignatureL()
	{
	INFO_PRINTF1(_L("Testing CSELFEditor construction with an invalid file - bad signature"));

	//Create an ELF file editor
	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewL(KSELFFileNameInvalidBySignature()));

	INFO_PRINTF3(_L("Opening [%S] returned [%d]"), &KSELFFileNameInvalidBySignature, err);

	T_SELFLIB_ASSERT_TRUE(KErrCorrupt == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor == NULL, 0);

#ifdef _DEBUG		//__UHEAP_FAILNEXT won't work on urel

	//Try it again with a failed memory alloc
	__UHEAP_FAILNEXT(1);

	TRAP(err, editor = CSELFEditor::NewL(KSELFFileNameInvalidBySignature()));

	INFO_PRINTF3(_L("Opening [%S] returned [%d] (with __UHEAP_FAILNEXT)"), &KSELFFileNameInvalidBySignature, err);

	T_SELFLIB_ASSERT_TRUE(KErrNoMemory == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor == NULL, 0);

#endif
	}

/**
 * There are valid ELF files that the ELF lib won't support (mainly) none Core files
 * This tests that it throws a KErrNotSupported.
 * @see CSELFEditor
 */
void CSELFLibWrapper::TestConstruction_ValidELF_NotSupported()
	{
	INFO_PRINTF1(_L("Testing CSELFEditor construction with an valid file but unsupported"));

	//Create an ELF file editor
	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewL(KSELFFileNameExe()));

	INFO_PRINTF3(_L("Opening [%S] returned [%d]"), &KSELFFileNameExe, err);

	T_SELFLIB_ASSERT_TRUE(KErrNotSupported == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor == NULL, 0);
	}

/**
 * Unit test for:
 * CSELFEditor::NewL
 * Tests with an invalid ELF file. This is invalid due to its size
 * @see CSELFEditor
 */
void CSELFLibWrapper::TestConstruction_InvalidELF_SizeL()
	{
	INFO_PRINTF1(_L("Testing CSELFEditor construction with an invalid file - bad size"));

	//Create an ELF file editor
	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewL(KSELFFileNameTiny()));

	INFO_PRINTF3(_L("Opening [%S] returned [%d]"), &KSELFFileNameTiny, err);

	T_SELFLIB_ASSERT_TRUE(KErrCorrupt == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor == NULL, 0);

#ifdef _DEBUG			//__UHEAP_FAILNEXT won't work on urel

	//Try it again with a failed memory alloc
	__UHEAP_FAILNEXT(1);

	TRAP(err, editor = CSELFEditor::NewL(KSELFFileNameTiny()));

	INFO_PRINTF3(_L("Opening [%S] returned [%d] (with __UHEAP_FAILNEXT)"), &KSELFFileNameTiny, err);

	T_SELFLIB_ASSERT_TRUE(KErrNoMemory == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor == NULL, 0);

#endif	
	}

/**
 * Unit test for:
 * CSELFEditor::GetElfHeaderL()
 * @see CSELFEditor
 */
void CSELFLibWrapper::TestGetElfHeaderL()
	{
	INFO_PRINTF1(_L("Testing CSELFEditor::GetElfHeaderL()"));

	//Try a file we know does exist
	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewL(KSELFFileName()));

	INFO_PRINTF3(_L("CSELFEditor::NewL(%S) returns [%d]"), &KSELFFileName, err);

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor != NULL, 0);

	CleanupStack::PushL(editor);

	Elf32_Ehdr header;
	editor->GetELFHeader(header);

	LogELFHeader(header);

	//Assert on the values we can be sure of to be sure potatoes
	T_SELFLIB_ASSERT_TRUE(header.e_type == ET_CORE, 1);
	T_SELFLIB_ASSERT_TRUE(header.e_version == EV_CURRENT, 1);
	T_SELFLIB_ASSERT_TRUE(header.e_machine == EM_ARM, 1);
	T_SELFLIB_ASSERT_TRUE(header.e_entry == 0, 1);
	T_SELFLIB_ASSERT_TRUE(header.e_ehsize == sizeof(Elf32_Ehdr), 1);
	T_SELFLIB_ASSERT_TRUE(header.e_phnum > 0, 1);

	CleanupStack::PopAndDestroy(editor);
	}

/**
 * Test we can insert more than one variant specific segment
 * @leave One of the system wide codes
 */
void CSELFLibWrapper::TestMultipleInsertionL()
	{
	INFO_PRINTF1(_L("Testing CSELFEditor Insert Multiple Variant Sepcific data"));

	//Get the number of variant data segments there before
	TInt prenumsegments = GetNumberofVariantSegmentsL(KSELFFileName());
	INFO_PRINTF2(_L("There are [0x%X] variant data segments before the test"), prenumsegments);

	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewL(KSELFFileName()));

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 0);
	T_SELFLIB_ASSERT_TRUE(editor != NULL, 0);

	CleanupStack::PushL(editor);

	INFO_PRINTF1(_L("Inserting the variant specific data"));
	TRAP(err, editor->InsertVariantSpecificDataL(KTestData1()));
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);

	INFO_PRINTF1(_L("Updating the ELF file"));
	TRAP(err, editor->WriteELFUpdatesL());

	INFO_PRINTF2(_L("ELF File updating returned [%d]"), err);
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);

	//Write a second time
	INFO_PRINTF1(_L("Inserting more variant specific data"));
	TRAP(err, editor->InsertVariantSpecificDataL(KTestData2()));
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);

	INFO_PRINTF1(_L("Updating the ELF file"));
	TRAP(err, editor->WriteELFUpdatesL());

	CleanupStack::PopAndDestroy(editor);

	TInt postnumsegments = GetNumberofVariantSegmentsL(KSELFFileName());
	INFO_PRINTF2(_L("There are [0x%X] variant data segments after the test"),postnumsegments);

	T_SELFLIB_ASSERT_TRUE(prenumsegments + 2 == postnumsegments, 0);

	TInt bufferRequired = GetVariantSegmentSizeL(postnumsegments - 1, KSELFFileName());
	RBuf8 data;
	data.CreateL(bufferRequired);
	data.CleanupClosePushL();

	TRAP(err, GetVariantSegmentDataL(postnumsegments - 1, data, KSELFFileName()));

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 1);
	T_SELFLIB_ASSERT_TRUE(0 == KTestData2().Compare(data), 1);

	TRAP(err, GetVariantSegmentDataL(postnumsegments - 2, data, KSELFFileName()));

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 1);
	T_SELFLIB_ASSERT_TRUE(0 == KTestData1().Compare(data), 1);

	INFO_PRINTF1(_L("Data looks good"));

	CleanupStack::PopAndDestroy(&data);

	}

/**
 * Adds nothing to the ELF file but calls the update function. Verifies that the
 * file remains the same
 * @leave One of the system wide codes
 */
void CSELFLibWrapper::TestInsertNothingL()
	{
	INFO_PRINTF1(_L("Testing Inserting nothing"));

	//Get size of file before
	RFile theFile;
	User::LeaveIfError(theFile.Open(iFs, KSELFFileName(), EFileRead));
	CleanupClosePushL(theFile);

	TInt previousFileSize = 0;
	User::LeaveIfError(theFile.Size(previousFileSize));
	CleanupStack::PopAndDestroy(&theFile);

	INFO_PRINTF2(_L("The test file before is [0x%X] bytes"), previousFileSize);

	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewL(KSELFFileName()));

	INFO_PRINTF3(_L("CSELFEditor::NewL(%S) returns [%d]"), &KSELFFileName, err);

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 1);
	T_SELFLIB_ASSERT_TRUE(editor != NULL, 1);

	CleanupStack::PushL(editor);

	//Update having made no updates
	INFO_PRINTF1(_L("Updating the ELF file"));
	TRAP(err, editor->WriteELFUpdatesL());

	CleanupStack::PopAndDestroy(editor);

	//Now get the file size again, should be the same
	TInt currentFileSize = 0;
	User::LeaveIfError(theFile.Open(iFs, KSELFFileName(), EFileRead));	
	CleanupClosePushL(theFile);

	User::LeaveIfError(theFile.Size(currentFileSize));
	INFO_PRINTF2(_L("The test file after is [0x%X] bytes"), currentFileSize);

	CleanupStack::PopAndDestroy(&theFile);

	T_SELFLIB_ASSERT_TRUE(currentFileSize == previousFileSize, 0);
	}

/**
 * Logs an ELF header to the test logs
 * @param aHeader Header to log
 */
void CSELFLibWrapper::LogELFHeader(const Elf32_Ehdr& aHeader)
	{
 	INFO_PRINTF1(_L("The ELF Header looks like:"));
 	INFO_PRINTF2(_L("\te_type:\t\t0x%X"), aHeader.e_type);
	INFO_PRINTF2(_L("\te_version:\t0x%X"), aHeader.e_version);
	INFO_PRINTF2(_L("\te_machine:\t0x%X"), aHeader.e_machine);
	INFO_PRINTF2(_L("\te_entry:\t0x%X"), aHeader.e_entry);
	INFO_PRINTF2(_L("\te_phoff:\t0x%X"), aHeader.e_phoff);
	INFO_PRINTF2(_L("\te_shoff:\t0x%X"), aHeader.e_shoff);
	INFO_PRINTF2(_L("\te_flags:\t0x%X"), aHeader.e_flags);
	INFO_PRINTF2(_L("\te_ehsize:\t0x%X"), aHeader.e_ehsize);
	INFO_PRINTF2(_L("\te_phentsize:\t0x%X"), aHeader.e_phentsize);
	INFO_PRINTF2(_L("\te_phnum:\t0x%X"), aHeader.e_phnum);
	INFO_PRINTF2(_L("\te_shentsize:\t0x%X"), aHeader.e_shentsize);
	INFO_PRINTF2(_L("\te_shnum:\t0x%X"), aHeader.e_shnum);
	INFO_PRINTF2(_L("\te_shstrndx:\t0x%X"), aHeader.e_shstrndx);
	}

void CSELFLibWrapper::TestPerformanceL()
	{
	INFO_PRINTF1(_L("Testing performance"));

	//Use case 1: Normal use case, insert 1 variant specific data segment
	TUint startTick = User::NTickCount();

	CSELFEditor* editor = NULL;
	editor = CSELFEditor::NewLC(KSELFFileName());
	editor->InsertVariantSpecificDataL(KTestData1());
	editor->WriteELFUpdatesL();

	TUint stopTick = User::NTickCount();
	TReal seconds = (TReal)(stopTick - startTick)/(TReal)HelpTicksPerSecond();
	INFO_PRINTF2(_L("Inserting one segment takes %f seconds"), seconds);
	CleanupStack::PopAndDestroy(editor);

	//Use case 2: Lots of inserts
	//Get size of file before
	RFile theFile;
	User::LeaveIfError(theFile.Open(iFs, KSELFFileName(), EFileRead));
	CleanupClosePushL(theFile);

	TInt startingFileSize = 0;
	User::LeaveIfError(theFile.Size(startingFileSize));
	CleanupStack::PopAndDestroy(&theFile);

	TInt prenumsegments = GetNumberofVariantSegmentsL(KSELFFileName());
	INFO_PRINTF2(_L("There are [0x%X] variant data segments before the test"), prenumsegments);

	editor = CSELFEditor::NewLC(KSELFFileName());

	INFO_PRINTF1(_L("Inserting the variant specific data"));

	static const int numToTest = 50;
	startTick = User::NTickCount();
	for(TUint i = 0; i < numToTest; i++)
		{
		editor->InsertVariantSpecificDataL(KTestData1());
		}

	INFO_PRINTF1(_L("Updating the ELF file"));
	editor->WriteELFUpdatesL();

	stopTick = User::NTickCount();
	seconds = (TReal)(stopTick - startTick)/(TReal)HelpTicksPerSecond();

	CleanupStack::PopAndDestroy(editor);

	TInt postnumsegments = GetNumberofVariantSegmentsL(KSELFFileName());
	INFO_PRINTF2(_L("There are [0x%X] variant data segments after the test"), postnumsegments);

	T_SELFLIB_ASSERT_TRUE(prenumsegments + numToTest == postnumsegments, 0);
	INFO_PRINTF4(_L("Starting file size: 0x%X %d and took %f seconds"), startingFileSize, startingFileSize, seconds);

	}

/**
 * Tests we can insert a large amount of data into the ELF file
 * @leave ONe of the system wide codes
 */
void CSELFLibWrapper::TestStressL()
	{
	INFO_PRINTF1(_L("Testing CSELFEditor Insert Large amount of Variant Sepcific data"));

	//Get the number of variant data segments there before
	TInt prenumsegments = GetNumberofVariantSegmentsL(KSELFFileName());
	INFO_PRINTF2(_L("There are [0x%X] variant data segments before the test"), prenumsegments);

	RBuf8 largeData;
	largeData.CreateL(KLargeDataSize);
	largeData.CleanupClosePushL();

	CSELFEditor* editor = NULL;
	TRAPD(err, editor = CSELFEditor::NewL(KSELFFileName()));

	INFO_PRINTF2(_L("getting %d"), err);
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 1);
	T_SELFLIB_ASSERT_TRUE(editor != NULL, 1);

	CleanupStack::PushL(editor);

	INFO_PRINTF1(_L("Inserting the variant specific"));
	TRAP(err, editor->InsertVariantSpecificDataL(largeData));
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);

	INFO_PRINTF1(_L("Updating the ELF file"));
	TRAP(err, editor->WriteELFUpdatesL());

	INFO_PRINTF2(_L("ELF File updating returned [%d]"), err);
	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);

	CleanupStack::PopAndDestroy(editor);

	TInt postnumsegments = GetNumberofVariantSegmentsL(KSELFFileName());
	INFO_PRINTF2(_L("There are [0x%X] variant data segments after the test"),postnumsegments);

	T_SELFLIB_ASSERT_TRUE(prenumsegments + 1 == postnumsegments, 0);

	TInt bufferRequired = GetVariantSegmentSizeL(postnumsegments - 1, KSELFFileName());
	RBuf8 data;
	data.CreateL(bufferRequired);
	data.CleanupClosePushL();

	TRAP(err, GetVariantSegmentDataL(postnumsegments - 1, data, KSELFFileName()));

	T_SELFLIB_ASSERT_TRUE(KErrNone == err, 2);
	T_SELFLIB_ASSERT_TRUE(0 == largeData.Compare(data), 2);

	INFO_PRINTF1(_L("Data looks good"));
	CleanupStack::PopAndDestroy(&data);
	CleanupStack::PopAndDestroy(&largeData);
	}

/**
 * Returns the number of nanokernel ticks in one second
 * @return Number of nanokernel ticks. 0 if unsuccesful
 */
TInt CSELFLibWrapper::HelpTicksPerSecond()
	{
	TInt nanokernel_tick_period;
	HAL::Get(HAL::ENanoTickPeriod, nanokernel_tick_period);
	static const TInt KOneMillion = 1000000;
	return KOneMillion/nanokernel_tick_period;
	}

/**
 * Utility to get the number of variant data segments in the file
 * @leave One of the system wide codes
 */
TInt CSELFLibWrapper::GetNumberofVariantSegmentsL(const TDesC& aFilename)
	{
	RFile elfFile;
	User::LeaveIfError(elfFile.Open(iFs, aFilename, EFileRead));
	CleanupClosePushL(elfFile);

	READ_STRUCTL(Elf32_Ehdr, ehdr, elfFile, 0);

	TUint count = 0;

	//Now go through program headers and count relevant dhdrs
	for(TUint i = 0; i < ehdr.e_phnum; i++)
		{
		READ_STRUCTL(Elf32_Phdr, phdr, elfFile, ehdr.e_phoff + (i * sizeof(Elf32_Phdr)));
		if(phdr.p_type == PT_NOTE)
			{
			READ_STRUCTL(Sym32_dhdr, dhdr, elfFile, phdr.p_offset);
			if(dhdr.d_type == ESYM_NOTE_VARIANT_DATA)
				++count;
			}
		}

	CleanupStack::PopAndDestroy(&elfFile);

	return count;
	}

/**
 * Gets the size of the aIndexth variant segment element we find
 * in the ELF file
 * @param aIndex Index of the segment of interest
 * @return The size required to read this data
 * @leave KErrNotFound if the index is wrong or one of the system wide codes
 */
TInt CSELFLibWrapper::GetVariantSegmentSizeL(const TUint aIndex, const TDesC& aFilename)
	{
	RFile elfFile;
	User::LeaveIfError(elfFile.Open(iFs, aFilename, EFileRead));
	CleanupClosePushL(elfFile);

	READ_STRUCTL(Elf32_Ehdr, ehdr, elfFile, 0);

	TUint count = 0;

	//Now go through program headers and count relevant dhdrs
	for(TUint i = 0; i < ehdr.e_phnum; i++)
		{
		READ_STRUCTL(Elf32_Phdr, phdr, elfFile, ehdr.e_phoff + (i * sizeof(Elf32_Phdr)));
		if(phdr.p_type == PT_NOTE)
			{
			READ_STRUCTL(Sym32_dhdr, dhdr, elfFile, phdr.p_offset);
			if(dhdr.d_type == ESYM_NOTE_VARIANT_DATA)
				{
				if(count == aIndex)
					{
					READ_STRUCTL(Sym32_variant_spec_data, vdata, elfFile, phdr.p_offset + sizeof(Sym32_dhdr));
					CleanupStack::PopAndDestroy(&elfFile);
					return vdata.es_size;
					}
				count++;
				}
			}
		}

	User::Leave(KErrNotFound);
	return 0; //avoid warnings
	}

/**
 * Gets the data of the aIndexth variant segment element we find
 * in the ELF file
 * @param aIndex Index of the segment of interest
 * @param aData buffer to hold data
 * @leave KErrNotFound if the index is wrong or one of the system wide codes
 */
void CSELFLibWrapper::GetVariantSegmentDataL(TUint aIndex, TDes8& aData, const TDesC& aFilename)
	{
	RFile elfFile;
	User::LeaveIfError(elfFile.Open(iFs, aFilename, EFileRead));
	CleanupClosePushL(elfFile);

	READ_STRUCTL(Elf32_Ehdr, ehdr, elfFile, 0);

	TUint count = 0;

	//Now go through program headers and count relevant dhdrs
	for(TUint i = 0; i < ehdr.e_phnum; i++)
		{
		READ_STRUCTL(Elf32_Phdr, phdr, elfFile, ehdr.e_phoff + (i * sizeof(Elf32_Phdr)));
		if(phdr.p_type == PT_NOTE)
			{
			READ_STRUCTL(Sym32_dhdr, dhdr, elfFile, phdr.p_offset);
			if(dhdr.d_type == ESYM_NOTE_VARIANT_DATA)
				{
				if(count == aIndex)
					{
					READ_STRUCTL(Sym32_variant_spec_data, vdata, elfFile, phdr.p_offset + sizeof(Sym32_dhdr));
					if(aData.MaxLength() < vdata.es_size)
						{
						User::Leave(KErrTooBig);
						}
					//read the data
					elfFile.Read(vdata.es_data, aData);
					CleanupStack::PopAndDestroy(&elfFile);
					return;
					}
				count++;
				}
			}
		}

	User::Leave(KErrNotFound);
	}

/**
 * Copies test files from not writable z:\ drive to writable place
 * @leave One of the system wide codes
 */
void CSELFLibWrapper::CopyFilesToWritablePlaceL()
	{
	INFO_PRINTF1(_L("Copying the files and setting correct attributes"));
 	BaflUtils::EnsurePathExistsL(iFs, KSELFFileName);

	TUint attToRemove = KEntryAttReadOnly;
	TUint attToSet = 0;

 	User::LeaveIfError(BaflUtils::CopyFile(iFs, KROMSELFFileName, KSELFFileName));
	iFs.SetAtt(KSELFFileName, attToSet, attToRemove);

	User::LeaveIfError(BaflUtils::CopyFile(iFs, KROMSELFFileNameInvalidBySignature, KSELFFileNameInvalidBySignature));
	iFs.SetAtt(KSELFFileNameInvalidBySignature, attToSet, attToRemove);

	User::LeaveIfError(BaflUtils::CopyFile(iFs, KROMSELFFileNameTiny, KSELFFileNameTiny));
	iFs.SetAtt(KSELFFileNameTiny, attToSet, attToRemove);

	User::LeaveIfError(BaflUtils::CopyFile(iFs, KROMSELFFileNameExe, KSELFFileNameExe));
	iFs.SetAtt(KSELFFileNameExe, attToSet, attToRemove);
	}

/**
 * Cleans the files copied by CopyFilesToWritablePlaceL() to tidy up
 */
void CSELFLibWrapper::TearDownFilesL()
	{
	INFO_PRINTF1(_L("Deleting the files"));

 	User::LeaveIfError(BaflUtils::DeleteFile(iFs, KSELFFileName));
	User::LeaveIfError(BaflUtils::DeleteFile(iFs, KSELFFileNameInvalidBySignature));
	User::LeaveIfError(BaflUtils::DeleteFile(iFs, KSELFFileNameTiny));
	User::LeaveIfError(BaflUtils::DeleteFile(iFs, KSELFFileNameExe));
	}
