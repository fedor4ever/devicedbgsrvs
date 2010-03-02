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
//  Test wrapper for unit tests of the self lib

/**
 * @file CSelfLibWrapper.h
 * @internalTechnology
 * @prototype
 */

#ifndef __CSELFLIB_WRAPPER_H__
#define __CSELFLIB_WRAPPER_H__

#include <datawrapper.h>
#include <selflib.h>
#include <hal.h>

//Constructor TestCase names
_LIT(KSELFNewLC, "NewLC");
_LIT(KSELFNewL, "NewL");

//Negative tests for construction
_LIT(KSELF_InvalidSignature, "TestConstruction_InvalidFile_Signature");
_LIT(KSELF_InvalidSize, "TestConstruction_InvalidFile_Size");
_LIT(KSELF_NotSupported, "TestConstruction_NotSupported");
_LIT(KSELF_TestConstruction_Inuse, "TestConstruction_Inuse");

//Test for getters
_LIT(KSELF_GetElfHeaderL, "GetElfHeaderL");

//Test for insertion
_LIT(KSELF_InsertVariantSpecificData, "InsertVariantSpecificData");
_LIT(KSELF_InsertMultipleVariants, "InsertMultipleVariants");
_LIT(KSELF_InsertNothingAndUpdate, "InsertNothingAndUpdate");
_LIT(KSELF_TestPerformance, "TestPerformance");
_LIT(KSELF_TestStress, "TestStress");

//ELF Files to test
_LIT(KSELFFileName, "C:\\data\\testelf1.elf");
_LIT(KSELFFileNameInvalidBySignature, "C:\\data\\testelfinvalid.elf");
_LIT(KSELFFileNameTiny, "C:\\data\\tinytestelf.elf");
_LIT(KSELFFileNameNonExistant, "C:\\idontexist1.elf");
_LIT(KSELFFileNameExe, "C:\\data\\testelfexecutable.elf");

_LIT(KROMSELFFileName, "Z:\\data\\testelf1.elf");
_LIT(KROMSELFFileNameInvalidBySignature, "Z:\\data\\testelfinvalid.elf");
_LIT(KROMSELFFileNameTiny, "Z:\\data\\tinytestelf.elf");
_LIT(KROMSELFFileNameNonExistant, "Z:\\idontexist1.elf");
_LIT(KROMSELFFileNameExe, "Z:\\data\\testelfexecutable.elf");

//Preamble tests
_LIT(KPrepareFiles, "PrepareFiles");
_LIT(KTearDownFiles, "TearDownFiles");

/**
 * Can only use in void functions
 * @param x Boolean
 * @param If we fail, pop and destroy y items
 */
#define T_SELFLIB_ASSERT_TRUE(x, y)\
	if(x == 0){\
		ERR_PRINTF2(_L("Assertion Error in CSELFLibWrapper.cpp line [%d]"), __LINE__);\
		SetBlockResult(EFail);\
		if(y>0)\
			CleanupStack::PopAndDestroy(y);\
		return;\
		}\

/**
 * Use for TUint returning functions
 * @param x Boolean
 * @param If we fail, pop and destroy y items and return z
 */
#define T_SELFLIB_ASSERT_TRUE_RET(x, y, z) \
	if(!x){\
		ERR_PRINTF2(_L("Assertion Error in CSELFLibWrapper.cpp line [%d]"), __LINE__);\
		SetBlockResult(EFail);\
		if(y>0)\
			CleanupStack::PopAndDestroy(y);\
		return z; \
		}

_LIT8(KTestData1, "This is some test variant data");
_LIT8(KTestData2, "This is some more variant data");
_LIT8(KTestData3, "This is even more variant data");

static const TUint KLargeDataSize = 0xE1000;

/**
 * This class is responsible for testing the SELF editing library
 */
class CSELFLibWrapper : public CDataWrapper
	{
	public:
		static CSELFLibWrapper* NewL();
		~CSELFLibWrapper();

		virtual TBool DoCommandL(const TTEFFunction& aCommand,
			   const TTEFSectionName& aSection,
			   const TInt aAsyncErrorIndex);

		virtual TAny* GetObject() { return NULL; /* *iObject */ }
		virtual void SetObject(TAny* aObject) { (void)aObject; }

	protected:
		void ConstructL();

	private:
		CSELFLibWrapper();
		void LogELFHeader(const Elf32_Ehdr& aHeader);

		//Test cases
		void TestNewL_L();
		void TestNewLC_L();
		void TestInsertDataL();
		void TestConstruction_InvalidELF_SignatureL();
		void TestConstruction_InvalidELF_SizeL();
		void TestConstruction_ValidELF_NotSupported();
		void TestConstruction_Inuse();
		void TestGetElfHeaderL();
		void TestMultipleInsertionL();
		void TestInsertNothingL();
		void TestPerformanceL();
		void TestStressL();

		void CopyFilesToWritablePlaceL();
		void TearDownFilesL();

		TInt GetNumberofVariantSegmentsL(const TDesC& aFilename);
		TInt GetVariantSegmentSizeL(const TUint aIndex, const TDesC& aFilename);
		void GetVariantSegmentDataL(TUint aIndex, TDes8& aData, const TDesC& aFilename);
		TInt HelpTicksPerSecond();

	private:
		RFs iFs;
	};

#endif // __CSELFLIB_WRAPPER_H__
