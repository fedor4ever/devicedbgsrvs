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

#include "dexcformatter.h"
//
// there is a line limit but what if file name is longer?
//

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CDexcFormatter* CDexcFormatter::NewL()
{
    LOG_MSG("->CDexcFormatter::NewL()\n");
	CDexcFormatter* self = CDexcFormatter::NewLC();
	CleanupStack::Pop(self);
    return self;
}

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CDexcFormatter* CDexcFormatter::NewLC()
{
    LOG_MSG("->CDexcFormatter::NewLC()\n");
	CDexcFormatter* self = new(ELeave) CDexcFormatter();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

/**
2nd stage construction
Reserves config parameter list, creates config parameter objects and sets them to default values.
*/
void CDexcFormatter::ConstructL()
{
    LOG_MSG("->CDexcFormatter::ConstructL()\n");
    TInt err;
	COptionConfig * config;

	iConfigList.ReserveL( (TInt)EDexcParamsLast );

	_LIT( KTrueFalseOpt, "True,False" );
	_LIT( KTrueOpt, "True" );
	_LIT( KFalseOpt, "False" );

	_LIT( KDumpStackPrompt, "Dump stack memory?" );
	config = COptionConfig::NewL(  (TInt)EDumpStack,
									KDEXCFormatterUid,
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETBool, 
									KDumpStackPrompt, 
                                    1,
									KTrueFalseOpt,
									1,
									KTrueOpt );

	err = iConfigList.Append( config );

    if(err != KErrNone)
    {
        delete config;
	    LOG_MSG("CDexcFormatter::ConstructL - unable to append dump stack config option!\n" );
        User::Leave(err);
    }
	iDumpStack = ETrue;

	_LIT( KDumpAsciiPrompt, "Stack contents in ASCII?" );
	config = COptionConfig::NewL(  (TInt)EAsciiStack,
									KDEXCFormatterUid,
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETBool, 
									KDumpAsciiPrompt, 
                                    1,
									KTrueFalseOpt,
									0,
									KFalseOpt );

	err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
	    LOG_MSG("CDexcFormatter::ConstructL - unable to append dump ascii config option!\n" );
        User::Leave(err);
    }
	iAsciiStack = EFalse;
}

/**
dtor frees config parameter list
*/
CDexcFormatter::~CDexcFormatter()
{
    LOG_MSG("->~CDexcFormatter()\n");
    iConfigList.ResetAndDestroy();
}

/**
   Sets pointer to the writer plugin.
  Configures formatter's dump data save plugin.

@param aDataSave pointer to writer plugin object.
@leave KErrNoMemory if aDataSave is NULL 
*/
void CDexcFormatter::ConfigureDataSaveL(CCrashDataSave* aDataSave)
{
    LOG_MSG2("->ConfigureDataSaveL(%d)\n", aDataSave);
    User::LeaveIfNull(aDataSave);
    iDataSave = aDataSave;
}

/**
   Sets pointer to the crash data server.
  Configures formatter's crash data source plugin.

@param aDataSource pointer to core dump server object.
@leave KErrNoMemory if aDataSource is NULL 
*/
void CDexcFormatter::ConfigureDataSourceL(CCrashDataSource* aDataSource)
{
    LOG_MSG2("->ConfigureDataSourceL(%d)\n", aDataSource);
    User::LeaveIfNull(aDataSource);
    iDataSource = aDataSource;
}

/**
   Returns a short formatter description.
@param aPluginDescription Descriptor capable of storing a short plugin description. 
*/
void CDexcFormatter::GetDescription(TDes& aPluginDescription)
{
    LOG_MSG("->CDexcFormatter::GetDescription()\n");
	aPluginDescription.Copy(KPluginDescription());
}


/**
Called by CDS to ask for the configuration parameters that the writer needs (see Plugin Configuration) 
@return actual number of implemented config parameters
*/
TInt CDexcFormatter::GetNumberConfigParametersL( )
{
	TInt count = iConfigList.Count();
	LOG_MSG2("->CDexcFormatter::GetNumberConfigParametersL() : returns:%d\n", count );
	return count;
}


