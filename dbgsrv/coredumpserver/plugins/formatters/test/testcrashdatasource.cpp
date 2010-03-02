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

#include <e32debug.h>

#include <rm_debug_api.h>
#include "testcrashdatasource.h"
#include <debuglogging.h>

CTestCrashDataSource::CTestCrashDataSource( )
    {
	RDebug::Printf( "test_crashdatasource.cpp::CTestCrashDataSource::CTestCrashDataSource()\n" );
    }


// second-phase constructor
void CTestCrashDataSource::ConstructL()
    {
	RDebug::Printf( "test_crashdatasource.cpp::CTestCrashDataSource::ConstructL()\n" );
    }


// destructor
CTestCrashDataSource::~CTestCrashDataSource()
    {
	RDebug::Printf( "test_crashdatasource.cpp::CTestCrashDataSource::~CTestCrashDataSource()\n" );
    }



TInt CTestCrashDataSource::SetSimRegValues(  const TUint32    aThreadId,
						RRegisterList & aRegisterList )
	{

	RDebug::Print( _L("CTestCrashDataSource::SetSimRegValues()\n") );

	if( aRegisterList.Count() > 0 )
		{

		TUint8  val8  = (TUint8)(KRegValue8 + aThreadId);
		TUint16 val16 = (TUint16)(KRegValue16 + aThreadId); 
		TUint32 val32 = (TUint32)(KRegValue32 + aThreadId); 
		TUint64 val64 = (TUint64)(MAKE_TUINT64(0xFEDCBA98u,0x76543210u) + aThreadId); 

		for( TInt i = 0; i < aRegisterList.Count(); i ++ )
			{
			
			TRegisterData & reg = aRegisterList[i];

			switch ( reg.iSize )
				{
				case 0:
					reg.iValue8  =  val8 + i;
					RDebug::Printf( " Setting reg[%d] iValue8=0x%X\n", i, reg.iValue8 ); 
					break;
				case 1:
					reg.iValue16 =  val16+ i;
					RDebug::Printf( " Setting reg[%d] iValue16=0x%X\n", i, reg.iValue16 ); 
					break;
				case 2:
					reg.iValue32 =  val32+ i;
					RDebug::Printf( " Setting reg[%d] iValue32=0x%X\n", i, reg.iValue32 ); 
					break;
				case 3:
					reg.iValue64 =  val64+ i;
					RDebug::Printf( " Setting reg[%d]=0x%X%X\n", i,
						I64HIGH(reg.iValue64), I64LOW(reg.iValue64) ); 
					break;
				}

            //reg.iAvailable = i%8 ? ETrue : EFalse;
            /*
			if ( 0 == ((i+1) % 8) )
				{
				RDebug::Printf( "   setting reg[%d] as not available\n", i );
				reg.iAvailable = EFalse;
				}
            */
			}

		return KErrNone;

		}

	return KErrBadHandle;

	}


void CTestCrashDataSource::PrintRegs( RRegisterList & aRegisterList )
	{
	RDebug::Printf( "CTestCrashDataSource::PrintRegs()\n" );

	if( aRegisterList.Count() > 0 )
		{
		for( TInt i = 0; i < aRegisterList.Count(); i ++ )
			{			
			TRegisterData & reg = aRegisterList[i];

			RDebug::Printf( " reg[%d] iRegClass=%d, iId=%d, iSubId=%d, iSize=%d, iAvailable=", 
				i, reg.iRegClass, reg.iId, reg.iSubId, reg.iSize );
            if(reg.iAvailable)
                RDebug::Printf( "ETrue\n" );
            else
                RDebug::Printf( "EFalse\n" );

			switch ( reg.iSize )
				{
				case 0:
					RDebug::Printf( "  iValue8=0x%X\n", reg.iValue8 ); break;
				case 1:
					RDebug::Printf( "  iValue16=0x%X\n", reg.iValue16 ); break;
				case 2:
					RDebug::Printf( "  iValue32=0x%X\n", reg.iValue32 ); break;
				case 3:
					RDebug::Printf( "  iValue64=0x%X%X\n", 
						I64HIGH(reg.iValue64), I64LOW(reg.iValue64) ); 
					break;
				}
			}
		}
	else
		{
		RDebug::Printf( " No registers to print\n" );
		}
	}


