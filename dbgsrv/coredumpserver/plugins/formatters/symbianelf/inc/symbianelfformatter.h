// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implements two examples of the ECOM Symbian Elf Core Dump API
//

/**
 @file
 @internalTechnology
 @released
*/

#ifndef __SYMBIAN_ELF_FORMATTER_H__
#define __SYMBIAN_ELF_FORMATTER_H__

// Abstract ECOM API definition
#include <formatterapi.h>
#include <symbianelfdefs.h>
#include <crashdatasource.h>

#include "symbianelfoptions.h"
#include <crashdatasave.h>
#include "tsymbianelfformatter.h"
#include "symbianelfstringinfo.h"

const TUint32 KSymbianELFFormatterUid = { 0x10282fe3 };
_LIT(KPluginDescription, "Symbian Elf Core Dump formatter");

/**
@internalTechnology
@released
Symbian ELF formatter plugin is loaded by the core dump server and notified about the crash event.
Then it dupms crash data in the ELF file format.
@see CCrashDataSave
*/
class CSymbianElfFormatter : public CCoreDumpFormatter
{
    friend void CSymbianElfFormatterTest::TestSuiteL(TSuite aSuite); //test harness
    enum { KElfDumpFileLength = 23 };
public:
	static CSymbianElfFormatter* NewL();
	static CSymbianElfFormatter* NewLC();
	virtual ~CSymbianElfFormatter();

public:

	virtual void ConfigureDataSaveL( CCrashDataSave * aDataSave );

	virtual void ConfigureDataSourceL( CCrashDataSource * aDataSource );

	virtual void GetDescription( TDes & aPluginDescription );

	virtual void CrashEventL( TCrashInfo* aCrashInfo );

	virtual TInt GetNumberConfigParametersL( );

	virtual COptionConfig * GetConfigParameterL( const TInt aIndex );

	virtual void SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue );

public:

private:

    /** Helper structure for creating data segs for stacks and .rodata/.bss/.data sections */
	struct TSegmentInfo
	{
        /** base memory address of the segment */
		TUint32 iSegBase;
        /** size of the segment data */
		TUint32 iSegSize;
        /** access rights to the segment data */
		TUint32 iFlags;

        /** reserved for future */
		TUint32 iSpare1;
        /** reserved for future */
		TUint32 iSpare2;
	};

    /**
     @publishedPartner
     @released
     List of TSegmentInfo structures
    @see TSegmentInfo
    @see RArray
    */
	typedef RArray<TSegmentInfo> RSegmentList;

	void ConstructL();
	CSymbianElfFormatter();
	void FindAllOfSameClassRepresentation( RRegisterList &aRegs,
											RRegisterList &aRegsOfSameClassRepr,
											ESYM_REGCLASS aClass,
											ESYM_REGREP aRepr );

	TUint WriteSymbianInfoL(const TCrashInfo &aCrashInfo, const TUint32 aCrc32);

	TUint WriteDataSegmentsL( const TUint64 & aThreadId,
							  const TUint64 & aPid,
							  const RThreadPointerList &aThreadList,
							  const RCodeSegPointerList &aCodeSegs,
							  RSegmentList &aDataSegs,
							  RArray<TUint> &aDataSegOffsets );

	TUint WriteCodeSegmentsL( const TUint64 & aThreadId,
							  const RCodeSegPointerList &aCodeSegs,
							  RArray<TUint> &aCodeSegOffsets );

	TUint WriteThreadInfoL(const TUint64 & aPid, const RThreadPointerList &aThreadList);

	TUint WriteProcessInfoL(const TUint64 & aPid, const RProcessPointerList &aProcessList);
	TUint WriteExecutableInfoL(const RCodeSegPointerList &aCodeSegs, const TUint64 &aTime, const TUint64 &aTid);

	void  WriteRegisterInfoL( const TUint64 & aTid,
							  const TUint64 & aPid,
							  RArray<TUint>& aOffsets,
							  RRegisterList &aArmExcRegs,
							  const RThreadPointerList &aThreadList);
	TUint WriteStringInfoL();

	void SetArmRegsFromCrashDataL(const TCrashInfo &aCrashInfo, RRegisterList &aArmExcRegs);

	TBool GetArmExcReg(const TCrashInfo &aCrashInfo, const TRegisterData & aReg, TRegisterValue32 & aVal);

	void UpdateCrashProgressL( const TDesC &aProgress );
	TUint32 CalcExecCrcL(const TUint64 &aTid, const TCodeSegInfo &aExecInfo );

	void PrepareFileNameL(TUint64 aStamp);
    TUint WriteTraceBufferL();

	static void CleanupCodeSegList(TAny *aArray);
	static void CleanupProcessList(TAny *aArray);
	static void CleanupThreadList(TAny *aArray);

private:

    /** Pointer to data source object created and managed by core dump server, provides API to gather necessary info about the crash */
	CCrashDataSource * iDataSource;

    /** Pointer to writer plugin created and managed by core dump server, provides API to store the dump information*/
	CCrashDataSave	 * iDataSave;

    /** Keep track of the current media-position indicator */
	TUint			   iOffset;

    /** Responsible for holding all the strings required to be stored in ELF string info segment */
	CStringInfoTable * iStrInfoTbl;

    /** Symbian ELF formatter configuration parameter list */
	CElfOptions		 * iElfOptions;

	/** Dump file Name */
	RBuf iElfFileName;
};

#endif // __SYMBIAN_ELF_FORMATTER_H__