/**
Called by CDS to ask for configuration parameter prompt (see Plugin Configuration) 

@param aIndex indicates which parameter to return
@return pointer to COptionConfig object representing the requested config parameter. Caller doesn't take ownership of the object!
@leave KErrBadHandle if index is out of bounds
@see COptionConfig
*/
COptionConfig * CDexcFormatter::GetConfigParameterL( const TInt aIndex )
{
	// return the config identified by aIndex
	if( ( aIndex < 0 ) || ( aIndex >= iConfigList.Count() ) )
		{
		User::Leave( KErrBadHandle );
		}

	return iConfigList[aIndex];
}

/**
Change a configuration parameter.
@param aIndex Index of parameter to change
@param aValue Unused 
@param aDescValue Path and filename to use
@leave KErrBadHandle if index is out of bounds or one of the other system wide error codes
*/
void CDexcFormatter::SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue )
{
	if( ( aIndex < 0 ) || ( aIndex >= iConfigList.Count() ) )
	{
		User::Leave( KErrBadHandle );
	}

	COptionConfig & config = *(iConfigList[aIndex]);

	if( ( aIndex != (TInt)EDumpStack ) && ( aIndex != (TInt)EAsciiStack ) )
	{
		LOG_MSG( "  Leaving : aIndex != EDumpStack && aIndex != EAsciiStack" );
		User::Leave( KErrArgument );
	}

	TBool setTo = ETrue;

	if( aDescValue.Length() )
	{
		if( 0 == aDescValue.CompareF(_L("True") ) )
		{
			setTo = ETrue;
			//LOG_MSG( " 0 == aDescValue.CompareF(_L(True" );
		}
		else if( 0 == aDescValue.CompareF(_L("False") ) )
		{
			//LOG_MSG( " 0 == aDescValue.CompareF(_L(False" );
			setTo = EFalse;
		}
		else
		{
			LOG_MSG( " ERROR !* : CDexcFormatter::SetConfigParameterL : Invalid string for boolean" );
			User::Leave( KErrArgument );
		}
	}
	else
	{
		if( 1 == aValue )
		{
			//LOG_MSG( " 1 == aValue" );
			setTo = ETrue;
		}
		else if ( 0 == aValue )
		{
			//LOG_MSG( " 0 == aValue " );
			setTo = EFalse;
		}
		else
		{
			LOG_MSG( "  Value of %d not valid. Must be 0 or 1\n" );
			User::Leave( KErrArgument );
		}
	}


	if( setTo )
	{
		config.Value( 1 );
		config.ValueL( _L("True") );
	}
	else
	{
		config.Value( 0 );
		config.ValueL( _L("False") );
	}

	if( aIndex == (TInt)EDumpStack )
	{
		iDumpStack = config.ValueAsBool();
	}
	else if( aIndex == (TInt)EAsciiStack )
	{
		iAsciiStack = config.ValueAsBool();
	}
}

/**
   Dumps basic information about the thread that crashed.
@param aThread TThreadInfo data structure holding information about the crashed thread.
@leave err one of the system wide error codes
*/
void CDexcFormatter::DumpLogL(const CThreadInfo& aThread)
{
    LOG_MSG("->DumpLogL()\n");
	iLine.Fill('-', 76);
	iDataSave->WriteL(iLine);

	_LIT8(KHdr, "\r\nEKA2 USER CRASH LOG\r\n");
	iLine = KHdr;
	iDataSave->WriteL(iLine);

    RBuf8 filename;
    filename.CreateL(KMaxFileName);
    filename.Copy(aThread.Name());
	_LIT8(KName, "Thread Name: %S\r\n");
	iLine.Format(KName, &filename);
    filename.Close();
	iDataSave->WriteL(iLine);

	_LIT8(KFmtTid, "Thread ID: %u\r\n");
	iLine.Format(KFmtTid, aThread.Id());
	iDataSave->WriteL(iLine);
	
	_LIT8(KFmtStack, "User Stack %08X-%08X\r\n");
	iLine.Format(KFmtStack, aThread.UsrStackAddr(), aThread.UsrStackAddr() + aThread.UsrStackSize());
	iDataSave->WriteL(iLine);
}
/**
   Dumps panic details.
@param aCrash TCrashInfo data structure holding information about the crash.
@leave err one of the system wide error codes
*/
void CDexcFormatter::DumpPanicInfoL(const TCrashInfo& aCrash)
{
    LOG_MSG("->DumpPanicInfoL()\n");
	//_LIT8(KFmtPanic, "\r\nPanic: %S-%d\r\n");
	_LIT8(KFmtPanic, "Panic: %S-%d\r\n");
    RBuf8 buf;
    buf.CreateL(aCrash.iCategory.Length());
    buf.Copy(aCrash.iCategory);
    iLine.Format(KFmtPanic, &buf, aCrash.iReason);
    buf.Close();
	iDataSave->WriteL(iLine);
}