void CTestCrashDataSource::GetRegisterListL( RRegisterList & aRegisterList )
{

	LOG_MSG("CTestCrashDataSource::GetRegisterListL()\n");

    TFunctionalityRegister ids[] = {ERegisterR0,
                                    ERegisterR1,
                                    ERegisterR2,
                                    ERegisterR3,
                                    ERegisterR4,
                                    ERegisterR5,
                                    ERegisterR6,
                                    ERegisterR7,
                                    ERegisterR8,
                                    ERegisterR9,
                                    ERegisterR10,
                                    ERegisterR11,
                                    ERegisterR12,
                                    ERegisterR13,
                                    ERegisterR14,
                                    ERegisterR15,
                                    ERegisterCpsr,
                                    ERegisterR13Svc,
                                    ERegisterR14Svc,
                                    ERegisterSpsrSvc,
                                    ERegisterR13Abt,
                                    ERegisterR14Abt,
                                    ERegisterSpsrAbt,
                                    ERegisterR13Und,
                                    ERegisterR14Und,
                                    ERegisterSpsrUnd,
                                    ERegisterR13Irq,
                                    ERegisterR14Irq,
                                    ERegisterSpsrIrq,
                                    ERegisterR8Fiq,
                                    ERegisterR9Fiq,
                                    ERegisterR10Fiq,
                                    ERegisterR11Fiq,
                                    ERegisterR12Fiq,
                                    ERegisterR13Fiq,
                                    ERegisterR14Fiq,
                                    ERegisterSpsrFiq,
                                    ERegisterLast
                                                    };

    aRegisterList.Reset();

    TRegisterData reg;
    //core regs
    const TFunctionalityRegister* r;
    for(r = ids; *r != ERegisterLast; ++r)
    {
        reg.iRegClass  = 0; //Core
        reg.iId        = *r; // R0, ...  
        reg.iSubId     = 0; // not used for Core Regs
        reg.iSize      = 2;
        reg.iAvailable = ETrue;
        aRegisterList.AppendL( reg );

    }

    //copro regs
    for(TInt i = 0; i < KCoProRegsCount; ++i)
    {
        reg.iRegClass  = 1; //CoPro
        reg.iId        = i; // R0, ...  
        reg.iSubId     = KRegSubId + i; 
        reg.iSize      = i%4;
        reg.iAvailable = i < KCoProRegsCount/2 ? ETrue : EFalse; //half not available
        aRegisterList.AppendL( reg );
    }
}

void CTestCrashDataSource::ReadRegistersL( const TUint64 aThreadId, RRegisterList &aRegisterList )
	{

	//RDebug::Print( _L("CTestCrashDataSource::ReadRegisters( threadId=0x%X)\n"), I64LOW(aThreadId) );

	if( aRegisterList.Count() )
		{
		RDebug::Printf( "Register list already supplied\n" );
        User::LeaveIfError(SetSimRegValues( aThreadId, aRegisterList ));
		}
	else
		{
		RDebug::Printf( "Creating register list\n" );
	    GetRegisterListL( aRegisterList );
        User::LeaveIfError(SetSimRegValues( aThreadId, aRegisterList ));
		}

	PrintRegs( aRegisterList );

	}



void CTestCrashDataSource::ReadMemoryL(
                   const TUint64    aThreadId,
                   const TUint32    aAddress,
                   const TUint32    aLength,
                   TDes8          & aData )
	{

	RDebug::Print( _L("CCrashDataSource::ReadMemory( tId=0x%X, addr=0x%X, len=0x%X)\n"), 
		aThreadId, aAddress, aLength );

	for( TUint32 i = 0; i < aLength; i++ )
		{
        aData.Append((TUint8*)&i, 1);
		RDebug::Printf( " Memory[%d]=0x%X\n", i, aData[i] );
		}
	}




