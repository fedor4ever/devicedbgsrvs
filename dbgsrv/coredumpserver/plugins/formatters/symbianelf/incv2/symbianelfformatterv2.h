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

#ifndef __SYMBIAN_ELF_FORMATTERV2_H__
#define __SYMBIAN_ELF_FORMATTERV2_H__

#include <e32property.h>

// Abstract ECOM API definition
#include <formatterapi.h>
#include <symbianelfdefs.h>
#include <crashdatasource.h>
#include <crashdatasave.h>

#include "symbianelfoptionsv2.h"
#include "tsymbianelfformatter.h"
#include "symbianelfstringinfov2.h"

const TUint32 KSymbianELFFormatterV2Uid = { 0x102836bb };
const TUint32 KCPSRUsrMode = 0x10;

_LIT(KPluginDescriptionV2, "Symbian Elf Core Dump formatter V2");

/**
@internalTechnology 
@released 
Symbian ELF formatter plugin is loaded by the core dump server and notified about the crash event.
Then it dupms crash data in the ELF file format.
@see CCrashDataSave
*/
class CSymbianElfFormatterV2 : public CCoreDumpFormatter
{
    friend void CSymbianElfFormatterTest::TestSuiteL(TSuite aSuite); //test harness
    enum { KElfDumpFileLength = 23 };
public:
	static CSymbianElfFormatterV2* NewL();
	static CSymbianElfFormatterV2* NewLC();
	virtual ~CSymbianElfFormatterV2();

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
     * @publishedPartner
     * @released
     * List of TSegmentInfo structures
     * @see TSegmentInfo
     * @see RArray
     */
	typedef RArray<TSegmentInfo> RSegmentList;

	void ConstructL();
	CSymbianElfFormatterV2();
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
	void UpdateMediaNameL(const TDesC &aMediaFileName, const TDesC &aStatus);

	TUint32 CalcExecCrcL(const TUint64 &aTid, const TCodeSegInfo &aExecInfo );

    void PrepareFileNameL(TUint64 aStamp);

    TUint WriteTraceBufferL();
    TUint WriteLockDataL();
    TUint WriteRomBuildInfoL();
	TUint WriteVariantSpecificDataL();

    void WriteRawMemoryToFileL(const TUint64 aThreadId, const TUint32 aAddress, const TUint32 aLength);
    void HelpWriteRawMemoryToFileL(const TUint64 aThreadId, const TUint32 aAddress, const TUint32 aLength);
    
    TBool IsSystemCrash();
    
	static void CleanupCodeSegList(TAny *aArray);
	static void CleanupProcessList(TAny *aArray);
	static void CleanupThreadList(TAny *aArray);
	
private:

    /** Pointer to data source object created and managed by core dump server, provides API to gather necessary info about the crash */
	CCrashDataSource* iDataSource;

    /** Pointer to writer plugin created and managed by core dump server, provides API to store the dump information*/
	CCrashDataSave* iDataSave;

    /** Keep track of the current media-position indicator */
	TUint iOffset;

    /** Responsible for holding all the strings required to be stored in ELF string info segment */
	CStringInfoTableV2* iStrInfoTbl;

    /** Symbian ELF formatter configuration parameter list */
	CElfOptionsV2* iElfOptions;
	
	/** Records the CPSR */
	TUint32 iCPSR;
	
	/** Dump file Name */
	RBuf iElfFileName;
	
	/** Crash UID lower 32 bits of crash time uniquely identifies the crash*/
	TUid iCrashUID;

	/** Crash UID Property Buffer */
	RBuf iCrashIDBuffer;

	/** @see ECrashprogress
	 * Crash Progress Buffer
	 */
	RBuf iCrashProgressBuffer;

	/** RProperty for reporting crash processing progress */
	RProperty iCrashProgress;

	/** RProperty for canceling crash processing */
	RProperty iCancelCrash;

	/** @see ECrashMediaName
	 * RProperty for reporting the crash media name
	 */
	RProperty iCrashMediaName;

private:
	
	void HandleCrashEventL(TCrashInfo* aCrashInfo);
	TUint HelpWriteTraceBufferL(TUint aTraceAvailable, TUint aTraceWanted, TUint aOffset);
	TUint HelpWriteTraceBufferWithTraceFrameWorkL(TUint aTraceWanted, TUint aOffset);
};

#endif // __SYMBIAN_ELF_FORMATTERV2_H__