/**
   Picks registers from available register list and puts them in the required register list.
   Order of the output list is determined by the order of aReqIds.

@param aAllRegs input RRegisterList of all available registers.
@param aReqRegs output RRegisterList for the required registers.
@param aReqIds EDF_Register_Last terminated array of IDs of required registers.
@return KErrNone if successful, otherwise one of the other system-wide error codes.
*/
TInt CDexcFormatter::PickRegisters(const RRegisterList& aAllRegs, RRegisterList& aReqRegs,
                                   const TFunctionalityRegister* const aReqIds) const
{
    LOG_MSG("->CDexcFormatter::PickRegistersL()\n");
    if(aReqIds == NULL)
    {
        return KErrArgument;
    }
        
    TInt regsCount = aAllRegs.Count();
    const TFunctionalityRegister* reg;
    for(reg = aReqIds; *reg != ERegisterLast; ++reg)
    {
        for(TInt i = 0; i < regsCount; ++i)
        {
            if(aAllRegs[i].GetId() == *reg)
            {
                aReqRegs.Append(aAllRegs[i]);
                break;
            }
        }
    }

    if(aReqRegs.Count() != (reg - aReqIds)) 
    {
	    LOG_MSG3("CDExcFormatter::PickRegisters - found only %d out of %d registers!\n", aReqRegs.Count(), (reg - aReqIds));
        return KErrNotFound;
    }
    return KErrNone;
}

/**
   Dumps exception details. Valid on ARM platform only!

@param aCrash TCrashInfo data structure holding information about the crash event.
@leave err one of the system wide error codes
*/
void CDexcFormatter::DumpExcInfoL(const TCrashInfo& aCrash)
{
    LOG_MSG("-> CDexcFormatter::DumpExcInfoL()\n");

	_LIT8(KHdr, "\r\nUNHANDLED EXCEPTION:\r\n");
	iLine = KHdr;
	iDataSave->WriteL(iLine);

	_LIT8(KFmt1, "code=%d PC=%08x FAR=%08x FSR=%08x\r\n");
	iLine.Format(KFmt1, aCrash.iContext.iExcCode,
                        aCrash.iContext.iR15,
                        aCrash.iContext.iFaultAddress,
                        aCrash.iContext.iFaultStatus);   
	iDataSave->WriteL(iLine);

	_LIT8(KFmt2, "R13svc=%08x R14svc=%08x SPSRsvc=%08x\r\n");
	iLine.Format(KFmt2, aCrash.iContext.iR13Svc,
                        aCrash.iContext.iR14Svc,
                        aCrash.iContext.iSpsrSvc);
	iDataSave->WriteL(iLine);

    LOG_MSG("<- CDexcFormatter::DumpExcInfoL()\n");
} 

/**
   Dumps register values of the crashed thread. Valid on ARM platform only!
@param aTid ID of the crashed thread
@leave err one of the system wide error codes
*/
void CDexcFormatter::DumpRegistersL(const TUint64 aTid)
{
    LOG_MSG2("-> CDexcFormatter::DumpRegistersL(%Lu)\n", aTid);

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
                                    ERegisterLast};

    RRegisterList allRegs;
    CleanupClosePushL(allRegs);
    RRegisterList reqRegs;
    CleanupClosePushL(reqRegs);

	_LIT8(KHdr, "\r\nUSER REGISTERS:\r\n");
	iLine = KHdr;
	iDataSave->WriteL(iLine);

    LOG_MSG("CDexcFormatter::DumpRegistersL - getting list of avaliable registers\n");
    iDataSource->GetRegisterListL(allRegs);

    LOG_MSG("CDexcFormatter::DumpRegistersL - picking required registers\n");
    User::LeaveIfError(PickRegisters(allRegs, reqRegs, ids)); 

    LOG_MSG("CDexcFormatter::DumpRegistersL - reading data of required registers\n");
    iDataSource->ReadRegistersL(aTid, reqRegs); 

    //display zeros when register is not available
    //use AppendFormat to get rid of this macro
