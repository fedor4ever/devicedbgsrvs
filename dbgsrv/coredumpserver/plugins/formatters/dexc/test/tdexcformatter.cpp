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

#include <e32std.h>
#include <e32base.h>
#include "tdexcformatter.h"
#include "dexcformatter.h"

CDexcFormatterTest * CDexcFormatterTest::NewL(RTest &itest)
{
    CDexcFormatterTest *self = CDexcFormatterTest::NewLC(itest);
    CleanupStack::Pop();
    return self;
}

CDexcFormatterTest * CDexcFormatterTest::NewLC(RTest &itest)
{
    CDexcFormatterTest *self = new(ELeave) CDexcFormatterTest(itest);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

void CDexcFormatterTest::ConstructL()
{
    server = new(ELeave) CTestCrashDataSource;
    formatter = CDexcFormatter::NewL();
    writer = CTestDataSave::NewL();
    formatter->ConfigureDataSourceL(server);
    formatter->ConfigureDataSaveL(writer);
}

CDexcFormatterTest::CDexcFormatterTest(RTest &aTest) : itest(aTest) { }

CDexcFormatterTest::~CDexcFormatterTest()
{
    delete server;
    delete writer;
    delete formatter;
}

void CDexcFormatterTest::CleanupEComArray(TAny* aArray)
{
	(static_cast<RImplInfoPtrArray*> (aArray))->ResetAndDestroy();
    REComSession::FinalClose();
}

void CDexcFormatterTest::TestSuiteL(TSuite aSuite) const
{
    RDebug::Print(_L("-> TestSuite(%d)\n"), aSuite);
    TInt err;
    TBuf<23> name(KTestName); 
    name.AppendNum(aSuite);
    itest.Next(name);

    switch(aSuite)
    {
        case EConfigData:
        {
            RDebug::Print(_L("EConfigData starting...\n"));
            TRAP(err, formatter->ConfigureDataSourceL(server));
            itest(err == KErrNone);
            itest(server == formatter->iDataSource);

            TRAP(err, formatter->ConfigureDataSaveL(writer));
            itest(err == KErrNone);
            itest(writer == formatter->iDataSave);

            TRAP(err, formatter->ConfigureDataSourceL(NULL)); 
            itest(err == KErrNoMemory);

            TRAP(err, formatter->ConfigureDataSaveL(NULL));
            itest(err == KErrNoMemory);
            RDebug::Print(_L("EConfigData passed!\n"));
            break;
        }

        case EFindImplementation:
        {
		
            RDebug::Print(_L("EFindImplementation starting...\n"));
            RImplInfoPtrArray infoArray;
            TCleanupItem cleanup(CDexcFormatterTest::CleanupEComArray, (TAny*)&infoArray);
            CleanupStack::PushL(cleanup);
            
            TRAPD(err, CDexcFormatter::ListAllImplementationsL(infoArray));
            itest(err == KErrNone);

			CCoreDumpFormatter* findImpFormatter = 0;            
            RBuf description;
            description.CreateL(MaxPluginDescription);
            CleanupClosePushL(description);

            err = KErrNotFound;
            for (TInt i = 0; i < infoArray.Count(); i++)
            {
                TUid uid = infoArray[i]->ImplementationUid();
                findImpFormatter  = CCoreDumpFormatter::NewL(uid);
				findImpFormatter->GetDescription(description);
                delete findImpFormatter;
                RDebug::Print(_L("found plugin[%d]: (%S)\n"), i, &description);
                if(description == KPluginDescription())
                {
                    itest(uid == KDexcFormatterUid);
                    err = KErrNone;
                    break;
                }
                description.Zero();
            }
            CleanupStack::PopAndDestroy();
            itest(err == KErrNone);
            CleanupStack::PopAndDestroy(); // infoArray, results in a call to CleanupEComArray
            break;

        }

        case EConfigParams:
        {
			RDebug::Print(_L("EConfigParams starting\n"));

			_LIT( KTrueFalseOpt, "True,False" );
			_LIT( KFalseOpt, "False" );
			_LIT( KTrueOpt, "True" );

            TInt err;
            COptionConfig * param = NULL;
		    TInt paramsCount = formatter->GetNumberConfigParametersL(); //it should be non leaving call
            itest(paramsCount == formatter->iConfigList.Count());

            TRAP(err, param = formatter->GetConfigParameterL(0));
            itest(param->Index() == CDexcFormatter::EDumpStack);
			itest(param->Uid() == KDEXCFormatterUid );
            itest(param->Source() == COptionConfig::EFormatterPlugin);
            itest(param->Type() == COptionConfig::ETBool);
            itest(param->Options() == KTrueFalseOpt);
            itest(param->ValueAsBool() == 1);
			param = NULL;

            TRAP(err, param = formatter->GetConfigParameterL(1));
            itest(param->Index() == CDexcFormatter::EAsciiStack);
			itest(param->Uid() == KDEXCFormatterUid );
            itest(param->Source() == COptionConfig::EFormatterPlugin);
            itest(param->Type() == COptionConfig::ETBool);
            itest(param->Options() == KTrueFalseOpt);
            itest(param->ValueAsBool() == 0);
			param = NULL;

			RDebug::Printf("->TRAP(err, param = formatter->GetConfigParameterL(paramsCount))\n");
            TRAP(err, param = formatter->GetConfigParameterL(paramsCount));
            itest(err == KErrBadHandle);
			param = NULL;

			RDebug::Printf("formatter->SetConfigParameterL( 0, (TInt)EFalse, KFalseOpt );\n");
		    TRAP(err, formatter->SetConfigParameterL( 0, (TInt)EFalse, KFalseOpt ));
			itest(err == KErrNone);
			itest(formatter->iConfigList[0]->ValueAsBool() == EFalse);
			TRAP(err, param = formatter->GetConfigParameterL(0));
            itest(formatter->iConfigList[0]->ValueAsBool() == EFalse);
            itest(param->ValueAsDesc() == KFalseOpt);

			RDebug::Printf("formatter->SetConfigParameterL( 0, (TInt)ETrue, KTrueOpt );\n");
		    TRAP(err, formatter->SetConfigParameterL( 0, (TInt)ETrue, KNullDesC ));
			itest(err == KErrNone);
			TRAP(err, param = formatter->GetConfigParameterL(0));
			itest(formatter->iConfigList[0]->ValueAsBool());
            itest(param->ValueAsDesc() == KTrueOpt);
            
			RDebug::Print(_L("EConfigParams finished\n"));

            break;
        }

        case EDumpLog:
        {           
			RDebug::Print(_L("EDumpLog starting.... \n"));
            //            0123456789012345678901234567890123456789012345678901234567890123456789012345
            _LIT8(KLine, "----------------------------------------------------------------------------");
            //                  0 12345678901234567890 1 23456789012345   6 78901234567890 1 2345678901234?
            _LIT8(KLogDump, "%S\r\nEKA2 USER CRASH LOG\r\nThread Name: %S\r\nThread ID: %u\r\nUser Stack %08X-%08X\r\n");
            TBuf8<65+76+KMaxName+11+16> reference; //line, thread name, id and 2x 8-digits

            //add newLC
            CThreadInfo *thread = CThreadInfo::NewL(KCrashTid,
                                                    KCrashThreadName,
                                                    KCrashPid,
                                                    KCrashThreadPriority,
                                                    KCrashSvcStackPtr,
                                                    KCrashSvcStackAddr,
                                                    KCrashSvcStackSize,
                                                    KCrashUsrStackAddr,
                                                    KCrashUsrStackSize);
            
			RDebug::Print(_L("  -> writer->OpenL(_L(\"\"), handle)\n"));
            writer->OpenL(_L("")); //clear writer buffer
            formatter->DumpLogL(*thread);
            delete thread;
            itest(writer->iData != KLogDump());
            reference.Format(KLogDump, &KLine, &KCrashThreadName8, KCrashTid, KCrashUsrStackAddr, KCrashUsrStackAddr + KCrashUsrStackSize);
            itest(writer->iData == reference);
			RDebug::Print(_L("EDumpLog finished\n"));
            break;
        }

        case EDumpPanicInfo:
        {
			RDebug::Print(_L("EDumpPanicInfo starting.... \n"));
            //                 0123456  7 8 9 0
			_LIT8(KPanicDump, "Panic: %S-%d\r\n");
            RBuf8 reference; 
            reference.CreateL(KPanicDump().Length() + 11 + KMaxExitCategoryName);
            CleanupClosePushL(reference);

            TCrashInfo crash;
            crash.iReason = KCrashReason;
            crash.iCategory = KCrashCategory;

            writer->OpenL(_L("")); //clear writer buffer
            formatter->DumpPanicInfoL(crash);
            itest(writer->iData != KPanicDump());

    		reference.Format(KPanicDump, &KCrashCategory8, KCrashReason);

            itest(writer->iData == reference);

            CleanupStack::PopAndDestroy(); //reference
			RDebug::Print(_L("EDumpPanicInfo finished\n"));
            break;
        }

        case EDumpExcInfo:
        {
			RDebug::Print(_L("EDumpExcInfo starting.... \n"));

            _LIT8(KExcDump, "\r\nUNHANDLED EXCEPTION:\r\ncode=%d PC=%08x FAR=%08x FSR=%08x\r\nR13svc=%08x R14svc=%08x SPSRsvc=%08x\r\n");
            //     012345 6 7890123456789012345678901234567890123 4 5
            RBuf8 reference;
            reference.CreateL(KExcDump().Length() + 11 + 6*4);
            CleanupClosePushL(reference);

            TCrashInfo crash;
			crash.iType = TCrashInfo::ECrashException;
			crash.iContext.iExcCode = KCrashExcCode;

            crash.iContext.iFaultAddress = KCrashFaultAddress;
            crash.iContext.iFaultStatus = KCrashFaultStatus;

			crash.iContext.iR15 = KRegValue32 + KCrashTid;
			crash.iContext.iR13Svc = crash.iContext.iR15 + 1;
			crash.iContext.iR14Svc = crash.iContext.iR15 + 2;
			crash.iContext.iSpsrSvc = crash.iContext.iR15 + 3;

            writer->Open(_L("")); //clear writer buffer
            formatter->DumpExcInfoL(crash);

			RDebug::Print( _L("-> itest(writer->iData != KExcDump())\n") );
            itest(writer->iData != KExcDump());

            TInt32 code = KCrashExcCode;
            TInt32 pc = KRegValue32 + KCrashTid; //index of r15
            TInt32 far = KCrashFaultAddress; 
            TInt32 fsr = KCrashFaultStatus;
            TInt32 r13svc = KRegValue32 + KCrashTid + 1; //index of r13svc
            TInt32 r14svc = KRegValue32 + KCrashTid + 2; //index of r14svc
            TInt32 spsr = KRegValue32 + KCrashTid + 3; //index of spsr 

            reference.Format(KExcDump, code, pc, far, fsr, r13svc, r14svc, spsr);

			/*
			RBuf8 referencePrint;
		    referencePrint.CreateL(256);
			referencePrint.Copy( reference );
			char *tonp = (char*) referencePrint.PtrZ();
			LOG_MSG2("  KExcDump=%s\n", tonp);
			referencePrint.Close();
			writer->Print();
			*/
			RDebug::Print( _L("-> itest(writer->iData == reference)\n") );
			itest(writer->iData == reference);

            CleanupStack::PopAndDestroy(); //reference
			RDebug::Print(_L("EDumpExcInfo finished\n"));

            break;
        }

        case EDumpRegisters:
        {
			RDebug::Print(_L("EDumpRegisters starting.... \n"));

            _LIT8(KCpsrDump, "\r\nUSER REGISTERS:\r\nCPSR=%08x\r\n");
            _LIT8(KRegsDump, "r%02d=%08x %08x %08x %08x\r\n");
            RBuf8 reference;
            reference.CreateL(KCpsrDump().Length() + 4 + KCoreRegsCount * (KRegsDump().Length() + 4*4));
            CleanupClosePushL(reference);

            writer->Open(_L("")); //clear writer buffer
            TRAP(err, formatter->DumpRegistersL(KCrashTid));
            itest(err == KErrNone);
            itest(writer->iData != KRegsDump());

            TInt32 cpsr = KRegValue32 + KCrashTid + 16; //index of cpsr 

            reference.Format(KCpsrDump, cpsr); 
            TBuf8<41> line;
            #define REG_VAL(i) ( KRegValue32 + KCrashTid + i )
            for(TInt i = 0; i < KCoreRegsCount; i+=4)
            {
                TInt reg0 = REG_VAL(i);
                TInt reg1 = REG_VAL(i+1);
                TInt reg2 = REG_VAL(i+2);
                TInt reg3 = REG_VAL(i+3);
                line.Format(KRegsDump, i, reg0, reg1, reg2, reg3);
                reference.Append(line);
            }
            #undef REG_VAL
            itest(writer->iData == reference);
            CleanupStack::PopAndDestroy(); //reference
			RDebug::Print(_L("EDumpRegisters finished\n"));

            break;
        }

        case EDumpCodeSegs:
        {
			RDebug::Print(_L("EDumpCodeSegs starting.... \n"));

            _LIT8(KSegsLine, "%08X-%08X %S\r\n");
            RBuf8 line;
            CleanupClosePushL(line);
            line.CreateL(KSegsLine().Length() + 2*4 + KMaxFileName);
            //               0123456789012345679012
	        _LIT8(KSegsDump, "\r\nCODE SEGMENTS:\r\n");
            RBuf8 reference;
            CleanupClosePushL(reference);
            reference.CreateL(KSegsDump().Length() + KCodeSegsCount * (line.MaxLength()) + 76);

            writer->Open(_L("")); //clear writer buffer
            formatter->DumpCodeSegsL(KCrashPid);
			//writer->Print();

            itest(writer->iData != KSegsDump());
			RDebug::Print( _L(" writer->iData.Length() = %d\n"), writer->iData.Length() );

            reference = KSegsDump();
            TUint base;
            TUint size;

			// See CTestCrashDataSource::GetCodeSegmentsL()
            
			base = KCrashCodeRunAddr + KCrashPid;
			size= KCrashCodeSize + KCrashPid;
            line.Format(KSegsLine, base, base + size, &KCrashProcessName8);
            reference.Append(line);

			base = KCrashCodeRunAddr + KCrashPid + 1;
			size= KCrashCodeSize + KCrashPid + 1;
            line.Format(KSegsLine, base, base + size, &KLibName1_8);
            reference.Append(line);

			base = KCrashCodeRunAddr + KCrashPid + 2;
			size= KCrashCodeSize + KCrashPid + 2;
            line.Format(KSegsLine, base, base + size, &KLibName2_8);
            reference.Append(line);
           
            reference.Append(_L8("\r\n"));

			/*
			RBuf8 referencePrint;
		    referencePrint.CreateL(256);
			referencePrint.Copy( reference );
			char *tonp = (char*) referencePrint.PtrZ();
			LOG_MSG2("  KCodeSeg=%s\n", tonp);
			referencePrint.Close();
			writer->Print();
			*/

			itest(writer->iData == reference);
            CleanupStack::PopAndDestroy(2);
			RDebug::Print(_L("EDumpCodeSegs finished\n"));
            break;
        }

        case EDumpStack:
        {
			RDebug::Print(_L("EDumpStack starting.... \n"));
			_LIT( KFalseOpt, "False" );

            CThreadInfo *thread = CThreadInfo::NewL(KCrashTid,
                                                    KCrashThreadName,
                                                    KCrashPid,
                                                    KCrashThreadPriority,
                                                    KCrashSvcStackPtr,
                                                    KCrashSvcStackAddr,
                                                    KCrashSvcStackSize,
                                                    KCrashUsrStackAddr,
                                                    KCrashUsrStackSize);

			CleanupStack::PushL(thread);

            RBuf8 stack;
            stack.CreateL(KCrashUsrStackSize*5);
            CleanupClosePushL(stack);
            
            writer->Open(_L("")); //clear writer buffer
		    formatter->SetConfigParameterL( 1, (TInt32)EFalse, KFalseOpt ); //binary dump
            formatter->DumpStackL(KCrashTid, *thread); 

            for( TInt i = 0; i < KCrashUsrStackSize; i++ )
		    {
                stack.Append((TUint8*)&i, 1);
            }

			RDebug::Printf("itest(writer->iData == stack);");
            itest(writer->iData == stack);
            itest(writer->iData != _L8(""));
            stack.Zero();

            for(TInt i = 0; i < KCrashUsrStackSize; i+=16)
            {
                stack.AppendNumFixedWidth(KCrashUsrStackAddr + i, EHex, 8);
                stack.Append(_L8(": "));
                for(TInt j = 0; j < 16; ++j)
                {
                    stack.AppendNumFixedWidth(i+j, EHex, 2); 
                    stack.Append(' ');
                }
                for(TInt j = 0; j < 16; ++j)
                {
                    TInt c = i+j;
                    if(c < 0x20 || c >= 0x7f) //something writable
						c = 0x2e; //.
                    stack.Append(TChar(c));
                }
                stack.AppendFormat(_L8("\r\n"));
            }

            writer->OpenL(_L("")); //clear writer buffer
			RDebug::Printf("formatter->SetConfigParameterL( 1, (TInt)ETrue, KNullDesC );");
		    formatter->SetConfigParameterL( 1, (TInt)ETrue, KNullDesC ); //ascii dump

			RDebug::Printf("formatter->DumpStackL(KCrashTid, *thread);");
            formatter->DumpStackL(KCrashTid, *thread); 

			RDebug::Printf("itest(writer->iData == stack);");
            itest(writer->iData == stack);
            itest(writer->iData != _L8(""));
            CleanupStack::PopAndDestroy();
			CleanupStack::PopAndDestroy(thread);
			RDebug::Print(_L("EDumpStack finished\n"));
            break;
        }
        
        default:
            break;
    }
    RDebug::Print(_L("<- TestSuite(%d)\n"), aSuite);
}

void RunTestsL()
{
    RDebug::Print(_L("-> RunTestsL()\n"));

    CDexcFormatterTest::TSuite suite[] = {
                                         CDexcFormatterTest::ENone,
                                         CDexcFormatterTest::EConfigData,
                                         CDexcFormatterTest::EFindImplementation,
                                         CDexcFormatterTest::EConfigParams,
                                         CDexcFormatterTest::EGetDescription,
                                         CDexcFormatterTest::EDumpLog,
                                         CDexcFormatterTest::EDumpPanicInfo,
                                         CDexcFormatterTest::EDumpExcInfo,
                                         CDexcFormatterTest::EDumpRegisters,
										 CDexcFormatterTest::EDumpCodeSegs,
										 CDexcFormatterTest::EDumpStack,
                                         CDexcFormatterTest::ECrash
                                         };

    RTest rtest(KTestTitle);
    rtest.Title(); 
    rtest.Start(_L(""));
    CDexcFormatterTest::TSuite *test = suite; 

    while(*test != CDexcFormatterTest::ECrash)
    {
        CDexcFormatterTest *itester = CDexcFormatterTest::NewLC(rtest);
        itester->TestSuiteL(*test++);
        CleanupStack::PopAndDestroy();
    }

    rtest.End();
    rtest.Close();
    RDebug::Print(_L("<- RunTestsL()\n"));
}

TInt E32Main()
{
    RDebug::Print(_L("-> E32Main()\n"));
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();
    __ASSERT_ALWAYS(cleanup, User::Panic(_L("D_EXC_TEST-NO_CLEANUP"), KErrNoMemory));

    TRAPD(err, RunTestsL());
	__ASSERT_ALWAYS(!err, User::Panic(_L("D_EXC_TEST-RunTestsL trapped with error: "), err));

	delete cleanup;
	__UHEAP_MARKEND;
    RDebug::Print(_L("<- E32Main()\n"));
    return err;
}

