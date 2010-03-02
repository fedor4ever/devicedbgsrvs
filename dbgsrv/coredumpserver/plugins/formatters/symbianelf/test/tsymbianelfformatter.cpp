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
//

#include <e32base.h>

#include "symbianelfformatter.h"
#include "tsymbianelfformatter.h"


CSymbianElfFormatterTest * CSymbianElfFormatterTest::NewL(RTest &itest)
{
    CSymbianElfFormatterTest *self = CSymbianElfFormatterTest::NewLC(itest);
    CleanupStack::Pop();
    return self;
}

CSymbianElfFormatterTest * CSymbianElfFormatterTest::NewLC(RTest &itest)
{
    CSymbianElfFormatterTest *self = new(ELeave) CSymbianElfFormatterTest(itest);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

void CSymbianElfFormatterTest::ConstructL()
{
    server = new(ELeave) CTestCrashDataSource;
    formatter = CSymbianElfFormatter::NewL();
	iStrInfoTbl = CStringInfoTable::NewL();
    writer = CTestDataSave::NewL();
    formatter->ConfigureDataSaveL(writer);
    formatter->ConfigureDataSourceL(server);
}

CSymbianElfFormatterTest::CSymbianElfFormatterTest(RTest &aTest) : itest(aTest) { }

CSymbianElfFormatterTest::~CSymbianElfFormatterTest()
{
    delete server;
    delete writer;
    delete iStrInfoTbl;
    delete formatter;
}

void CSymbianElfFormatterTest::CleanupEComArray(TAny* aArray)
{
	(static_cast<RImplInfoPtrArray*> (aArray))->ResetAndDestroy();
    REComSession::FinalClose();
}

void CSymbianElfFormatterTest::TestSuiteL(TSuite aSuite)
{
    RDebug::Print(_L("-> TestSuite(%d)\n"), aSuite);
    TInt err;
    //TInt handle;
    //           123456789012345678901
    TBuf<23> name(KTestName); 
    name.AppendNum(aSuite);
    itest.Next(name);

	iCrashInfo.iType = TCrashInfo::ECrashException;
	iCrashInfo.iExcNumber = KCrashExcCode;
	iCrashInfo.iTime = KCrashTime;
	iCrashInfo.iPid = KCrashPid;
	iCrashInfo.iTid = KCrashTid;

	iCrashInfo.iContext.iFaultAddress = KCrashFaultAddress;
	iCrashInfo.iContext.iFaultStatus = KCrashFaultStatus;
	

	iCrashInfo.iContext.iR15 = KRegValue32 + KCrashTid;
	iCrashInfo.iContext.iR13Svc = iCrashInfo.iContext.iR15 + 1;
	iCrashInfo.iContext.iR14Svc = iCrashInfo.iContext.iR15 + 2;
	iCrashInfo.iContext.iSpsrSvc = iCrashInfo.iContext.iR15 + 3;

	if( iStrInfoTbl )
		delete iStrInfoTbl;

	iStrInfoTbl = CStringInfoTable::NewLC();
	formatter->iStrInfoTbl = iStrInfoTbl;

    RBuf8 reference;
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
            TCleanupItem cleanup(CSymbianElfFormatterTest::CleanupEComArray, (TAny*)&infoArray);
            CleanupStack::PushL(cleanup);
            
            TRAPD(err, CSymbianElfFormatter::ListAllImplementationsL(infoArray));
            itest(err == KErrNone);
            
            CCoreDumpFormatter* formatter = 0;
            RBuf description;
            description.CreateL(MaxPluginDescription);
            CleanupClosePushL(description);

            err = KErrNotFound;
            for (TInt i = 0; i < infoArray.Count(); i++)
            {
                TUid uid = infoArray[i]->ImplementationUid();
                formatter = CCoreDumpFormatter::NewL(uid);
                formatter->GetDescription(description);
                delete formatter;
                RDebug::Print(_L("found plugin[%d]: (%S)\n"), i, &description);
                if(description == KPluginDescription())
                {
                    itest(uid == KSymbianElfFormatterUid);
                    err = KErrNone;
                    break;
                }
                description.Zero();
            }
            CleanupStack::PopAndDestroy();
            description.Close();
            itest(err == KErrNone);
            CleanupStack::PopAndDestroy(); // infoArray, results in a call to CleanupEComArray
            break;
        }
        case EWriteSymbianInfo:
        {

			RDebug::Print(_L("EWriteSymbianInfo starting...\n"));
            TRAP(err, formatter->ConfigureDataSourceL(server));
            itest(err == KErrNone);
            itest(server == formatter->iDataSource);

            TRAP(err, formatter->ConfigureDataSaveL(writer));
            itest(err == KErrNone);
            itest(writer == formatter->iDataSave);

            reference.CreateL( sizeof(Sym32_dhdr) + sizeof(Sym32_syminfod) );
            CleanupClosePushL(reference);

            TPtrC8 ptr;
            Sym32_dhdr dhdr;
            ptr.Set(reinterpret_cast<TUint8*>(&dhdr), sizeof(Sym32_dhdr));
            TInt32 index = iStrInfoTbl->GetIndex(KSymbian);
            dhdr.d_name	= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
            dhdr.d_descrsz	= sizeof(Sym32_syminfod);
            dhdr.d_type	= ESYM_NOTE_SYM;
            dhdr.d_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
            dhdr.d_elemnum = 1;
            reference.Append(ptr);

            Sym32_syminfod seg;
            ptr.Set(reinterpret_cast<TUint8*>(&seg), sizeof(Sym32_syminfod));
            
            seg.sd_date_time = (Elf64_Word)KCrashTime;
            seg.sd_exit_type = KCrashType;
            seg.sd_exit_reason = KCrashReason;
            seg.sd_thread_id = KCrashTid;
            seg.sd_proc_id = KCrashPid;
        
            TBuf8<KMaxExitCategoryName> buf;
            buf.Copy(KCrashCategory);
            if (buf.Length() > 0)
            {
                iStrInfoTbl->AddStringL(buf);
            }
            index = iStrInfoTbl->GetIndex(buf);
            seg.sd_exit_cat = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;

            Sym32_execid id = {I64LOW(KCrashTime), KCrc32};            
            seg.sd_execid = id;
            
            reference.Append(ptr);

            writer->OpenL(_L("") ); //clear writer buffer

			formatter->iOffset = 0;
            TInt offset = 0;
            
			RDebug::Print(_L("->formatter->WriteSymbianInfoL( )\n"));

			formatter->WriteSymbianInfoL( iCrashInfo, KCrc32 );

			RDebug::Print(_L("<-formatter->WriteSymbianInfoL( )\n"));
            itest(offset + sizeof(Sym32_dhdr) + sizeof(Sym32_syminfod) == formatter->iOffset);
			itest(offset != formatter->iOffset);
			
			//RDebug::Print(_L("itest3\n"));
            //itest(reference == writer->iData);
			//writer->Print();						

            itest(reference != _L8(""));

            CleanupStack::PopAndDestroy(&reference);
            break;
        }
        case EWriteCodeSegments:
        {

			RDebug::Print(_L("EWriteCodeSegments starting...\n"));
            TRAP(err, formatter->ConfigureDataSourceL(server));
            itest(err == KErrNone);
            itest(server == formatter->iDataSource);

            TRAP(err, formatter->ConfigureDataSaveL(writer));
            itest(err == KErrNone);
            itest(writer == formatter->iDataSave);

			RCodeSegPointerList codeSegs;
			RArray<TUint> codeSegOffsets;
			CleanupClosePushL(codeSegOffsets);
			TUint totalCodeSegListDescSize;

			RDebug::Print(_L("server->GetCodeSegmentsL( KCrashTid, codeSegs );\n"));
			server->GetCodeSegmentsL( KCrashTid, codeSegs, totalCodeSegListDescSize );

			formatter->iOffset = 0;

			writer->OpenL(_L("") ); //clear writer buffer

			RDebug::Print(_L("formatter->WriteCodeSegmentsL( KCrashTid, codeSegs, codeSegOffsets );\n"));
            formatter->WriteCodeSegmentsL( KCrashTid, codeSegs, codeSegOffsets );

			RDebug::Print(_L("itest(codeSegOffsets.Count() == KCodeSegsCount );\n"));
			itest(codeSegOffsets.Count() == KCodeSegsCount );

			for(TInt i = 0; i < KCodeSegsCount; i++)
            {
				RDebug::Printf( "itest( codeSegs[i] = %d );\n", codeSegOffsets[i] );
                itest( codeSegOffsets[i] == 0 );
            }

			RDebug::Printf( "itest(0 == formatter->iOffset);" );
            itest(0 == formatter->iOffset);
			
			RDebug::Printf( "CleanupStack::PopAndDestroy(&codeSegOffsets);" );
			CleanupStack::PopAndDestroy(&codeSegOffsets);
			RDebug::Printf( "CleanupStack::PopAndDestroy(&codeSegs);" );
            codeSegs.ResetAndDestroy();

            break;
        }
        case EWriteDataSegments:
        {
			RDebug::Print(_L("EWriteDataSegments starting...\n"));
            TRAP(err, formatter->ConfigureDataSourceL(server));
            itest(err == KErrNone);
            itest(server == formatter->iDataSource);

            TRAP(err, formatter->ConfigureDataSaveL(writer));
            itest(err == KErrNone);
            itest(writer == formatter->iDataSave);

            reference.CreateL(KDataSegsCount * (KDataSegSize + KCrashPid + KDataSegsCount*(KDataSegsCount+1)/2) ); 
            CleanupClosePushL(reference);

            RBuf8 buf;
            buf.CreateL((KDataSegSize + KCrashPid + KDataSegsCount));
            for(TInt i = 0; i < KDataSegSize + KCrashPid + KDataSegsCount; ++i)
                    buf.Append((TUint8*)&i, 1);

            for(TInt i = 0; i < KDataSegsCount; ++i)
            {
                reference.Append(buf.Ptr(), KDataSegSize + KCrashPid + i);
            }
            buf.Close();

			formatter->iOffset = 0;
            TInt offset = formatter->iOffset;

			RThreadPointerList *threadList = new(ELeave)RThreadPointerList ;
			formatter->iDataSource->GetThreadListL((TUint64)KCrashPid, *threadList);

			RCodeSegPointerList codeSegs;
			CleanupClosePushL(codeSegs);
			CSymbianElfFormatter::RSegmentList dataSegs;
			CleanupClosePushL(dataSegs);
			RArray<TUint> dataSegOffsets;
			CleanupClosePushL(dataSegOffsets);

			writer->OpenL(_L("") ); //clear writer buffer

            formatter->WriteDataSegmentsL( 
				KCrashTid, 
				KCrashPid, 
				*threadList, 
				codeSegs, 
				dataSegs, 
				dataSegOffsets );

			//RDebug::Print(_L("itest1\n"));
            //itest(offset + reference.Length() == formatter->iOffset);

			RDebug::Print(_L("itest2\n"));
            itest(offset != formatter->iOffset);

			//RDebug::Print(_L("itest3\n"));
            //itest(reference == writer->iData);

			// Note that the number of sements is not the same as KDataSegsCount, 
			//since we also create thread stack segments

			RDebug::Print(_L("itest4\n"));
            itest(reference != _L8(""));

			threadList->ResetAndDestroy();
			threadList->Close();
			delete threadList;

			CleanupStack::PopAndDestroy(&dataSegOffsets);
			CleanupStack::PopAndDestroy(&dataSegs);
			CleanupStack::PopAndDestroy(&codeSegs);

            CleanupStack::PopAndDestroy(); //reference

            break;
        }
        case EWriteThreadsInfo:
        {

			RDebug::Print(_L("EWriteThreadsInfo  starting...\n"));
            TRAP(err, formatter->ConfigureDataSourceL(server));
            itest(err == KErrNone);
            itest(server == formatter->iDataSource);

            TRAP(err, formatter->ConfigureDataSaveL(writer));
            itest(err == KErrNone);
            itest(writer == formatter->iDataSave);

            reference.CreateL( sizeof(Sym32_dhdr) + sizeof(Sym32_thrdinfod)*KCrashThreadsCount );
            CleanupClosePushL(reference);

            TPtrC8 ptr;
            Sym32_dhdr dhdr;
            ptr.Set(reinterpret_cast<TUint8*>(&dhdr), sizeof(Sym32_dhdr));
            TInt32 index = iStrInfoTbl->GetIndex(KThread);
            dhdr.d_name	= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
            dhdr.d_descrsz	= sizeof(Sym32_thrdinfod);
            dhdr.d_type	= ESYM_NOTE_THRD;
            dhdr.d_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
            dhdr.d_elemnum = KCrashThreadsCount;
            reference.Append(ptr);

			RThreadPointerList *threadList = new(ELeave) RThreadPointerList;
			formatter->iDataSource->GetThreadListL(KCrashPid, *threadList);

            Sym32_thrdinfod seg;
            ptr.Set(reinterpret_cast<TUint8*>(&seg), sizeof(Sym32_thrdinfod));

            for(TInt i = 0; i < KCrashThreadsCount; ++i)
				{
				seg.td_id = KCrashTid + i;
				seg.td_owning_process = KCrashPid;
				seg.td_priority = KCrashThreadPriority + i;
				seg.td_svc_sp = KCrashSvcStackPtr + i;
				seg.td_svc_stack = KCrashSvcStackAddr + i;
				seg.td_svc_stacksz = KCrashSvcStackSize + i;
				seg.td_usr_stack = KCrashUsrStackAddr + i;
				seg.td_usr_stacksz = KCrashUsrStackSize + i;

				TBuf8<KMaxProcessName> buf;
				buf.Copy(KCrashThreadName);
				if (buf.Length() > 0)
				{
					iStrInfoTbl->AddStringL(buf);
				}

				index = iStrInfoTbl->GetIndex(buf);
				seg.td_name = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;

				reference.Append(ptr);

				}

            writer->OpenL(_L("") ); //clear writer buffer

			formatter->iOffset = 0;
            TInt offset = 0;

            formatter->WriteThreadInfoL(KCrashPid, *threadList);

			RDebug::Print(_L("itest1\n"));
            itest(offset + reference.Length() == formatter->iOffset);
			RDebug::Print(_L("itest2\n"));
            itest(offset != formatter->iOffset);

			//RDebug::Print(_L("itest3\n"));
            //itest(reference == writer->iData);

			RDebug::Print(_L("itest4\n"));
            itest(reference != _L8(""));

			threadList->ResetAndDestroy();
			threadList->Close();
			delete threadList;

            CleanupStack::PopAndDestroy(); //reference

            break;
        }
        case EWriteProcessInfo:
        {
			RDebug::Print(_L("EWriteProcessInfo starting...\n"));
            TRAP(err, formatter->ConfigureDataSourceL(server));
            itest(err == KErrNone);
            itest(server == formatter->iDataSource);

            TRAP(err, formatter->ConfigureDataSaveL(writer));
            itest(err == KErrNone);
            itest(writer == formatter->iDataSave);

            reference.CreateL( sizeof(Sym32_dhdr) + sizeof(Sym32_procinfod) );
            CleanupClosePushL(reference);

			RProcessPointerList *processList = new(ELeave)RProcessPointerList;
			formatter->iDataSource->GetProcessListL( *processList );

            TPtrC8 ptr;
            Sym32_dhdr dhdr;
            ptr.Set(reinterpret_cast<TUint8*>(&dhdr), sizeof(Sym32_dhdr));
            TInt32 index = iStrInfoTbl->GetIndex(KProcess);
            dhdr.d_name	= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
            dhdr.d_descrsz	= sizeof(Sym32_procinfod);
            dhdr.d_type	= ESYM_NOTE_PROC;
            dhdr.d_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
            dhdr.d_elemnum = 1;
            reference.Append(ptr);

            Sym32_procinfod seg;
            ptr.Set(reinterpret_cast<TUint8*>(&seg), sizeof(Sym32_procinfod));
            
            seg.pd_id = KCrashPid;

            TBuf8<KMaxProcessName> buf;
            buf.Copy(KCrashProcessName);
            if (buf.Length() > 0)
            {
                iStrInfoTbl->AddStringL(buf);
            }
            index = iStrInfoTbl->GetIndex(buf);
            seg.pd_name = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
            reference.Append(ptr);

            writer->OpenL(_L("") ); //clear writer buffer

			formatter->iOffset = 0;
            TInt offset = 0;

            formatter->WriteProcessInfoL(KCrashPid, *processList );

			RDebug::Print(_L("itest1\n"));
            itest(offset + reference.Length() == formatter->iOffset);

			RDebug::Print(_L("itest2\n"));
            itest(offset != formatter->iOffset);

			//RDebug::Print(_L("itest3\n"));
            //itest(reference == writer->iData);

			RDebug::Print(_L("itest4\n"));
            itest(reference != _L8(""));

			processList->ResetAndDestroy();
			processList->Close();
			delete processList;

            CleanupStack::PopAndDestroy(&reference); //reference

            break;
        }

        case EWriteExecutableInfo:
        {
			RDebug::Print(_L("EWriteExecutableInfo starting...\n"));
            TRAP(err, formatter->ConfigureDataSourceL(server));
            itest(err == KErrNone);
            itest(server == formatter->iDataSource);

            TRAP(err, formatter->ConfigureDataSaveL(writer));
            itest(err == KErrNone);
            itest(writer == formatter->iDataSave);

            reference.CreateL(sizeof(Sym32_dhdr) + KCodeSegsCount * sizeof(Sym32_execinfod) );
            CleanupClosePushL(reference);

            TPtrC8 ptr;
            Sym32_dhdr dhdr;
            ptr.Set(reinterpret_cast<TUint8*>(&dhdr), sizeof(Sym32_dhdr));
            TInt32 index = iStrInfoTbl->GetIndex(KExecutable);
            dhdr.d_name	= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
            dhdr.d_descrsz	= sizeof(Sym32_execinfod);
            dhdr.d_type	= ESYM_NOTE_EXEC;
            dhdr.d_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
            dhdr.d_elemnum = KCodeSegsCount;
            reference.Append(ptr);

            Sym32_execinfod seg;
            ptr.Set(reinterpret_cast<TUint8*>(&seg), sizeof(Sym32_execinfod));

            Sym32_execid id = {I64LOW(KCrashTime), KCrc32};
            
            seg.ed_execid = id;
            seg.ed_XIP = KCrashXIP;

            for(TInt i = 0; i < KCodeSegsCount; i++)
            {
                if(i == 2)
                    seg.ed_XIP = !KCrashXIP;

                seg.ed_codesize = KCrashCodeSize + KCrashPid + i;
                seg.ed_coderunaddr = KCrashCodeRunAddr + KCrashPid + i;
                seg.ed_codeloadaddr = KCrashCodeLoadAddr + KCrashPid + i;
                seg.ed_rodatasize = KCrashRODataSize + KCrashPid + i;
                seg.ed_rodatarunaddr = KCrashRODataRunAddr + KCrashPid + i;
                seg.ed_rodataloadaddr = KCrashRODataLoadAddr + KCrashPid + i;
                seg.ed_datasize = KCrashDataSize + KCrashPid + i;
                seg.ed_datarunaddr = KCrashDataRunAddr + KCrashPid + i;
                seg.ed_dataloadaddr = KCrashDataLoadAddr + KCrashPid + i;
                TBuf8<KMaxFileName> buf;
                switch(i)
                {
                    case 0:    
                        buf.Copy(KCrashFileName);
                        break;
                    case 1:
                        buf.Copy(KLibName1);
                        break;
                    case 2:
                        buf.Copy(KLibName2);
                        break;

                }
                if (buf.Length() > 0)
                {
                    iStrInfoTbl->AddStringL(buf);
                }
                index = iStrInfoTbl->GetIndex(buf);
                seg.ed_name = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
                reference.Append(ptr);
            }
            

            writer->OpenL(_L("") ); //clear writer buffer
			formatter->iOffset = 0;
            TInt offset = 0;

			RCodeSegPointerList segList;

            formatter->iDataSource->GetCodeSegmentsL(KCrashTid, segList);

			formatter->WriteExecutableInfoL( segList, KCrashTime, KCrashTid );

            segList.ResetAndDestroy();

			RDebug::Print(_L("itest1\n"));
            itest(offset + reference.Length() == formatter->iOffset);

			RDebug::Print(_L("itest2\n"));
            itest(offset != formatter->iOffset);

			//RDebug::Print(_L("itest3\n"));
            //itest(reference == writer->iData);

			RDebug::Print(_L("itest4\n"));
            itest(reference != _L8(""));

            CleanupStack::PopAndDestroy(&reference); //reference
			
            break;
        }

        case EWriteRegistersInfo:
        {

			RDebug::Print(_L("EWriteRegistersInfo starting...\n"));
            TRAP(err, formatter->ConfigureDataSourceL(server));
            itest(err == KErrNone);
            itest(server == formatter->iDataSource);

            TRAP(err, formatter->ConfigureDataSaveL(writer));
            itest(err == KErrNone);
            itest(writer == formatter->iDataSave);

            reference.CreateL( sizeof(Sym32_dhdr) + sizeof(Sym32_reginfod) * (1 + KCoProRegsCount/4) 
                                                  + sizeof(Sym32_regdatad) * (KCoreRegsCount+KCoProRegsCount/2) );
            CleanupClosePushL(reference);

            TPtrC8 ptr;
            Sym32_dhdr dhdr;
            //Sym32_reginfod seg;
            //Sym32_regdatad reg;
            for(TInt i =0; i < KCrashThreadsCount; ++i)
            {
                TBuf8<255> buf;
                buf.Format(_L8("CORE.SYMBIAN.REGISTER.%u"), KCrashTid + i);
                iStrInfoTbl->AddStringL(buf);
                TInt32 index = iStrInfoTbl->GetIndex(buf);
                dhdr.d_name	= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
                dhdr.d_descrsz	= (sizeof(Sym32_reginfod) + sizeof(Sym32_regdatad)) * KCoreRegsCount; 
                dhdr.d_type	= ESYM_NOTE_REG;
                dhdr.d_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
                dhdr.d_elemnum = KCoreRegsCount + KCoProRegsCount/4; //core regs plus one for each copro reg size
                ptr.Set(reinterpret_cast<TUint8*>(&dhdr), sizeof(Sym32_dhdr));
                reference.Append(ptr);

                /*
                 defect in formatter code needs to be fixed, so not sure how is it going to be implemented
                 but it should match the code below
                //all core regs are 32-bit and available
                seg.rid_class = ESYM_CORE;
                seg.rid_num_registers = KCoreRegsCount;
                seg.rid_repre = ESYM_REG_32;
                seg.rid_thread_id = KCrashTid + i;
                seg.rid_version = iStrInfoTbl->Getindex(KSymbianElfCoreDumpVersion);
                ptr.Set(reinterpret_cast<TUint8*>(&seg), sizeof(Sym32_reginfod));
                reference.Append(ptr);

                for(TInt j = 0; j < KCoreRegsCount; ++j)
                {
                    reg.rd_id = regid; //r0...r15 
                    reg.rd_sub_id = 0; //not important for core
                    reg.rd_data = 0; //good question??
                    ptr.Set(reinterpret_cast<TUint8*>(&reg), sizeof(Sym32_regdatad));
                    reference.Append(ptr);
                }

                //done with core regs now four types of copro regs
                seg.rid_class = ESYM_COPRO;
                seg.rid_num_registers = KCoProRegsCount/4;
                seg.rid_num_registers /=2; //half of them is not available
                seg.rid_repre = ESYM_REG_8;
                seg.rid_thread_id = KCrashTid + i;
                seg.rid_version = iStrInfoTbl->Getindex(KSymbianElfCoreDumpVersion);
                ptr.Set(reinterpret_cast<TUint8*>(&seg), sizeof(Sym32_reginfod));
                reference.Append(ptr);
                
                for(TInt j = 0; j < KCoProRegsCount/2; ++j) //half of them is available
                {
                    if(i%4 == 0)
                    {
                        reg.rd_id = j;
                        reg.rd_sub_id = KRegSubId + i;
                        reg.rd_data = 0; //what about that?
                        ptr.Set(reinterpret_cast<TUint8*>(&reg), sizeof(Sym32_regdatad));
                        reference.Append(ptr);
                    }
                }
                */
            }

            writer->OpenL(_L("") ); //clear writer buffer

            CleanupStack::PopAndDestroy(&reference); //reference
            break;    
        }

        default:
            break;
    }

	RDebug::Print(_L("-> CleanupStack::PopAndDestroy( iStrInfoTbl );\n"));
	CleanupStack::PopAndDestroy( iStrInfoTbl );
	iStrInfoTbl = NULL;
	RDebug::Print(_L("<- TestSuiteL()\n"));
}