#define REG_VAL(i) reqRegs[i].iAvailable ? reqRegs[i].GetContent32() : 0
	_LIT8(KFmtCpsr, "CPSR=%08x\r\n");
    TInt regsCount = reqRegs.Count();
	iLine.Format(KFmtCpsr, REG_VAL(regsCount - 1)); 
	iDataSave->WriteL(iLine);

    regsCount -= 4;
	for(TInt i = 0; i < regsCount; i+=4)
	{
		_LIT8(KFmtReg, "r%02d=%08x %08x %08x %08x\r\n");
        iLine.Format(KFmtReg, i, REG_VAL(i), REG_VAL(i+1), REG_VAL(i+2), REG_VAL(i+3));
		iDataSave->WriteL(iLine);
	}
#undef REG_VAL
    CleanupStack::PopAndDestroy(2, &allRegs);
}

/**
Cleanup item implementation for code segment list in DumpCodeSegsL() method.
@param aArray pointer to the list that is supposed to be freed
*/
void CDexcFormatter::CleanupCodeSegList(TAny *aArray)
{
    LOG_MSG("->CDexcFormatter::CleanupCodeSegList()\n");
	RCodeSegPointerList *codeSegList = static_cast<RCodeSegPointerList*> (aArray);
    codeSegList->ResetAndDestroy();
    codeSegList->Close();
}

/**
   Dumps code segments of the process that owned the crashed thread.
@param aTid ID of the crashed thread
@leave err one of the system wide error codes
*/
void CDexcFormatter::DumpCodeSegsL(const TUint64 aTid)
{
    LOG_MSG2("-> CDexcFormatter::DumpCodeSegsL(%Lu)\n", aTid);
	_LIT8(KHdr, "\r\nCODE SEGMENTS:\r\n");
	_LIT8(KFmtSegs, "%08X-%08X %S\r\n");

	iLine = KHdr;
	iDataSave->WriteL(iLine);

    RCodeSegPointerList segs;
    TCleanupItem cleanup(CDexcFormatter::CleanupCodeSegList, (TAny*)&segs);
    CleanupStack::PushL(cleanup);

    //User::LeaveIfError(iDataSource->GetCodeSegments(aPid, segs));
    LOG_MSG("CDexcFormatter::DumpCodeSegsL() - getting code segments data\n");
    iDataSource->GetCodeSegmentsL(aTid, segs);

    RBuf8 filename;
    CleanupClosePushL(filename);
    filename.CreateL(KMaxFileName);
	for (TInt i=0; i < segs.Count(); i++)
	{
        filename.Copy(segs[i]->iName); //conversion
        iLine.Format(KFmtSegs, segs[i]->iCodeRunAddr, segs[i]->iCodeRunAddr + segs[i]->iCodeSize, &filename);
        iDataSave->WriteL(iLine);
	}
    LOG_MSG2("CDexcFormatter::DumpCodeSegsL - code segments count:%d\n", segs.Count());
    CleanupStack::PopAndDestroy(2);

    //compatibility with legacy D_EXC implementation
    iLine.Zero();
    iLine.Append(_L8("\r\n"));
    iDataSave->WriteL(iLine);
}

/**
   Dumps thread stack information and content. 
@param aTid ID of the crashed thread
@param aInfo CThreadInfo data structure holding information about the crashed thread
@leave err one of the system wide error codes
*/
void CDexcFormatter::DumpStackL(const TUint64 aTid, const CThreadInfo& aInfo)
{
    LOG_MSG2("-> CDexcFormatter::DumpStackL(%Lu)\n", aTid);

    TInt length = KBufSize < aInfo.UsrStackSize() ? KBufSize : aInfo.UsrStackSize(); 

	RBuf8 buf;
    buf.CreateL(length);
    CleanupClosePushL(buf);

	TUint top = aInfo.UsrStackAddr() + aInfo.UsrStackSize();
	for(TUint base = aInfo.UsrStackAddr(); base < top; base += KBufSize)
	{
        LOG_MSG3("CDexcFormatter::DumpStackL - reading %d bytes of memory from:0x%X\n", length, base);
		iDataSource->ReadMemoryL(aTid, base, length, buf); 

        //6571de54: 07 00 00 10 14 04 17 f8 00 00 00 00 d4 4e 40 00 .............N@.
        //<-addr-------------------------memory----------------------representation>
		if(iAsciiStack)
		{
			TBuf8<80> out; //74 used
			TBuf8<20> ascii; //16 usd
			TInt len = buf.Length();
            for(TUint offset = 0; offset < len; offset += 16) 
			{
				out.Zero();
				ascii.Zero();
				out.AppendNumFixedWidth(base + offset, EHex, 8); //addr
				out.Append(_L(": "));

				for(TUint byte = 0; byte < 16; ++byte)
				{
					TUint8 c = *(buf.Ptr() + offset + byte);
					out.AppendNumFixedWidth(c, EHex, 2); //each memory byte
					out.Append(' ');

					if(c < 0x20 || c >= 0x7f) //something writable
						c = 0x2e; //.
					ascii.Append(TChar(c)); //byte representation
				}
				out.Append(ascii);
                out.Append(_L("\r\n"));
				iDataSave->WriteL(out);
			} 
		}
		else //binary dump
			iDataSave->WriteL(buf);
	}
    CleanupStack::PopAndDestroy(&buf); 
}

