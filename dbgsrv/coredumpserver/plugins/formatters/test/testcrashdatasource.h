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
// Simulated Data Source API that supplies crash information 
// by implementing the data source API CCrashDataSource 
//



/**
 @file
 @internalTechnology
 @released
*/

#ifndef TEST_CRASH_DATA_SOURCE_H
#define TEST_CRASH_DATA_SOURCE_H

#include <e32std.h>
#include <e32base.h>


#include <crashdatasource.h>
#include <scmdatatypes.h>
//
// CTestCrashDataSource 
//
class CTestCrashDataSource : public CCrashDataSource 
{
public:

	CTestCrashDataSource();

	virtual ~CTestCrashDataSource();
protected:
	virtual void ConstructL();

public:
	
	/** This call returns information on all available registers. It does not
	return the register contents. This is used to establish which registers 
	a formatter could ask for, in particular for Co Processor registers. 
	The callee (not the caller) allocates aRegisterList. 
	This method is used by a formatter to establish which registers it can ask for, 
	and thus need only be called once per session. */
	virtual void GetRegisterListL( RRegisterList  &	aRegisterList );

	/** Ask the Core Dump server for some register data. The caller allocates
	the array and fills in the details of the registers that it would like 
	the contents for */
	virtual void ReadRegistersL( const TUint64 aThreadId, RRegisterList &aRegisterList );

	//
	// Argument aLength must be a multiple of aAccessSize
    // Argument aAddress is aAccessSize aligned
    // The existence of argument aEndianess is not published, 
	// allowing for future implementation
	//
	virtual void ReadMemoryL(
                   const TUint64    aThreadId,
                   const TUint32    aAddress,
                   const TUint32    aLength,
                   TDes8          & aData );


	//
	// Get process list 
	// 
	virtual void GetProcessListL( RProcessPointerList & aData,
							      TUint & aTotalProcessListDescSize );

	//
	// Get executable list 
	// 
	virtual void GetExecutableListL( RExecutablePointerList & aData,
							      TUint & aTotalExecutableListDescSize );


    //
  	// Get the thread list for a given process
  	// 
  	//virtual TInt GetThreadList( const TUint32 aProcessId, RThreadList& aThreadList );
	virtual void GetThreadListL(	const TUint64 aProcessId, 
									RThreadPointerList & aThreadList,
									TUint & aTotalThreadListDescSize );


  	virtual void GetThreadListL(	const TUint32 aProcessId, 
									RThreadPointerList & aThreadList,
									TUint & aTotalThreadListDescSize );

    //
    // API calls below come from the prototype
    //
    virtual void GetCodeSegmentsL(const TUint64 aTid, RCodeSegPointerList &aCodeSegs, TUint &aTotalCodeSegListDescSize);
    
    virtual void ReadTraceBufferL(TDes8 &aTraceData );
    
    virtual void GetLocksL(TSCMLockData& aLockData);
    
    virtual void GetROMBuildInfoL(TRomHeaderData& aRomHeader);
    
	virtual TUint GetExceptionStackSizeL(const Debug::TArmProcessorModes aMode);

	virtual void GetExceptionStackL(const Debug::TArmProcessorModes aMode, TDes8& aStack, TUint aStartReadPoint);
	
	virtual TInt GetDataSourceFunctionality(TDes8& aFuncBuffer);

	virtual TInt GetDataSourceFunctionalityBufSize(TUint& aBufSize);

	virtual TInt GetVariantSpecificDataSize(TUint& aDataSize);

	virtual TInt GetVariantSpecificData(TDes8& aVarSpecData);

	virtual TVersion GetVersion() const;

private:

	TInt SetSimRegInfo( RRegisterList  &	aRegisterList );

	TInt SetSimRegValues( const TUint32    aThreadId,
		                  RRegisterList  &	aRegisterList );

	void PrintRegs( RRegisterList  &	aRegisterList );

};

//test consts defines
enum TTestExcCode 
	{ 
	EPrefetchAbort=0, 
	EDataAbort=1, 
	EUndefinedOpcode=2
	};
enum TTestExitType 
	{ 
	EException, 
	EPanic, 
	ETerminate, 
	EPending 
	}; 

enum TTestExitReason 
	{ 
	EUser, 
	EKernel, 
	ENone 
	};

const TInt KCrashFaultAddress = 0x01020304;
const TInt KCrashFaultStatus = 0x00011000;
const TInt32 KCrc32 = 0x1234FEDC;
const TCrashInfo::TCrashType KCrashType = TCrashInfo::ECrashKill;
const TTestExitReason KCrashReason = EKernel;
const TRmdArmExcInfo::TExceptionType KCrashExcCode = TRmdArmExcInfo::EPrefetchAbort;


const TBool KCrashXIP = EFalse;
_LIT( KCrashCategory, "crash_category" );
_LIT8( KCrashCategory8, "crash_category" );
_LIT( KCrashFileName, "crash_application.exe");
_LIT8( KCrashFileName8, "crash_application.exe");
const TUint64 KCrashTid = 0x0123;
const TUint64 KCrashPid = 0x0789;
const TUint64 KCrashTime = MAKE_TUINT64(0xFEDCBA98u,0x76543210u);
_LIT( KCrashThreadName, "crash-thread");
_LIT8( KCrashThreadName8, "crash-thread");
const TUint KCrashThreadsCount = 4;
const TUint KCrashThreadPriority = 42;
const TUint KCrashUsrStackAddr = 0x44332211;
const TUint KCrashUsrStackSize = 0x20;
const TUint KCrashSvcStackPtr = 0x11223344;
const TUint KCrashSvcStackAddr = 0x22334455;
const TUint KCrashSvcStackSize = 0x20;

const TUint KCrashCodeSize = 0x10;
const TUint KCrashCodeRunAddr = 0x01010101;
const TUint KCrashCodeLoadAddr = 0x02020202;
const TUint KCrashRODataSize = 0x20;
const TUint KCrashRODataRunAddr = 0x03030303;
const TUint KCrashRODataLoadAddr = 0x04040404;
const TUint KCrashDataSize = 0x30;
const TUint KCrashDataRunAddr = 0x05050505;
const TUint KCrashDataLoadAddr = 0x60606006;


_LIT( KCrashProcessName, "crash_application.exe");
_LIT( KLibName1, "application1.dll");
_LIT( KLibName2, "application2.dll");

_LIT8( KCrashProcessName8, "crash_application.exe");
_LIT8( KLibName1_8, "application1.dll");
_LIT8( KLibName2_8, "application2.dll");


const TUint KCoreRegsCount = 16;
const TUint KCoProRegsCount = 16;
const TUint KRegSubId = 0x11;
const TUint8 KRegValue8 = 0x10;
const TUint16 KRegValue16 = 0x3210;
const TUint32 KRegValue32 = 0x76543210;
const TUint64 KRegValue64 = MAKE_TUINT64(0xFEDCBA98u,0x76543210u);

const TUint KCodeSegsCount = 3;
const TUint KCodeSegBase = 0x01020304;
const TUint KCodeSegSize = 0x20;

const TUint KDataSegsCount = 7;
const TUint KDataSegBase = 0x09080706;
const TUint KDataSegSize = 0x40;

#endif // TEST_CRASH_DATA_SOURCE_H