void CTestCrashDataSource::GetProcessListL( RProcessPointerList & aProcList, 
									        TUint & aTotalProcessListDescSize )
	{

	RDebug::Print( _L("CTestCrashDataSource::GetProcessList( )\n") );

    CProcessInfo *processInfo = CProcessInfo::NewL(KCrashPid, KCrashFileName );

    TInt err = aProcList.Append( processInfo ); 
    if(err != KErrNone)
        {
        delete processInfo;
        User::Leave(err);
        }
    }

void CTestCrashDataSource::GetExecutableListL( RExecutablePointerList & aExecutableList, 
									        TUint & aTotalExecutableListDescSize )
	{

	RDebug::Print( _L("CTestCrashDataSource::GetExecutableList( )\n") );

    CExecutableInfo *executableInfo = CExecutableInfo::NewL(KCrashFileName, EFalse, EFalse );

    TInt err = aExecutableList.Append( executableInfo ); 
    if(err != KErrNone)
        {
        delete executableInfo;
        User::Leave(err);
        }
    }

void CTestCrashDataSource::GetThreadListL(	const TUint64 aProcessId, 
									RThreadPointerList & aThreadList,
									TUint & aTotalThreadListDescSize )
	{
	GetThreadListL( I64LOW( aProcessId ), aThreadList, aTotalThreadListDescSize );
	}


void CTestCrashDataSource::GetThreadListL( const TUint32 aProcessId, 
									RThreadPointerList & aThreadList,
									TUint & aTotalThreadListDescSize )
	{

	RDebug::Print( _L("CTestCrashDataSource::GetThreadList( pId=0x%X)\n"), aProcessId );



	LOG_MSG2( "CTestCrashDataSource::GetThreadListL( pId=0x%X)\n", aProcessId );

	aThreadList.ResetAndDestroy();
	aTotalThreadListDescSize = 0;

        for(TInt i =0; i < KCrashThreadsCount; ++i)
        {
            CThreadInfo *thread = CThreadInfo::NewL( KCrashTid + i,
                                        KCrashThreadName,
                                        KCrashPid,
                                        KCrashThreadPriority + i,
                                        KCrashSvcStackPtr + i,
                                        KCrashSvcStackAddr + i,
                                        KCrashSvcStackSize + i,
                                        KCrashUsrStackAddr + i,
                                        KCrashUsrStackSize + i);
            aThreadList.Append(thread);
            //delete thread;
        }

	}