void RunTestsL()
{
    RDebug::Print(_L("-> RunTestsL()0\n"));

	RArray<TUint> pt;
	CleanupStack::PushL(&pt);
	CleanupStack::Check(&pt);
		
    CSymbianElfFormatterTest::TSuite suite[] = {
                                         CSymbianElfFormatterTest::ENone,
                                         CSymbianElfFormatterTest::EConfigData,
                                         //CSymbianElfFormatterTest::EFindImplementation,
                                         CSymbianElfFormatterTest::EConfigParams,
                                         CSymbianElfFormatterTest::EGetDescription,
                                         CSymbianElfFormatterTest::EWriteSymbianInfo,
                                         CSymbianElfFormatterTest::EWriteCodeSegments,
                                         CSymbianElfFormatterTest::EWriteDataSegments,
                                         CSymbianElfFormatterTest::EWriteThreadsInfo,
                                         CSymbianElfFormatterTest::EWriteProcessInfo,
                                         CSymbianElfFormatterTest::EWriteExecutableInfo,
                                         //CSymbianElfFormatterTest::EWriteRegistersInfo,
                                         CSymbianElfFormatterTest::ECrash
                                         };
	RDebug::Print(_L("-> RunTestsL()2\n"));

    RTest rtest(KTestTitle);
    rtest.Title(); 
    rtest.Start(_L(""));
    CSymbianElfFormatterTest::TSuite *test = suite; 

	CleanupStack::Check(&pt);
	RDebug::Print(_L("-> RunTestsL()3\n"));

    while(*test != CSymbianElfFormatterTest::ECrash)
    {
		RDebug::Print(_L("-> CleanupStack::Check(&pt);\n"));
		CleanupStack::Check(&pt);

        CSymbianElfFormatterTest *itester = CSymbianElfFormatterTest::NewLC(rtest);
        itester->TestSuiteL(*test++);

		RDebug::Print(_L("-> CleanupStack::PopAndDestroy( itester );\n"));
        CleanupStack::PopAndDestroy( itester );
		RDebug::Print(_L("<- CleanupStack::PopAndDestroy( itester );\n"));
	}


    rtest.End();
    rtest.Close();
    RDebug::Print(_L("<- RunTestsL()\n"));
	CleanupStack::Check(&pt);

	pt.Close();
	CleanupStack::Pop(&pt);
}

	
TInt E32Main()
{
    RDebug::Print(_L("-> E32Main()\n"));
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();

    __ASSERT_ALWAYS(cleanup, User::Panic(_L("SYMBIAN_ELF_TEST-NO_CLEANUP"), KErrNoMemory));

	TRAPD(err, RunTestsL());

	RDebug::Print(_L("E32Main() after TRAPD(err=%d, RunTestsL());\n"), err);
	__ASSERT_ALWAYS(!err, User::Panic(_L("SYMBIAN_ELF_TEST-RunTestsL trapped with error: "), err));

	RDebug::Print(_L("delete cleanup;\n"));
	delete cleanup;
	__UHEAP_MARKEND;
    RDebug::Print(_L("<- E32Main()\n"));
	return err;
}