/**
    Retrives the 'cancel crash' property value and checks if required to abort the crash dump. Updates 'crash progress' property. 

@param aProgress descriptor with the crash progress value.
@leave KErrAbort if crash cancel property has been set
 */
void CDexcFormatter::UpdateCrashProgressL(const TDesC &aProgress) const
    {
    LOG_MSG("->CDexcFormatter::UpdateCrashProgress()\n");
    TInt cancelCrash = EFalse;
    TInt err = RProperty::Get(KCoreDumpServUid, ECancelCrash, cancelCrash);
    if(err != KErrNone)
        {
        LOG_MSG2("CDexcFormatter::UpdateCrashProgress - unable to retrive 'cancel crash' value! err:%d\n", err);
        }

    if(cancelCrash)
        {
        LOG_MSG("CDexcFormatter::UpdateCrashProgress - aborting dump in progress\n");
        User::Leave(KErrAbort);
        }

    err = RProperty::Set(KCoreDumpServUid, ECrashProgress, aProgress);
    if(err != KErrNone)
        {
        LOG_MSG2("CDexcFormatter::UpdateCrashProgress - unable to retrive 'crash progress' value! err:%d\n", err);
        }
#ifdef DEBUG 
    User::After(1000000);
#endif
  }

/**
    Gets information about the threads of the crashed process and finds the one that caused the crash. 
@return reference to the CThreadInfo data structure holding information about the crashed thread
@leave err one of the system wide error codes
*/
const CThreadInfo& CDexcFormatter::GetCrashedThreadInfoLC(const TCrashInfo &aCrashInfo) const
    {
    LOG_MSG("->CDexcFormatter::GetCrashedThreadInfoLC()\n");

	RThreadPointerList *threadList = new(ELeave)RThreadPointerList;
    TCleanupItem cleanup(CDexcFormatter::CleanupThreadList, (TAny*)threadList);
    CleanupStack::PushL(cleanup);

	CThreadInfo *threadInfo = NULL;
	TUint totalThreadListDescSize;
	iDataSource->GetThreadListL((TUint64)aCrashInfo.iPid, *threadList, totalThreadListDescSize); //for crashed process

	for(TInt i = 0; i < threadList->Count(); i++)
		{
		if(aCrashInfo.iTid == (*threadList)[i]->Id())
			{
			threadInfo = (*threadList)[i]; //crashed thread
			break;
			}
		}

	if(NULL == threadInfo)
		{
        LOG_MSG("CDexcFormatter::CrashEventL - crashed thread not found!\n");
        User::Leave( KErrNotFound ); 
		}

    return *threadInfo;
    }

/**
    Prepares the name of the dump file. Checks DataSave filename option, if it is provided by the user
    it appends crashed thread ID and extension, if not it uses default name appended with thread ID and extension. 
@param aTid ID of the crashed thread
@param aExtension descriptor holding the file extension
@return descriptor holding the final dump file name, caller is responsible to pop it from the cleanup stack
@leave err one of the system wide error codes
 */