void CTestCrashDataSource::GetCodeSegmentsL( const TUint64 aTid, RCodeSegPointerList &aCodeSegs, TUint &aTotalCodeSegListDescSize )
	{

	RDebug::Printf( "CTestCrashDataSource::GetExecutableInfo(pId=0x%X)\n", aTid );
	TInt err;
    
    TCodeSegInfo *segInfo;

    if(KCrashXIP)
        RDebug::Printf( " Exec info XIP == ETrue\n" );
    else
        RDebug::Printf( " Exec info XIP == EFalse\n" );

    
	segInfo = new(ELeave) TCodeSegInfo;
	segInfo->iName			 = KCrashFileName;
    segInfo->iXIP			 = ETrue;
    segInfo->iCodeSize       = KCrashCodeSize + aTid;
    segInfo->iCodeRunAddr    = KCrashCodeRunAddr + aTid;
    segInfo->iCodeLoadAddr   = KCrashCodeLoadAddr + aTid;
    segInfo->iRoDataSize     = KCrashRODataSize + aTid;
    segInfo->iRoDataRunAddr  = KCrashRODataRunAddr + aTid;
    segInfo->iRoDataLoadAddr = KCrashRODataLoadAddr + aTid;
    segInfo->iDataSize       = KCrashDataSize + aTid;
    segInfo->iDataRunAddr    = KCrashDataRunAddr + aTid;
    segInfo->iDataLoadAddr   = KCrashDataLoadAddr + aTid;
	segInfo->iType			 = EExeCodeSegType;
    err = aCodeSegs.Append(segInfo);
	if(err != KErrNone)
		{
        delete segInfo;
        User::Leave(err);
        }
	aTotalCodeSegListDescSize += sizeof(TCodeSegInfo);

	segInfo = new(ELeave) TCodeSegInfo;
	segInfo->iName			 = KLibName1;
    segInfo->iXIP			 = ETrue;
    segInfo->iCodeSize       = KCrashCodeSize + aTid + 1;
    segInfo->iCodeRunAddr    = KCrashCodeRunAddr + aTid + 1;
    segInfo->iCodeLoadAddr   = KCrashCodeLoadAddr + aTid + 1;
    segInfo->iRoDataSize     = KCrashRODataSize + aTid + 1;
    segInfo->iRoDataRunAddr  = KCrashRODataRunAddr + aTid + 1;
    segInfo->iRoDataLoadAddr = KCrashRODataLoadAddr + aTid + 1;
    segInfo->iDataSize       = KCrashDataSize + aTid + 1;
    segInfo->iDataRunAddr    = KCrashDataRunAddr + aTid + 1;
    segInfo->iDataLoadAddr   = KCrashDataLoadAddr + aTid + 1;
	segInfo->iType			 = EDllCodeSegType;
    err = aCodeSegs.Append(segInfo);
	if(err != KErrNone)
		{
        delete segInfo;
        User::Leave(err);
        }
	aTotalCodeSegListDescSize += sizeof(TCodeSegInfo);

	segInfo = new(ELeave) TCodeSegInfo;
	segInfo->iName			 = KLibName2;
    segInfo->iXIP			 = EFalse;
    segInfo->iCodeSize       = KCrashCodeSize + aTid + 2;
    segInfo->iCodeRunAddr    = KCrashCodeRunAddr + aTid + 2;
    segInfo->iCodeLoadAddr   = KCrashCodeLoadAddr + aTid + 2;
    segInfo->iRoDataSize     = KCrashRODataSize + aTid + 2;
    segInfo->iRoDataRunAddr  = KCrashRODataRunAddr + aTid + 2;
    segInfo->iRoDataLoadAddr = KCrashRODataLoadAddr + aTid + 2;
    segInfo->iDataSize       = KCrashDataSize + aTid + 2;
    segInfo->iDataRunAddr    = KCrashDataRunAddr + aTid + 2;
    segInfo->iDataLoadAddr   = KCrashDataLoadAddr + aTid + 2;
	segInfo->iType			 = EDllCodeSegType;
    err = aCodeSegs.Append(segInfo);
	if(err != KErrNone)
		{
        delete segInfo;
        User::Leave(err);
        }
	aTotalCodeSegListDescSize += sizeof(TCodeSegInfo);

	}

void CTestCrashDataSource::ReadTraceBufferL(TDes8 &aTraceData )
	{
	User::Leave(KErrNotSupported);
	}

void CTestCrashDataSource::GetLocksL(TSCMLockData& aLockData)
	{
	User::Leave(KErrNotSupported);
	}

void CTestCrashDataSource::GetROMBuildInfoL(TRomHeaderData& aRomHeader)
	{
	User::Leave(KErrNotSupported);
	}

TUint CTestCrashDataSource::GetExceptionStackSizeL(const Debug::TArmProcessorModes aMode)
	{
	User::Leave(KErrNotSupported);
	return 0;
	}

void CTestCrashDataSource::GetExceptionStackL(const Debug::TArmProcessorModes aMode, TDes8& aStack, TUint aStartReadPoint)
	{
	User::Leave(KErrNotSupported);
	}

TInt CTestCrashDataSource::GetDataSourceFunctionality(TDes8& aFuncBuffer)
	{
	return KErrNotSupported;
	}

TInt CTestCrashDataSource::GetDataSourceFunctionalityBufSize(TUint& aBufSize)
	{
	return KErrNotSupported;
	}

TVersion CTestCrashDataSource::GetVersion() const
	{
	TVersion ver(2,0,0);
	return ver;
	}

TInt CTestCrashDataSource::GetVariantSpecificDataSize(TUint& aDataSize)
	{
	aDataSize = 0;
	return KErrNotSupported;
	}

TInt CTestCrashDataSource::GetVariantSpecificData(TDes8& aVarSpecData)
	{
	(void)aVarSpecData;
	return KErrNotSupported;
	}

//eof