const TDesC& CDexcFormatter::DumpFileNameLC(TUint64 aTid, const TDesC &aExtension)
{
    COptionConfig *option = iDataSave->GetConfigParameterL(CCrashDataSave::ECoreFilePath);
	const TDesC &userFileName = option->ValueAsDesc();
	HBufC *filename = HBufC::NewLC(userFileName.Length() + KDexcFileNameLength);
    if(userFileName.Length() > 0)
        {
	    *filename = userFileName;
        _LIT(KTidFormat, "%Lu");
        filename->Des().AppendFormat(KTidFormat, aTid);
        }
    else
        {
        _LIT(KTxtFileFormat, "c:\\d_exc_%Lu");
        filename->Des().Format(KTxtFileFormat, aTid);
        }
    filename->Des().Append(aExtension);
    return *filename;
}


/**
   Main formatter method. Handles crash event notification and drives dump process.

@param aCrashInfo pointer to the TCrashInfo data structure holding information about the crash. 
@leave err one of the system wide error codes
*/
void CDexcFormatter::CrashEventL(TCrashInfo *aCrashInfo)
{
    LOG_MSG("-> CrashEventL()\n");

    User::LeaveIfNull(aCrashInfo);
    LOG_MSG("CDexcFormatter::CrashEvenL - crash info is sane\n");
    User::LeaveIfNull(iDataSource);
    LOG_MSG("CDexcFormatter::CrashEvenL - data source is sane\n");
    User::LeaveIfNull(iDataSave);
    LOG_MSG("CDexcFormatter::CrashEvenL - data save is sane\n");

    _LIT(KProgressStart, "crash dump started");
    UpdateCrashProgressL(KProgressStart);

    const CThreadInfo &threadInfo = GetCrashedThreadInfoLC(*aCrashInfo);
    _LIT(KProgressText, "dumping text file");
    UpdateCrashProgressL(KProgressText);

    LOG_MSG("CDexcFormatter::CrashEventL - opening text file\n");
    _LIT(KTxtFileExt, ".txt");
    iDataSave->OpenL(DumpFileNameLC(aCrashInfo->iTid, KTxtFileExt));
    CleanupStack::PopAndDestroy(); //filename
    DumpLogL(threadInfo);

    if(aCrashInfo->iType == TCrashInfo::ECrashKill)
    {
        LOG_MSG("CDexcFormatter::CrashEventL - dumping panic info\n");
        DumpPanicInfoL(*aCrashInfo);
    }
#ifdef __MARM__
    else //ECrashException
    {
        LOG_MSG("CDexcFormatter::CrashEventL - dumping exeception info\n");
        DumpExcInfoL(*aCrashInfo);
    }
    LOG_MSG("CDexcFormatter::CrashEventL - dumping registers info\n");
    DumpRegistersL(aCrashInfo->iTid);
#endif
    LOG_MSG("CDexcFormatter::CrashEventL - dumping code segments info\n");
    DumpCodeSegsL(aCrashInfo->iTid);

    LOG_MSG("CDexcFormatter::CrashEventL - closing text file\n");
    iDataSave->CloseL();

    if(iDumpStack)
    {
        _LIT(KProgressStack, "dumping stack file");
        UpdateCrashProgressL(KProgressStack);
        LOG_MSG("CDexcFormatter::CrashEventL - opening stack file\n");
        _LIT(KStkFileExt, ".stk");
        iDataSave->OpenL(DumpFileNameLC(aCrashInfo->iTid, KStkFileExt));
        CleanupStack::PopAndDestroy();//filename
        LOG_MSG("CDexcFormatter::CrashEventL - dumping stack\n");
        DumpStackL(aCrashInfo->iTid, threadInfo);
        LOG_MSG("CDexcFormatter::CrashEventL - closing stack file\n");
        iDataSave->CloseL();
    }
    _LIT(KProgressCompleted, "crash dump completed");
    UpdateCrashProgressL(KProgressCompleted);
    CleanupStack::PopAndDestroy(); //GetCrashedThreadInfoLC()::threadList , results in a call to CleanupThreadList 
    LOG_MSG("CDexcFormatter::CrashEventL - crash finished\n");
}

/**
Cleanup item implementation for thread list in CrashEventL() method.
@param aArray pointer to the list that is supposed to be freed
*/
void CDexcFormatter::CleanupThreadList(TAny *aArray)
{
    LOG_MSG("->CDexcFormatter::CleanupThreadList()\n");
	RThreadPointerList *threadList = static_cast<RThreadPointerList*> (aArray);
    threadList->ResetAndDestroy();
    threadList->Close();
    delete threadList;
}

