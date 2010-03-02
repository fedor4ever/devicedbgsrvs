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
// Implementation of CSymbianElfFormatter class
//

/**
 @file
 @internalTechnology
 @released
 @see CCoreDumpFormatter
*/

#include <e32base.h>
#include <e32property.h>
#include <e32debug.h>

#include <coredumpserverapi.h>
#include "symbianelfformatter.h"
#include "symbianelfstringinfo.h"

#define KMaxCrc32Size 1000

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CSymbianElfFormatter* CSymbianElfFormatter::NewL()
{
	LOG_MSG("->CSymbianElfFormatter::NewL()->\n");
	CSymbianElfFormatter* self = CSymbianElfFormatter::NewLC();
	CleanupStack::Pop();
	return self;
}

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CSymbianElfFormatter* CSymbianElfFormatter::NewLC()
{
	LOG_MSG("->CSymbianElfFormatter::NewLC()->\n");
	CSymbianElfFormatter* self = new(ELeave)CSymbianElfFormatter();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

/**
ctor nothing really
*/
CSymbianElfFormatter::CSymbianElfFormatter()
{
	LOG_MSG("->CSymbianElfFormatter::CSymbianElfFormatter()\n");
}

/**
2nd stage construction
Creates elf options object
*/
void CSymbianElfFormatter::ConstructL()
{
	iElfOptions = CElfOptions::NewL( (TUint32)KSymbianELFFormatterUid );
	iElfFileName.CreateL(KMaxFileName);
}

/**
dtor frees elf options object
*/
CSymbianElfFormatter::~CSymbianElfFormatter()
{
	LOG_MSG( "CSymbianElfFormatter::~CSymbianElfFormatter()\n" );

	iElfFileName.Close();

    if(iElfOptions)
    {
	    delete iElfOptions;
        iElfOptions = NULL;
    }
}

/**
Sets pointer to the writer plugin.  Configures formatter's dump data save plugin.
@param aDataSave Pointer to writer plugin
@leave KErrNoMemory if aDataSave is NULL
*/
void CSymbianElfFormatter::ConfigureDataSaveL( CCrashDataSave * aDataSave )
{
	LOG_MSG("->CSymbianElfFormatter::ConfigureDataSave()\n");
    User::LeaveIfNull(aDataSave);
	iDataSave  = aDataSave;
}

/**
Sets pointer to the crash data server. Configures formatter's crash data source plugin
@param aDataSource Pointer to Crash Data Source
@leave KErrNoMemory if aDataSource is NULL
*/
void CSymbianElfFormatter::ConfigureDataSourceL( CCrashDataSource * aDataSource )
{
	LOG_MSG("->CSymbianElfFormatter::ConfigureDataSource()\n");
    User::LeaveIfNull(aDataSource);
	iDataSource = aDataSource;
}

/**
Returns a description of the plugin.
@param aPluginDescription Output parameter that contains the plugin description.
*/
void CSymbianElfFormatter::GetDescription( TDes & aPluginDescription )
{
	LOG_MSG("CSymbianElfFormatter::GetDescription()\n");
	aPluginDescription.Copy(KPluginDescription());
}

/**
Called by CDS to ask for the configuration parameters that the writer needs (see Plugin Configuration)
@return actual number of implemented config parameters
*/
TInt CSymbianElfFormatter::GetNumberConfigParametersL( )
	{
	TInt numberConfigParameters = iElfOptions->GetNumberConfigParametersL( );
	LOG_MSG2("->CSymbianElfFormatter::GetNumberConfigParametersL() will return %d \n",
		numberConfigParameters );
	return ( numberConfigParameters );
	}

/**
Called by CDS to ask for configuration parameter prompt (see Plugin Configuration)

@param aIndex indicates which parameter to return
@return pointer to COptionConfig object representing the requested config parameter. Caller doesn't take ownership of the object!
@leave KErrBadHandle if index is out of bounds
@see COptionConfig
*/
COptionConfig * CSymbianElfFormatter::GetConfigParameterL( const TInt aIndex )
	{
	return ( iElfOptions->GetConfigParameterL( aIndex ) );
	}

/**
Change a configuration parameter.
@param aIndex Index of the configuration parameter.
@param aValue Value of the configuration parameter as an int
@param aDescValue Value of the configuration parameter as a string
@leave err one of the system wide error codes
@see COptionConfig
@see CElfOptions
*/
void CSymbianElfFormatter::SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue )
	{
	LOG_MSG3("->CSymbianElfFormatter::SetConfigParameterL(index=%d, val=%d)\n", aIndex, aValue );
	iElfOptions->SetConfigParameterL( aIndex, aValue, aDescValue );
	}

/**
Create and save Sym32_syminfod segment.
@param aCrashInfo TCrashInfo data structure holding information about the crash.
@param aCrc32 CRC calculated for the crashed executable
@return value of the media-position indicator at which the call started storing its data
@leave err one of the system wide error codes
@see Sym32_syminfod
*/
TUint CSymbianElfFormatter::WriteSymbianInfoL(const TCrashInfo &aCrashInfo, const TUint32 aCrc32 )
{
	LOG_MSG2( "CSymbianElfFormatter::WriteSymbianInfoL( pid=0x%X)\n", I64LOW(aCrashInfo.iPid) );

	TUint offset = iOffset;
	LOG_MSG2( " iOffset at start of WriteSymbianInfoL->%d", iOffset );

	Sym32_dhdr *symInfo_descriptor = new(ELeave) Sym32_dhdr();
	CleanupStack::PushL(symInfo_descriptor);

	TInt32 index = iStrInfoTbl->GetIndex(KSymbian);
	symInfo_descriptor->d_name		= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
	symInfo_descriptor->d_descrsz	= sizeof(Sym32_syminfod);
	LOG_MSG2( " symInfo_descriptor->d_descrsz = 0x%X", symInfo_descriptor->d_descrsz);
	symInfo_descriptor->d_type		= ESYM_NOTE_SYM;
	symInfo_descriptor->d_version	= iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
    symInfo_descriptor->d_elemnum	= 1;

	LOG_MSG3( "  iDataSave->WriteL( iOffset=%d, symInfo_descriptor, sizeof(Sym32_dhdr)=%d)", iOffset, sizeof(Sym32_dhdr) );
	iDataSave->WriteL( iOffset, symInfo_descriptor, sizeof(Sym32_dhdr));

	LOG_MSG( "  CleanupStack::PopAndDestroy(symInfo_descriptor);" );
	CleanupStack::PopAndDestroy(symInfo_descriptor); // symInfo_descriptor

	iOffset += sizeof(Sym32_dhdr);

	// get Symbian Info from iDataSource
	Sym32_syminfod *symInfo = new(ELeave) Sym32_syminfod();
	CleanupStack::PushL(symInfo);

	symInfo->sd_exit_type = aCrashInfo.iType; // ECrashException or EEventsKillThread
	LOG_MSG2( " Sym32_syminfod->sd_exit_type = 0x%X", symInfo->sd_exit_type );

	index = KErrNotFound;

	if( TCrashInfo::ECrashException == aCrashInfo.iType )
		{
		LOG_MSG2( " symInfo->sd_exit_reason = aCrashInfo->iExcNumber = 0x%X", aCrashInfo.iExcNumber );
		symInfo->sd_exit_reason = aCrashInfo.iExcNumber;
		}
	else if( TCrashInfo::ECrashKill == aCrashInfo.iType )
		{
		LOG_MSG2( " symInfo->sd_exit_reason = aCrashInfo.iReason = 0x%X", aCrashInfo.iReason );
		symInfo->sd_exit_reason = aCrashInfo.iReason;

		TBuf8<KMaxExitCategoryName> buf;
		LOG_MSG( "  buf.Copy(aCrashInfo.iCategory)" );
		buf.Copy(aCrashInfo.iCategory);
		if (buf.Length() > 0)
			{
			index = iStrInfoTbl->AddStringL(buf);
			}
		LOG_MSG2( "  Added Crash Category at index %d", index );
		}

	symInfo->sd_thread_id = aCrashInfo.iTid;
	symInfo->sd_proc_id = aCrashInfo.iPid;
	symInfo->sd_date_time = aCrashInfo.iTime;
	LOG_MSG2( " sizeof(Sym32_syminfod->symInfo->sd_date_time) = 0x%X", sizeof(symInfo->sd_date_time)  );
	LOG_MSG3( " time high low = 0x%X%X", I64HIGH(symInfo->sd_date_time), I64LOW(symInfo->sd_date_time) );

	symInfo->sd_execid.exec_id = aCrashInfo.iTime;
	symInfo->sd_execid.exec_crc = aCrc32;
	symInfo->sd_exit_cat = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;

	// Write the Symbian specifics to file
	LOG_MSG3( "  iDataSave->WriteL( iOffset=%d, symInfo, sizeof(Sym32_syminfod)=%d)",
		iOffset, sizeof(Sym32_syminfod) );
	iDataSave->WriteL( iOffset, symInfo, sizeof(Sym32_syminfod));
	CleanupStack::PopAndDestroy(symInfo);

	iOffset += sizeof(Sym32_syminfod);
	return offset;
}

/**
Creates TDataSegment segments by checking the .rodata and .bss/.data sections of the code segments provided by data source.
It may also create data segments for the user stacks of threads in the crashed process, depending on user configuration.
@param aThreadId The id of the crashed thread.
@param aPid ID of the process that owned the crashed thread
@param aThreadList list of the threads in the crashed process or the whole system, depending on user configuration
@param aCodeSegList list of all code segments loaded into process that crashed
@param aDataSegs output list of created data segments
@param aDataSegOffsets output list of media-location offsets where segment data was stored
@return value of the media-position indicator at which the call started storing its data
@leave err one of the system wide error codes
@see TDataSegment
*/
TUint CSymbianElfFormatter::WriteDataSegmentsL( const TUint64 & aThreadId,
											    const TUint64 & aPid,
                                                const RThreadPointerList &aThreadList,
                                                const RCodeSegPointerList &aCodeSegList,
                                                RSegmentList &aDataSegs,
                                                RArray<TUint> &aDataSegOffsets )
	{

	LOG_MSG2( "CSymbianElfFormatter::WriteDataSegmentsL(aThreadId=0x%X)\n", I64LOW(aThreadId) );
	TUint offset = iOffset;

	if(!iElfOptions->CreateDataSegments())
        {
        LOG_MSG("CSymbianElfFormatter::WriteDataSegmentsL - not configured to dump data segs");
        return offset;
        }

    LOG_MSG("CSymbianElfFormatter::WriteDataSegmentsL - generating data segments\n");
    TSegmentInfo segInfo;
    // Create data segments for the threads' stacks in this process
    for( TInt i = 0; i < aThreadList.Count(); i++ )
        {

		if( aThreadList[i]->ProcessId() != aPid )
			{
			// Thread not in crashed process
			continue;
			}

        CThreadInfo &threadInfo = *aThreadList[i];

        LOG_MSG2( "  Generating stack data segment for thread 0x%X\n", I64LOW(threadInfo.Id()) );

        segInfo.iSegBase = threadInfo.UsrStackAddr();
        segInfo.iSegSize = threadInfo.UsrStackSize();
        segInfo.iFlags = PF_W | PF_R;

        LOG_MSG3( "CSymbianElfFormatter::WriteDataSegmentsL - .stack base=0x%X, size=%d)", segInfo.iSegBase, segInfo.iSegSize );
        aDataSegs.AppendL( segInfo );
        }

    LOG_MSG2( "  Found %d .stack data segments for crashed thread", aDataSegs.Count() );

    // Create data segments for the .rodata .data .bss sections of the exe and dlls in this proc
    for(TInt i = 0;  i < aCodeSegList.Count(); i++)
        {
        TCodeSegInfo &codeSeg = *aCodeSegList[i];
        if(codeSeg.iRoDataSize != 0)
            {
            segInfo.iSegBase = codeSeg.iRoDataRunAddr;
            segInfo.iSegSize = codeSeg.iRoDataSize;
            segInfo.iFlags = PF_R;
            LOG_MSG3( "CSymbianElfFormatter::WriteDataSegmentsL - .rodata base=0x%X, size=%d)", segInfo.iSegBase, segInfo.iSegSize );
            aDataSegs.AppendL(segInfo);
            }

        if(codeSeg.iDataSize != 0)
            {
            segInfo.iSegBase = codeSeg.iDataRunAddr;
            segInfo.iSegSize = codeSeg.iDataSize;
            segInfo.iFlags = PF_W | PF_R;
            LOG_MSG3( "CSymbianElfFormatter::WriteDataSegmentsL - .data base=0x%X, size=%d)", segInfo.iSegBase, segInfo.iSegSize );
            aDataSegs.AppendL(segInfo);
            }
        }

    LOG_MSG2( "  Total %d Data Segments for crashed thread", aDataSegs.Count() );

    // Dumping memory content of the identified data segments
    for (TInt i = 0; i < aDataSegs.Count(); i++)
        {
        RBuf8 memData;
        CleanupClosePushL(memData);

        memData.CreateL( aDataSegs[i].iSegSize );

        LOG_MSG4( "iDataSource->ReadMemory(ThreadId=0x%X, at=0x%X, size=0x%X)\n",
                I64LOW(aThreadId), aDataSegs[i].iSegBase, aDataSegs[i].iSegSize );

        TRAPD( memReadErr, iDataSource->ReadMemoryL(aThreadId, aDataSegs[i].iSegBase, aDataSegs[i].iSegSize, memData ) );
        if( KErrNone != memReadErr )
            {
            LOG_MSG2( "  ERROR %d from ReadMemory(). Will generate empty segment\n", memReadErr );
            // This means that there will be a data segment that points to no data
            aDataSegOffsets.AppendL( 0 );
            CleanupStack::PopAndDestroy(&memData);
            continue;
            }

        // save memData to core dump
        LOG_MSG3( "  iDataSave->WriteL( at iOffset=%d, size=0x%X )", iOffset, aDataSegs[i].iSegSize );
        iDataSave->WriteL( iOffset, const_cast<TUint8*>(memData.Ptr()), aDataSegs[i].iSegSize );
        aDataSegOffsets.AppendL( iOffset );
        iOffset += aDataSegs[i].iSegSize;
        CleanupStack::PopAndDestroy(&memData);
        }

	return offset;
	}

/**
Calculate the CRC of the executable code text section up to 1kbytes. We only leave if there is a terminal
error. If we cannot generate the CRC.
@param aTid ID of the crashed thread
@param aExecInfo code segment structure holding iformation required to calculate crc
@return calculated CRC value
@leave err one of the system wide error codes
@see TCodeSegInfo
*/
TUint32 CSymbianElfFormatter::CalcExecCrcL(const TUint64 &aTid, const TCodeSegInfo &aExecInfo )
	{
	TUint32 crc32 = 0xDEADDEAD;

    TInt crcSize = aExecInfo.iCodeSize > KMaxCrc32Size ? KMaxCrc32Size : aExecInfo.iCodeSize;

	RBuf8 crcData;
	crcData.CreateL( crcSize );
    crcData.CleanupClosePushL();

    iDataSource->ReadMemoryL(aTid, aExecInfo.iCodeRunAddr, crcSize, crcData);

	Mem::Crc32(crc32, crcData.Ptr(), crcData.Length());

    CleanupStack::PopAndDestroy( &crcData );

    return crc32;
	}

/**
Goes through all codes segments and saves empty code segments structures as the code .text section is going to be available on the host.
@param aThreadId The id of the crashed thread.
@param aCodeSegs list of all code segments loaded into process that crashed
@param aCodeSegOffsets output list of media-location offsets where segment data was stored
@return value of the media-position indicator at which the call started storing its data
@leave err one of the system wide error codes
*/
TUint CSymbianElfFormatter::WriteCodeSegmentsL( const TUint64 & aThreadId, const RCodeSegPointerList &aCodeSegs, RArray<TUint> &aCodeSegOffsets )
{

	LOG_MSG2( "CSymbianElfFormatter::WriteCodeSegmentsL(aThreadId=0x%X)\n", I64LOW(aThreadId) );

	TUint offset = iOffset;

	if(!iElfOptions->CreateCodeSegments())
    {
        LOG_MSG("CSymbianElfFormatter::WriteCodeSegmentsL - not configured to dump code segs");
        return iOffset;
    }

    for (TInt i = 0, count = aCodeSegs.Count(); i < count; i++)
    {
        aCodeSegOffsets.AppendL(0); //not dumping .text section, as it will be present on the host
	}

	return offset;
}

/**
Creates and saves Sym32_thrdinfod structures for all threads provided.
@param aPid The id of the process of the crashed thread.
@param aThreadList list of the threads in the crashed process or the whole system, depending on user configuration
@return value of the media-position indicator at which the call started storing its data
@leave err one of the system wide error codes
@see Sym32_thrdinfod
*/
TUint CSymbianElfFormatter::WriteThreadInfoL( const TUint64 & aPid, const RThreadPointerList &aThreadList )
{

	LOG_MSG2("->CSymbianElfFormatter::WriteThreadInfoL(aPid=0x%X)\n", I64LOW(aPid) );

	TUint offset = iOffset;

	if( !iElfOptions->CreateThreadSegments() )
	{
        LOG_MSG("CSymbianElfFormatter::WriteThreadInfoL - not configured to dump thread info");
        return iOffset;
    }

    Sym32_dhdr *descriptor = new(ELeave) Sym32_dhdr();
    CleanupStack::PushL(descriptor);

    TInt index = iStrInfoTbl->GetIndex(KThread);
    descriptor->d_name		= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
    descriptor->d_descrsz	= sizeof(Sym32_thrdinfod);
    descriptor->d_type		= ESYM_NOTE_THRD;
    descriptor->d_version	= iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
    descriptor->d_elemnum	= aThreadList.Count();

    iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
    iOffset += sizeof(Sym32_dhdr);
    CleanupStack::PopAndDestroy(descriptor);

    Sym32_thrdinfod *thrInfo = new(ELeave) Sym32_thrdinfod();
    CleanupStack::PushL(thrInfo);

    RBuf8 buf;
    buf.CreateL( KMaxFullName );
    CleanupClosePushL(buf);

    for( TInt i = 0; i < aThreadList.Count(); i++ )
    {
        //LOG_MSG3( "  Creating Sym32_thrdinfod for Thread[%d].Id()=0x%X", i, I64LOW( aThreadList[i]->Id()) );
        CThreadInfo &threadInfo = *aThreadList[i];

        buf.Copy( threadInfo.Name() );

        TInt index = KErrNotFound;
        if (buf.Length() > 0)
        {
            index = iStrInfoTbl->AddStringL(buf);
        }

        thrInfo->td_name = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
        thrInfo->td_id = threadInfo.Id();
        thrInfo->td_owning_process = threadInfo.ProcessId();
        thrInfo->td_priority = threadInfo.Priority();
        thrInfo->td_svc_sp = threadInfo.SvcStackPtr();
        thrInfo->td_svc_stack = threadInfo.SvcStackAddr();
        thrInfo->td_svc_stacksz = threadInfo.SvcStackSize();
        thrInfo->td_usr_stack = threadInfo.UsrStackAddr();
        thrInfo->td_usr_stacksz = threadInfo.UsrStackSize();

        iDataSave->WriteL( iOffset, thrInfo, sizeof(Sym32_thrdinfod));

        iOffset += sizeof(Sym32_thrdinfod);
    }

    CleanupStack::PopAndDestroy(2, thrInfo);
	return offset;
}

/**
Creates and saves Sym32_procinfod for all processes provided.
@param aPid The id of the process of the crashed thread.
@param aProcessList list of the processes holding the crashed process or the whole system, depending on user configuration
@return value of the media-position indicator at which the call started storing its data
@leave err one of the system wide error codes
@see Sym32_procinfod
*/
TUint CSymbianElfFormatter::WriteProcessInfoL( const TUint64 & aPid, const RProcessPointerList &aProcessList )
{

	LOG_MSG2("->CSymbianElfFormatter::WriteProcessInfoL(aPid=0x%X)\n", I64LOW(aPid) );

	TUint offset = iOffset;

	if (!iElfOptions->CreateProcessSegments())
		{
        LOG_MSG("CSymbianElfFormatter::WriteProcessInfoL - not configured to dump process info");
        return iOffset;
        }

    Sym32_dhdr *descriptor = new(ELeave) Sym32_dhdr();
    CleanupStack::PushL(descriptor);
    descriptor->d_elemnum = aProcessList.Count();

    TInt32 index = iStrInfoTbl->GetIndex(KProcess);
    descriptor->d_name		= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
    descriptor->d_descrsz	= sizeof(Sym32_procinfod);
    descriptor->d_type		= ESYM_NOTE_PROC;
    descriptor->d_version	= iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);

    iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
    iOffset += sizeof(Sym32_dhdr);

    Sym32_procinfod *symProcInfo = new(ELeave) Sym32_procinfod();
    CleanupStack::PushL(symProcInfo );

    RBuf8 buf;
    buf.CreateL( KMaxFullName );
    CleanupClosePushL(buf);

    //LOG_MSG2( "  Will create %d Sym32_procinfod structures\n", descriptor->d_elemnum );
    for(TInt i = 0; i < aProcessList.Count(); i++ )
        {
        CProcessInfo &procInfo = *aProcessList[i];

        //LOG_MSG3( "  Creating Sym32_procinfod for Proc[%d].Id()=0x%X", i, I64LOW( procInfo->Id() ) );

        symProcInfo->pd_id = procInfo.Id();

		RProcess proc;
		if( KErrNone == proc.Open( symProcInfo->pd_id ) )
			{
			symProcInfo->pd_priority = proc.Priority();
			proc.Close();
			}
		else
			{
			symProcInfo->pd_priority = 0;
			}

        buf.Copy( procInfo.Name() );

        index = KErrNotFound;
        if (buf.Length() > 0)
            {
            index = iStrInfoTbl->AddStringL(buf);
            }
        symProcInfo->pd_name = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;

        iDataSave->WriteL( iOffset, symProcInfo, sizeof(Sym32_procinfod));

        iOffset += sizeof(Sym32_procinfod);
        }

	CleanupStack::PopAndDestroy(3, descriptor);

	return offset;
}

/**
Creates and saves Sym32_execinfod structures for all the code segments provided
@param aCodeSegs list of all code segments loaded into process that crashed
@param aTime time of the crash.
@param aTid ID of the crashed thread
@return value of the media-position indicator at which the call started storing its data
@leave err one of the system wide error codes
@see Sym32_execinfod
*/
TUint CSymbianElfFormatter::WriteExecutableInfoL( const RCodeSegPointerList &aCodeSegs, const TUint64 &aTime, const TUint64 &aTid )
{
	LOG_MSG("CSymbianElfFormatter::WriteExecutableInfoL()\n");

	TUint offset = iOffset;

	if(!iElfOptions->CreateExecSegments())
        {
        LOG_MSG("CSymbianElfFormatter::WriteExecutableInfoL - not configured to dump executable info");
        return iOffset;
        }

    // Get Executable Info
    Sym32_dhdr *descriptor = new(ELeave) Sym32_dhdr();
    CleanupStack::PushL(descriptor);

    TInt32 index = iStrInfoTbl->GetIndex(KExecutable);
    descriptor->d_name		= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
    descriptor->d_descrsz	= sizeof(Sym32_execinfod);
    descriptor->d_type		= ESYM_NOTE_EXEC;
    descriptor->d_version	= iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
    descriptor->d_elemnum	= aCodeSegs.Count();

    //LOG_MSG3( "   Write ESYM_NOTE_EXEC size =%d at iOffset->%d\n", sizeof(Sym32_dhdr), iOffset );
    iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
    CleanupStack::PopAndDestroy(descriptor); // descriptor

    iOffset += sizeof(Sym32_dhdr);

    Sym32_execinfod *execInfo = new(ELeave) Sym32_execinfod();
    CleanupStack::PushL(execInfo);

    RBuf8 name;
    name.CreateL(KMaxFullName);
    CleanupClosePushL(name);

    for(TInt i = 0, count = aCodeSegs.Count(); i < count; i++)
        {
        TCodeSegInfo &segInfo = *aCodeSegs[i];
        name.Copy(segInfo.iName);
        index = KErrNotFound;

        if (name.Length() > 0)
        {
            index = iStrInfoTbl->AddStringL(name);
        }

        execInfo->ed_name = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
        //LOG_MSG2( "   execInfo->ed_name = %d\n", execInfo->ed_name );

        execInfo->ed_XIP = segInfo.iXIP;
        //LOG_MSG2( "   execInfo->ed_XIP = %d\n", execInfo->ed_XIP );

        execInfo->ed_execid.exec_id = aTime;
        execInfo->ed_execid.exec_crc = CalcExecCrcL(aTid, segInfo);

        execInfo->ed_codesize = segInfo.iCodeSize;
        //LOG_MSG2( "   execInfo->ed_codesize = 0x%x\n", execInfo->ed_codesize );
        execInfo->ed_coderunaddr = segInfo.iCodeRunAddr;
        //LOG_MSG2( "   execInfo->ed_coderunaddr = 0x%x\n", execInfo->ed_coderunaddr );
        execInfo->ed_codeloadaddr = segInfo.iCodeLoadAddr;
        //LOG_MSG2( "   execInfo->ed_codeloadaddr = 0x%x\n", execInfo->ed_codeloadaddr );
        execInfo->ed_rodatasize = segInfo.iRoDataSize;
        execInfo->ed_rodatarunaddr = segInfo.iRoDataRunAddr;
        execInfo->ed_rodataloadaddr = segInfo.iRoDataLoadAddr;
        execInfo->ed_datasize = segInfo.iDataSize;
        execInfo->ed_datarunaddr = segInfo.iDataRunAddr;
        //LOG_MSG2( "   execInfo->ed_datarunaddr = 0x%x\n", execInfo->ed_datarunaddr );
        execInfo->ed_dataloadaddr = segInfo.iDataLoadAddr;
        //LOG_MSG2( "   execInfo->ed_dataloadaddr = 0x%x\n", execInfo->ed_dataloadaddr );

        //LOG_MSG3( "   Write execInfo size =%d at iOffset->%d\n", sizeof(Sym32_execinfod), iOffset );
        iDataSave->WriteL( iOffset, execInfo, sizeof(Sym32_execinfod));

        iOffset += sizeof(Sym32_execinfod);
        }

    CleanupStack::PopAndDestroy(2, execInfo);

	return offset;
}

/**
The method finds all registers of a specific class (core register or coprocessor register)
and of a specific represenation (8-bit, 16-bit, 32-bit, 64-bit etc).

@param aRegs Input : all registers of all classes and representations.
@param aRegsOfSameClassRepr Output : All registers of the same class and representaion.
@param aClass Input : Which class to use as filter
@param aRepr INput : Which representation to use as filter.
*/
void CSymbianElfFormatter::FindAllOfSameClassRepresentation(RRegisterList &aRegs, RRegisterList &aRegsOfSameClassRepr, ESYM_REGCLASS aClass, ESYM_REGREP aRepr)
{
	for (TUint j = 0; j < aRegs.Count(); j++)
	{
		if ( aRegs[j].Available() &&
			(aRegs[j].GetClass() == aClass) &&
			(aRegs[j].GetSize() == aRepr) )
		{
			aRegsOfSameClassRepr.Append(aRegs[j]);
		}
	}
}

/**
 Write registers for threads. For non-crashed threads we save a thread's registers
 if the user setting CreateRegisterSegmentForAllThreadsInProcess() is true.
 For the crashed thread we save the thread's registers if the user setting
 CreateRegisterSegmentForCrashedThread() is true.
 We can only save register information for threads within the crashed process.
 The threads in the thread list may contain all threads in the system or only the
 threads in the crashed process, depending on user choice CreateSystemCrashInfo().
@param aTid The id of the crashed thread.
@param aPid The id of the process of the crashed thread.
@param aOffsets output array of offsets to each threads register information segment.
@param aArmExcRegs register context of the crashed thread
@param aThreadList list of the threads in the crashed process or the whole system, depending on user configuration
@leave err one of the system wide error codes
*/
void CSymbianElfFormatter::WriteRegisterInfoL( const TUint64 &aTid,
											   const TUint64 &aPid,
											   RArray<TUint>& aOffsets,
											   RRegisterList &aArmExcRegs,
                                               const RThreadPointerList &aThreadList )
{

	LOG_MSG2( "CSymbianElfFormatter::WriteRegisterInfoL(aTid=0x%X)\n", I64LOW(aTid) );

    if(!iElfOptions->CreateRegisterSegments())
	    {
        LOG_MSG("CSymbianElfFormatter::WriteRegisterInfoL - not configured to dump register info");
        return;
        }

    Sym32_dhdr *descriptor = new(ELeave) Sym32_dhdr();
    CleanupStack::PushL(descriptor);

    Sym32_reginfod *regInfo = new(ELeave) Sym32_reginfod();
    CleanupStack::PushL(regInfo);
    // For each thread, Get register info using ReadRegisters( threadId ...)
    for (TUint i = 0; i<aThreadList.Count(); i++)
    {

        if( aThreadList[i]->ProcessId() != aPid )
            {
            // Thread not in crashed process
            continue;
            }

        RRegisterList *regListPtr;
        RRegisterList regList;
        CleanupClosePushL(regList);

        if( aThreadList[i]->Id() == aTid )
            {
            // Use the registers at the time of the crash: aArmExcRegs
            regListPtr = &aArmExcRegs;
            }
        else
            {
            iDataSource->ReadRegistersL(aThreadList[i]->Id(), regList);
            regListPtr = &regList;
            }

        /*
        LOG_MSG3( "  Register list for thread 0x%X has %d entries",
            I64LOW(aThreadList[i]->Id()), regListPtr->Count() );
        */

        RRegisterList tmpRegList;
        CleanupClosePushL(tmpRegList);

        TBuf8<255> buf;
        buf.Format(_L8("CORE.SYMBIAN.REGISTER.0x%X%X"), I64HIGH(aThreadList[i]->Id()), I64LOW(aThreadList[i]->Id()));
        TInt32 index = iStrInfoTbl->AddStringL(buf);
        descriptor->d_name = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
        descriptor->d_type = ESYM_NOTE_REG;
        descriptor->d_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
        descriptor->d_elemnum = 1;

        TUint dataOffset = iOffset;
        FindAllOfSameClassRepresentation(*regListPtr, tmpRegList, ESYM_REG_CORE, ESYM_REG_8);

        LOG_MSG2( "  aOffsets.Count()=%d", aOffsets.Count() );

        if (tmpRegList.Count() > 0)
        {
            //LOG_MSG2( "  found %d ESYM_REG_8 registers", tmpRegList.Count() );
            // Writereg8data
            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                TUint8 content = tmpRegList[j].GetContent8();
                iDataSave->WriteL( iOffset, &content, sizeof(TUint8));
                iOffset += sizeof(TUint8);
            }

            // writedescr
            aOffsets.Append((TUint)iOffset);
            descriptor->d_descrsz   = sizeof(Sym32_reginfod) + tmpRegList.Count()*sizeof(Sym32_regdatad);
            iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
            iOffset += sizeof(Sym32_dhdr);

            regInfo->rid_class = ESYM_REG_CORE;
            regInfo->rid_num_registers = tmpRegList.Count();
            regInfo->rid_repre = ESYM_REG_8;
            regInfo->rid_thread_id = aThreadList[i]->Id();
            regInfo->rid_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);

            iDataSave->WriteL( iOffset, regInfo, sizeof(Sym32_reginfod));
            iOffset += sizeof(Sym32_reginfod);

            // writereginfod8
            Sym32_regdatad regData;

            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                regData.rd_id = tmpRegList[j].GetId();
                regData.rd_sub_id = tmpRegList[j].GetSubId();
                regData.rd_data = dataOffset + j*sizeof(TUint8);

                iDataSave->WriteL( iOffset, &regData, sizeof(Sym32_regdatad));
                iOffset += sizeof(Sym32_regdatad);
            }
        }
        tmpRegList.Reset();

        dataOffset = iOffset;
        FindAllOfSameClassRepresentation(*regListPtr, tmpRegList, ESYM_REG_CORE, ESYM_REG_16);
        if (tmpRegList.Count() > 0)
        {
            //LOG_MSG2( "  found %d ESYM_REG_16 registers", tmpRegList.Count() );
            // Writereg16data
            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                TUint16 content = tmpRegList[j].GetContent16();
                iDataSave->WriteL( iOffset, &content, sizeof(TUint16));
                iOffset += sizeof(TUint16);
            }

            // writedescr
            aOffsets.Append(iOffset);
            descriptor->d_descrsz   = sizeof(Sym32_reginfod) + tmpRegList.Count()*sizeof(Sym32_regdatad);
            iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
            iOffset += sizeof(Sym32_dhdr);

            // writereginfod16
            regInfo->rid_class = ESYM_REG_CORE;
            regInfo->rid_num_registers = tmpRegList.Count();
            regInfo->rid_repre = ESYM_REG_16;
            regInfo->rid_thread_id = aThreadList[i]->Id();
            regInfo->rid_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);

            iDataSave->WriteL( iOffset, regInfo, sizeof(Sym32_reginfod));
            iOffset += sizeof(Sym32_reginfod);

            // writereginfod8
            Sym32_regdatad regData;

            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                regData.rd_id = tmpRegList[j].GetId();
                regData.rd_sub_id = tmpRegList[j].GetSubId();
                regData.rd_data = dataOffset + j*sizeof(TUint16);

                iDataSave->WriteL( iOffset, &regData, sizeof(Sym32_regdatad));
                iOffset += sizeof(Sym32_regdatad);
            }
        }
        tmpRegList.Reset();

        dataOffset = iOffset;
        //LOG_MSG2( "  Write reg 32. dataOffset = iOffset = %d", iOffset );
        FindAllOfSameClassRepresentation(*regListPtr, tmpRegList, ESYM_REG_CORE, ESYM_REG_32);
        if (tmpRegList.Count() > 0)
        {
            //LOG_MSG2( "  found %d ESYM_REG_32 registers", tmpRegList.Count() );

            // WriteReg32data
            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                /*
                LOG_MSG3( "  Saving register Id=0x%X, val=0x%X",
                    tmpRegList[j].GetId(), tmpRegList[j].GetContent32() );
                */
                TUint32 content = tmpRegList[j].GetContent32();
                iDataSave->WriteL( iOffset, &content, sizeof(TUint32));
                iOffset += sizeof(TUint32);
            }

            //LOG_MSG2( "  ->aOffsets.Append(iOffset=%d)", iOffset );
            // writedescr
            aOffsets.Append(iOffset);
            descriptor->d_descrsz   = sizeof(Sym32_reginfod) + tmpRegList.Count()*sizeof(Sym32_regdatad);
            //LOG_MSG2( "  ->iDataSave->WriteL( iOffset=%d, descriptor, sizeof(Sym32_dhdr))", iOffset );
            iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
            iOffset += sizeof(Sym32_dhdr);
            //LOG_MSG2( "  iOffset += sizeof(Sym32_dhdr) = %d", iOffset );

            // writereginfod32
            regInfo->rid_class = ESYM_REG_CORE;
            regInfo->rid_num_registers = tmpRegList.Count();
            regInfo->rid_repre = ESYM_REG_32;
            regInfo->rid_thread_id = aThreadList[i]->Id();
            regInfo->rid_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);

            //LOG_MSG2( "  iDataSave->WriteL( iOffset = %d, regInfo, sizeof(Sym32_reginfod))", iOffset );
            iDataSave->WriteL( iOffset, regInfo, sizeof(Sym32_reginfod));
            iOffset += sizeof(Sym32_reginfod);
            //LOG_MSG2( "  iOffset += sizeof(Sym32_reginfod) = %d", iOffset );

            // writereginfod32
            Sym32_regdatad regData;

            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                regData.rd_id = tmpRegList[j].GetId();
                regData.rd_sub_id = tmpRegList[j].GetSubId();
                regData.rd_data = dataOffset + j*sizeof(TUint32);
                //LOG_MSG2( "  regData.rd_data = %d", regData.rd_data  );

                iDataSave->WriteL( iOffset, &regData, sizeof(Sym32_regdatad));
                iOffset += sizeof(Sym32_regdatad);
            }
        }
        tmpRegList.Reset();

        dataOffset = iOffset;
        FindAllOfSameClassRepresentation(*regListPtr, tmpRegList, ESYM_REG_CORE, ESYM_REG_64);
        if (tmpRegList.Count() > 0)
        {
            // WriteReg64data
            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                TUint64 content = tmpRegList[j].GetContent64();
                iDataSave->WriteL( iOffset, &content, sizeof(TUint64));
                iOffset += sizeof(TUint64);
            }

            // writedescr
            aOffsets.Append(iOffset);
            descriptor->d_descrsz   = sizeof(Sym32_reginfod)  + tmpRegList.Count()*sizeof(Sym32_regdatad);
            iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
            iOffset += sizeof(Sym32_dhdr);

            // writereginfod64
            regInfo->rid_class = ESYM_REG_CORE;
            regInfo->rid_num_registers = tmpRegList.Count();
            regInfo->rid_repre = ESYM_REG_64;
            regInfo->rid_thread_id = aThreadList[i]->Id();
            regInfo->rid_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);

            iDataSave->WriteL( iOffset, regInfo, sizeof(Sym32_reginfod));
            iOffset += sizeof(Sym32_reginfod);

            // writereginfod64
            Sym32_regdatad regData;

            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                regData.rd_id = tmpRegList[j].GetId();
                regData.rd_sub_id = tmpRegList[j].GetSubId();
                regData.rd_data = dataOffset + j*sizeof(TUint64);

                iDataSave->WriteL( iOffset, &regData, sizeof(Sym32_regdatad));
                iOffset += sizeof(Sym32_regdatad);
            }
        }
        tmpRegList.Reset();

        dataOffset = iOffset;
        FindAllOfSameClassRepresentation(*regListPtr, tmpRegList, ESYM_REG_COPRO, ESYM_REG_8);

        if (tmpRegList.Count() > 0)
        {
            // Writereg8data
            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                TUint8 content = tmpRegList[j].GetContent8();
                iDataSave->WriteL( iOffset, &content, sizeof(TUint8));
                iOffset += sizeof(TUint8);
            }

            // writedescr
            aOffsets.Append(iOffset);
            descriptor->d_descrsz   = sizeof(Sym32_reginfod)  + tmpRegList.Count()*sizeof(Sym32_regdatad);
            iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
            iOffset += sizeof(Sym32_dhdr);

            regInfo->rid_class = ESYM_REG_COPRO;
            regInfo->rid_num_registers = tmpRegList.Count();
            regInfo->rid_repre = ESYM_REG_8;
            regInfo->rid_thread_id = aThreadList[i]->Id();
            regInfo->rid_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);

            iDataSave->WriteL( iOffset, regInfo, sizeof(Sym32_reginfod));
            iOffset += sizeof(Sym32_reginfod);

            // writereginfod8
            Sym32_regdatad regData;

            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                regData.rd_id = tmpRegList[j].GetId();
                regData.rd_sub_id = tmpRegList[j].GetSubId();
                regData.rd_data = dataOffset + j*sizeof(TUint8);

                iDataSave->WriteL( iOffset, &regData, sizeof(Sym32_regdatad));
                iOffset += sizeof(Sym32_regdatad);
            }
        }
        tmpRegList.Reset();

        dataOffset = iOffset;
        FindAllOfSameClassRepresentation(*regListPtr, tmpRegList, ESYM_REG_COPRO, ESYM_REG_16);
        if (tmpRegList.Count() > 0)
        {
            // Writereg16data
            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                TUint16 content = tmpRegList[j].GetContent16();
                iDataSave->WriteL( iOffset, &content, sizeof(TUint16));
                iOffset += sizeof(TUint16);
            }

            // writedescr
            aOffsets.Append(iOffset);
            descriptor->d_descrsz   = sizeof(Sym32_reginfod)  + tmpRegList.Count()*sizeof(Sym32_regdatad);
            iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
            iOffset += sizeof(Sym32_dhdr);

            // writereginfod16
            regInfo->rid_class = ESYM_REG_COPRO;
            regInfo->rid_num_registers = tmpRegList.Count();
            regInfo->rid_repre = ESYM_REG_16;
            regInfo->rid_thread_id = aThreadList[i]->Id();
            regInfo->rid_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);

            iDataSave->WriteL( iOffset, regInfo, sizeof(Sym32_reginfod));
            iOffset += sizeof(Sym32_reginfod);

            // writereginfod8
            Sym32_regdatad regData;

            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                regData.rd_id = tmpRegList[j].GetId();
                regData.rd_sub_id = tmpRegList[j].GetSubId();
                regData.rd_data = dataOffset + j*sizeof(TUint16);

                iDataSave->WriteL( iOffset, &regData, sizeof(Sym32_regdatad));
                iOffset += sizeof(Sym32_regdatad);
            }
        }
        tmpRegList.Reset();

        dataOffset = iOffset;
        FindAllOfSameClassRepresentation(*regListPtr, tmpRegList, ESYM_REG_COPRO, ESYM_REG_32);
        if (tmpRegList.Count() > 0)
        {
            //LOG_MSG2( "  found %d ESYM_REG_32 CoPro registers", tmpRegList.Count() );

            // WriteReg32data
            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                /*
                LOG_MSG4( "  Saving CoPro register Id=0x%X, SubId=0x%X, val=0x%X",
                    tmpRegList[j].GetId(), tmpRegList[j].GetSubId(), tmpRegList[j].GetContent32() );
                */
                TUint32 content = tmpRegList[j].GetContent32();
                iDataSave->WriteL( iOffset, &content, sizeof(TUint32));
                iOffset += sizeof(TUint32);
            }

            // writedescr
            aOffsets.Append(iOffset);
            descriptor->d_descrsz   = sizeof(Sym32_reginfod)  + tmpRegList.Count()*sizeof(Sym32_regdatad);
            iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
            iOffset += sizeof(Sym32_dhdr);

            // writereginfod32
            regInfo->rid_class = ESYM_REG_COPRO;
            regInfo->rid_num_registers = tmpRegList.Count();
            regInfo->rid_repre = ESYM_REG_32;
            regInfo->rid_thread_id = aThreadList[i]->Id();
            regInfo->rid_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);

            iDataSave->WriteL( iOffset, regInfo, sizeof(Sym32_reginfod));
            iOffset += sizeof(Sym32_reginfod);

            // writereginfod32
            Sym32_regdatad regData;

            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                regData.rd_id = tmpRegList[j].GetId();
                regData.rd_sub_id = tmpRegList[j].GetSubId();
                regData.rd_data = dataOffset + j*sizeof(TUint32);

                iDataSave->WriteL( iOffset, &regData, sizeof(Sym32_regdatad));
                iOffset += sizeof(Sym32_regdatad);
            }
        }
        tmpRegList.Reset();

        dataOffset = iOffset;
        FindAllOfSameClassRepresentation(*regListPtr, tmpRegList, ESYM_REG_COPRO, ESYM_REG_64);
        if (tmpRegList.Count() > 0)
        {
            // WriteReg64data
            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                TUint64 content = tmpRegList[j].GetContent64();
                iDataSave->WriteL( iOffset, &content, sizeof(TUint64));
                iOffset += sizeof(TUint64);
            }

            // writedescr
            aOffsets.Append(iOffset);
            descriptor->d_descrsz   = sizeof(Sym32_reginfod)  + tmpRegList.Count()*sizeof(Sym32_regdatad);
            iDataSave->WriteL( iOffset, descriptor, sizeof(Sym32_dhdr));
            iOffset += sizeof(Sym32_dhdr);

            // writereginfod64
            regInfo->rid_class = ESYM_REG_COPRO;
            regInfo->rid_num_registers = tmpRegList.Count();
            regInfo->rid_repre = ESYM_REG_64;
            regInfo->rid_thread_id = aThreadList[i]->Id();
            regInfo->rid_version = iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);

            iDataSave->WriteL( iOffset, regInfo, sizeof(Sym32_reginfod));
            iOffset += sizeof(Sym32_reginfod);

            // writereginfod64
            Sym32_regdatad regData;

            for (TUint j = 0; j < tmpRegList.Count(); j++)
            {
                regData.rd_id = tmpRegList[j].GetId();
                regData.rd_sub_id = tmpRegList[j].GetSubId();
                regData.rd_data = dataOffset + j*sizeof(TUint64);

                iDataSave->WriteL( iOffset, &regData, sizeof(Sym32_regdatad));
                iOffset += sizeof(Sym32_regdatad);
            }
        }

        //LOG_MSG( "  CleanupStack::PopAndDestroy( tmpRegList )" );
        CleanupStack::PopAndDestroy(&tmpRegList); // tmpRegList. Will call Close().

        //LOG_MSG( "  CleanupStack::PopAndDestroy(regList)" );
        CleanupStack::PopAndDestroy(&regList); // regList. Will call Close().
    }
    //LOG_MSG( "  CleanupStack::PopAndDestroy(descriptor)" );
    CleanupStack::PopAndDestroy(2, descriptor);

	//LOG_MSG2( "  Final aOffsets.Count()=%d", aOffsets.Count() );
}

/**
Saves content of the string info segment.
@return value of the media-position indicator at which the call started storing its data
*/
TUint CSymbianElfFormatter::WriteStringInfoL()
{
	TUint offset = iOffset;

	Sym32_dhdr *strInfo_descriptor = new(ELeave) Sym32_dhdr();
	CleanupStack::PushL(strInfo_descriptor);
	TInt32 index = iStrInfoTbl->GetIndex(KStr);
	strInfo_descriptor->d_name		= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
	strInfo_descriptor->d_descrsz	= iStrInfoTbl->GetSize();
	strInfo_descriptor->d_type		= ESYM_NOTE_STR;
	strInfo_descriptor->d_version	= iStrInfoTbl->GetIndex(KSymbianElfCoreDumpVersion);
    strInfo_descriptor->d_elemnum	= 1;
	LOG_MSG2( "   Writing Sym32_dhdr strInfo_descriptor at iOffset -> %d\n", iOffset );
	iDataSave->WriteL( iOffset, strInfo_descriptor, sizeof(Sym32_dhdr));
	CleanupStack::PopAndDestroy(strInfo_descriptor); // strInfo_descriptor
	iOffset += sizeof(Sym32_dhdr);

	// Write all strings
	TText8* buf = iStrInfoTbl->GetAllStrings();
	TUint size = iStrInfoTbl->GetSize();

	LOG_MSG2( "   Writing strings at iOffset -> %d\n", iOffset );
	iDataSave->WriteL( iOffset, buf, size);
	LOG_MSG2( "   Strings size=%d\n", size);
	iOffset += size;

	return offset;
}

/**
Returns requested register value.
@param aCrashInfo TCrashInfo data structure holding information about the crash.
@param aReg structure holding the information about the register caller wants the value of
@param aVal output parameter where the register value is being stored
@return ETrue if was able to get register value, EFalse otherwise
*/
TBool CSymbianElfFormatter::GetArmExcReg(const TCrashInfo &aCrashInfo, const TRegisterData & aReg, TRegisterValue32 & aVal)
	{

	if( ESYM_REG_CORE != aReg.GetClass() )
		{
		return EFalse;
		}

	switch( aReg.GetId() )
		{
		case ERegisterR0:
			aVal = aCrashInfo.iContext.iR0;
			break;
		case ERegisterR1:
			aVal = aCrashInfo.iContext.iR1;
			break;
		case ERegisterR2:
			aVal = aCrashInfo.iContext.iR2;
			break;
		case ERegisterR3:
			aVal = aCrashInfo.iContext.iR3;
			break;
		case ERegisterR4:
			aVal = aCrashInfo.iContext.iR4;
			break;
		case ERegisterR5:
			aVal = aCrashInfo.iContext.iR5;
			break;
		case ERegisterR6:
			aVal = aCrashInfo.iContext.iR6;
			break;
		case ERegisterR7:
			aVal = aCrashInfo.iContext.iR7;
			break;
		case ERegisterR8:
			aVal = aCrashInfo.iContext.iR8;
			break;
		case ERegisterR9:
			aVal = aCrashInfo.iContext.iR9;
			break;
		case ERegisterR10:
			aVal = aCrashInfo.iContext.iR10;
			break;
		case ERegisterR11:
			aVal = aCrashInfo.iContext.iR11;
			break;
		case ERegisterR12:
			aVal = aCrashInfo.iContext.iR12;
			break;
		case ERegisterR13:
			aVal = aCrashInfo.iContext.iR13;
			break;
		case ERegisterR14:
			aVal = aCrashInfo.iContext.iR14;
			break;
		case ERegisterR15:
			aVal = aCrashInfo.iContext.iR15;
			break;
		case ERegisterCpsr:
			aVal = aCrashInfo.iContext.iCpsr;
			break;
		case ERegisterR13Svc:
			aVal = aCrashInfo.iContext.iR13Svc;
			break;
		case ERegisterR14Svc:
			aVal = aCrashInfo.iContext.iR14Svc;
			break;
		case ERegisterSpsrSvc:
			aVal = aCrashInfo.iContext.iSpsrSvc;
			break;
		default:
			return EFalse;
		}

	return ETrue;
	}

/**
 This method is responsible for creating a member variable that is a list of the
 registers available at the time of the crash for the crashed thread.
 The registers not passed to the formatter at the time of the crash are
 obtained from the data source API.
@param aCrashInfo TCrashInfo data structure holding information about the crash.
@param aArmExcRegs output list of registers
@leave err one of the system wide error codes
 */
void CSymbianElfFormatter::SetArmRegsFromCrashDataL(const TCrashInfo &aCrashInfo, RRegisterList &aArmExcRegs)
	{
	LOG_MSG("->CSymbianElfFormatter::SetArmRegsFromCrashDataL()\n");

//    aArmExcRegs.Reset();
	// Read all registers first, then overwrite values with what we get
	// from crash data, if available
	iDataSource->ReadRegistersL( aCrashInfo.iTid, aArmExcRegs );

	if( TCrashInfo::ECrashException != aCrashInfo.iType )
		{
		// The registers for the crashed thread are only available for exceptions.
		// When not an exception, use the registers are returned by the ReadRegisters call
		// This is not an error
		return;
		}

	TRegisterValue32 val;

	for(TInt numRegs=0; numRegs < aArmExcRegs.Count(); numRegs ++ )
		{
		if( GetArmExcReg( aCrashInfo, aArmExcRegs[numRegs], val ) )
			{
			//LOG_MSG3( "  Setting reg[%d] to =0x%X", numRegs, val );
			aArmExcRegs[numRegs].SetContent( (TUint) val );
			aArmExcRegs[numRegs].SetAvailable( ETrue );
			}
		}

	// Now add the FSR and FAR to the available registers for the crashed thread
	// These are Co-Pro registers
	TRegisterData reg;
	reg.iId			= 15; // Co-Pro 15
	reg.iSize		= ESYM_REG_32;
	reg.iRegClass	= ESYM_REG_COPRO;
	reg.iAvailable	= ETrue;

	/*
	Create two Co-Processor registers to represent the fault status FSR and
	fault address FAR values. The fault status and address values depend on
	the type of exception. See /e32/nkern/arm/vectors.cia

	The sub id of the register is based on the ARM instruction used to access the reg:

                             Op1 CRn CRm Op2=0. Op1 bits 10-8, CRn bits 7-4
      fault address  mrc p15, 0, c6, c0  =>          0       ,    6
      fault status   mrc p15, 0, c5, c0  =>          0       ,    5
    */

	reg.iSubId = 6;
	reg.iSubId <<= 4;
	reg.SetContent( aCrashInfo.iContext.iFaultAddress );
	reg.SetAvailable( ETrue );
	aArmExcRegs.AppendL( reg );

	reg.iSubId = 5;
	reg.iSubId <<= 4;
	reg.SetContent( aCrashInfo.iContext.iFaultStatus );
	reg.SetAvailable( ETrue );
	aArmExcRegs.AppendL( reg );
	}

/**
 * Prepares the name of the dump file. Checks DataSave filename option, if it is provided by the user
 * it appends the time stamp, if not it uses default name appended with the time stamp.
 * @param aStamp lower 32 bits of the time stamp
 * @leave err one of system wide error codes
 */
void CSymbianElfFormatter::PrepareFileNameL(TUint64 aStamp)
{
	LOG_MSG("->CSymbianElfFormatterV2::PrepareFileNameL()\n");

	COptionConfig *option = iDataSave->GetConfigParameterL(CCrashDataSave::ECoreFilePath);
	const TDesC &userFileName = option->ValueAsDesc();

	if(userFileName.Length() == 0)
		{
		_LIT(KTxtFileFormat, "E:\\core%u.elf");
		iElfFileName.Format(KTxtFileFormat, aStamp);
		}
	else
		{
		iElfFileName = userFileName;
		}
}

/**
    Cleanup item implementation for thread list in CrashEventL() method.
@param aArray pointer to the list that is supposed to be freed
*/
void CSymbianElfFormatter::CleanupCodeSegList(TAny *aArray)
{
    LOG_MSG("->CSymbianElfFormatter::CleanupCodeSegList()\n");
	RCodeSegPointerList *codeSegList = static_cast<RCodeSegPointerList*> (aArray);
    codeSegList->ResetAndDestroy();
    codeSegList->Close();
    delete codeSegList;
}

/**
    Cleanup item implementation for thread list in CrashEventL() method.
@param aArray pointer to the list that is supposed to be freed
*/
void CSymbianElfFormatter::CleanupProcessList(TAny *aArray)
{
    LOG_MSG("->CSymbianElfFormatter::CleanupProcessList()\n");
	RProcessPointerList *processList = static_cast<RProcessPointerList*> (aArray);
    processList->ResetAndDestroy();
    processList->Close();
    delete processList;
}

/**
    Cleanup item implementation for thread list in CrashEventL() method.
@param aArray pointer to the list that is supposed to be freed
*/
void CSymbianElfFormatter::CleanupThreadList(TAny *aArray)
{
    LOG_MSG("->CSymbianElfFormatter::CleanupThreadList()\n");
	RThreadPointerList *threadList = static_cast<RThreadPointerList*> (aArray);
    threadList->ResetAndDestroy();
    threadList->Close();
    delete threadList;
}

/**
 Handle a crash event and generate Symbian ELF formatted crash data. Write the data using the
 provided data save interface.
@param aCrashInfo Crash Data @see TCrashInfo.
@leave err one of the system wide error codes
*/
void CSymbianElfFormatter::CrashEventL( TCrashInfo* aCrashInfo )
{
	LOG_MSG("->CSymbianElfFormatter::CrashEvent (SELF V1)\n");

    //LOG_MSG("CSymbiaElfFormatter::CrashEvenL - checking if crash info is sane\n");
    User::LeaveIfNull(aCrashInfo);
    //LOG_MSG("CSymbianElfFormatter::CrashEvenL - checking if data source is sane\n");
    User::LeaveIfNull(iDataSource);
    //LOG_MSG("CSymbianElfFormatter::CrashEvenL - checking if data save is sane\n");
    User::LeaveIfNull(iDataSave);

    _LIT(KProgressStart, "-");
    UpdateCrashProgressL( KProgressStart );

    RRegisterList armExcRegs;
    CleanupClosePushL(armExcRegs);
    SetArmRegsFromCrashDataL(*aCrashInfo, armExcRegs);

	iStrInfoTbl = CStringInfoTable::NewLC();
	PrepareFileNameL(aCrashInfo->iTime);
	iDataSave->OpenL(iElfFileName);

	// This is a workaround for fole media, since it does not support
	// saving beyond the current file size. At the start this is 0, so we
	// must save this data even with dummy values.
	Elf32_Ehdr *elfHdr = new(ELeave) Elf32_Ehdr();
	CleanupStack::PushL(elfHdr);
	iDataSave->WriteL( 0, elfHdr, sizeof(Elf32_Ehdr));

	// Move file pointer to skip Elf Header as the info in that struct isn't known
	// until everything has been written.
	iOffset = sizeof(Elf32_Ehdr);
	//LOG_MSG2( " iOffset start ->%d", iOffset );

    RProcessPointerList *processList = new(ELeave)RProcessPointerList;
    TCleanupItem processCleanup(CSymbianElfFormatter::CleanupProcessList, (TAny*)processList);
    CleanupStack::PushL(processCleanup);

    //LOG_MSG("CSymbianElfFormatter::CrashEvenL - getting process list\n");
	iDataSource->GetProcessListL( *processList );
	// Search processList for the crashed process, then get its details
    CProcessInfo *crashedProc = NULL;
    TInt i = 0;
	for (i = 0; i < processList->Count(); i++ )
		{
		if( (*processList)[i]->Id() == aCrashInfo->iPid )
			{
            LOG_MSG("CSymbianElfFormatter::CrashEventL - found crashed process on the process list\n");
			crashedProc = (*processList)[i];
			break;
			}
		}

	if( !crashedProc )
		{
		LOG_MSG( "  ERROR CProcessInfo == NULL. Crashed proc not found in process list" );
		User::Leave( KErrBadHandle );
		return; // Here for Coverity reasons
		}

    if(!iElfOptions->CreateSystemCrashInfo())
        {
		LOG_MSG3( "  Not generating SystemCrashInfo, only keeping crashed proc i=%d, 0x%X in proc list",
			i, I64LOW(crashedProc->Id()));
        processList->Remove(i);
        processList->ResetAndDestroy();
        processList->AppendL(crashedProc);
        }

	LOG_MSG2( " processList->Count()=%d", processList->Count() );

	TUint64 threadRequestPid;
	if( iElfOptions->CreateSystemCrashInfo() )
		{
		// All threads in the system
		threadRequestPid = (TUint64)-1;
		//LOG_MSG( "  iElfOptions->CreateSystemCrashInfo() is True => Get All System Threads" );
		}
	else
		{
		// This should only return the threads under the crashed process
		//LOG_MSG( "  iElfOptions->CreateSystemCrashInfo() is False => Get Process Specific Threads" );
		threadRequestPid = aCrashInfo->iPid;
		}

    RThreadPointerList *threadList = new(ELeave) RThreadPointerList;
    TCleanupItem threadCleanup(CSymbianElfFormatter::CleanupThreadList, (TAny*)threadList);
    CleanupStack::PushL(threadCleanup);

    //LOG_MSG("CSymbianElfFormatter::CrashEvenL - getting thread list\n");
	iDataSource->GetThreadListL( threadRequestPid, *threadList );

 	if(!iElfOptions->CreateProcessData())
        {

		LOG_MSG2( " threadList->Count()=%d before removal of non-crashed threads in process", threadList->Count() );
		// User does not want threads from the crashed process. So remove these threads from the
		// threads list. We still want the crashed thread and any other threads not belonging to
		// this process.
		TBool found;
		do
			{

			found = EFalse;
			for(TInt i =0; i < threadList->Count(); i++)
				{
				if( ((*threadList)[i]->ProcessId() == aCrashInfo->iPid) &&
					((*threadList)[i]->Id() != aCrashInfo->iTid) )
					{
					LOG_MSG2("  removing thread 0x%X\n", I64LOW((*threadList)[i]->Id()) );
					CThreadInfo *someThread = (*threadList)[i];
					threadList->Remove(i);
					delete someThread;
					found = ETrue;
					break;
					}
				}

			} while( found );

		}

	LOG_MSG2( " threadList->Count()=%d", threadList->Count() );

    RCodeSegPointerList *codeSegList = new(ELeave) RCodeSegPointerList;
    TCleanupItem codeSegCleanup(CSymbianElfFormatter::CleanupCodeSegList, (TAny*)codeSegList);
    CleanupStack::PushL(codeSegCleanup);

    iDataSource->GetCodeSegmentsL(aCrashInfo->iTid, *codeSegList);

    TCodeSegInfo *execInfo = NULL;
    for(TInt i = 0; i < codeSegList->Count(); i++)
        {
        if((*codeSegList)[i]->iType == EExeCodeSegType)
            {
            LOG_MSG("CSymbianElfFormatter::CrashEventL - found exe code seg list\n");
            execInfo = (*codeSegList)[i];
            break;
            }
        }

    if(!execInfo)
		{
		LOG_MSG( "  ERROR TCodeSegInfo == NULL. Crashed exec not found in code seg list" );
		User::Leave( KErrBadHandle );
		return; // Here for Coverity reasons
		}

	TUint32 crc32 = CalcExecCrcL(aCrashInfo->iTid, *execInfo);

    _LIT(KWriteSymbianInfoProgress, "Write Symbian Info");
    UpdateCrashProgressL( KWriteSymbianInfoProgress );
	TUint symbianInfoOffset = WriteSymbianInfoL(*aCrashInfo, crc32);

    RSegmentList dataSegs;
    CleanupClosePushL(dataSegs);
    RArray<TUint> dataSegOffsets;
    CleanupClosePushL(dataSegOffsets);

    _LIT( KWriteDataSegProgress, "Write Data Segments" );
    UpdateCrashProgressL( KWriteDataSegProgress );
	TUint dataSegmentsOffset = WriteDataSegmentsL( aCrashInfo->iTid, aCrashInfo->iPid, *threadList, *codeSegList, dataSegs, dataSegOffsets );

    RArray<TUint> codeSegOffsets;
    CleanupClosePushL(codeSegOffsets);

    _LIT( KWriteCodeSegProgress, "Write Code Segments" );
    UpdateCrashProgressL( KWriteCodeSegProgress );
	TUint codeSegmentsOffset = WriteCodeSegmentsL( aCrashInfo->iTid, *codeSegList, codeSegOffsets );

	//LOG_MSG2( "   iOffset before WriteThreadInfoL-> %d\n", iOffset );
    _LIT( KWriteThreadProgress, "Write Thread Data" );
    UpdateCrashProgressL( KWriteThreadProgress );
	TUint threadInfoOffset = WriteThreadInfoL( aCrashInfo->iPid, *threadList );

	//LOG_MSG2( "   iOffset before WriteProcessInfoL-> %d\n", iOffset );
    _LIT( KWriteProcessProgress, "Write Process Data" );
    UpdateCrashProgressL( KWriteProcessProgress );
	TUint processInfoOffset = WriteProcessInfoL( aCrashInfo->iPid, *processList );

	//LOG_MSG2( "   iOffset before WriteExecutableInfoL-> %d\n", iOffset );
    _LIT( KWriteExeProgress, "Write Executable Data" );
    UpdateCrashProgressL( KWriteExeProgress );
	TUint executableInfoOffset = WriteExecutableInfoL( *codeSegList, aCrashInfo->iTime, aCrashInfo->iTid );
	//LOG_MSG2( "   iOffset after WriteExecutableInfoL-> %d\n", iOffset );

	RArray<TUint> registerInfoOffsets;
	CleanupClosePushL(registerInfoOffsets);

    _LIT( KWriteRegProgress, "Write Register Data" );
    UpdateCrashProgressL( KWriteRegProgress );
	//LOG_MSG( "  -> WriteRegisterInfoL()\n" );
	WriteRegisterInfoL(aCrashInfo->iTid, aCrashInfo->iPid, registerInfoOffsets, armExcRegs, *threadList);
	//LOG_MSG( "  <- WriteRegisterInfoL()\n" );
	TUint stringInfoOffset = WriteStringInfoL();

	LOG_MSG2( "   programTableOffset set to -> %d\n", iOffset );
	TUint programTableOffset = iOffset;

	// Create program header table 2.3 by collecting and assembling all bits and pieces needed

	TUint noProgramTblEntries = 0;

    _LIT( KCreatePHdrProgress, "Write ELF Headers" );
    UpdateCrashProgressL( KCreatePHdrProgress );

	Elf32_Phdr *header = new(ELeave) Elf32_Phdr();
	CleanupStack::PushL(header);

	// create program header with p_type=PT_NOTE and p_offset pointing to Symbian Info
	header->p_type = PT_NOTE;
	header->p_offset = symbianInfoOffset;
	//LOG_MSG2( "   Symbian Info Elf32_Phdr.p_offset points to -> %d\n", header->p_offset );
	header->p_vaddr = 0;
	header->p_paddr = 0;
	header->p_filesz = 0;
	header->p_memsz = 0;
	header->p_flags = 0; //no flags for note segments
	header->p_align = 4;
	//LOG_MSG2( "   Writing 1st Elf32_Phdr (Symbian Info ) at offset %d\n", iOffset );
	iDataSave->WriteL( iOffset, header, sizeof(Elf32_Phdr));

	iOffset += sizeof(Elf32_Phdr);
	noProgramTblEntries++;

	if( iElfOptions->CreateDataSegments() )
	{
        LOG_MSG2("SymbianElfFormatter:: dataSegs.Count90 = %d", dataSegs.Count());
		for (TInt i=0; i < dataSegs.Count(); i++) // each segment in segs
		{

			header->p_type = PT_LOAD;
			header->p_offset =  dataSegOffsets[i];
			LOG_MSG2( "   header->p_offset = dataSegmentsOffset ->=%d\n", header->p_offset );
			header->p_vaddr = dataSegs[i].iSegBase;
			header->p_paddr = 0;
			header->p_filesz = dataSegs[i].iSegSize;
			header->p_memsz = dataSegs[i].iSegSize;
			header->p_flags = dataSegs[i].iFlags;
			header->p_align = 4;
			LOG_MSG2( "   Writing this Data Seg PT_LOAD at offset %d\n", iOffset );
			iDataSave->WriteL( iOffset, header, sizeof(Elf32_Phdr));
			iOffset += sizeof(Elf32_Phdr);
			noProgramTblEntries++;
		}
	}

	if( iElfOptions->CreateCodeSegments() )
	{
		for (TInt i=0; i < codeSegList->Count(); i++) // each segment in segs
		{
			header->p_type = PT_LOAD;
			header->p_offset =  codeSegOffsets[i];
			//LOG_MSG2( "   header->p_offset = codeSegmentsOffset ->=%d\n", header->p_offset );
			header->p_vaddr = (*codeSegList)[i]->iCodeRunAddr;
			header->p_paddr = 0;
			header->p_filesz = 0; //not dumping the code text section
			header->p_memsz = (*codeSegList)[i]->iCodeSize;
			header->p_flags = PF_R | PF_X; // Code segment
			header->p_align = 4;
			//LOG_MSG2( "   Writing this Code Seg PT_LOAD at offset %d\n", iOffset );
			iDataSave->WriteL( iOffset, header, sizeof(Elf32_Phdr));
			iOffset += sizeof(Elf32_Phdr);
			noProgramTblEntries++;
		}
	}

	if( iElfOptions->CreateThreadSegments() )
	{

		LOG_MSG( "  iElfOptions->CreateThreadSegments is True => creating P Hdr thread\n" );

		// create program header for crashed thread segment
		header->p_type = PT_NOTE;
		header->p_offset =  threadInfoOffset;
		LOG_MSG2( "   header->p_offset = threadInfoOffset=%d\n", threadInfoOffset );
		header->p_vaddr = 0;
		header->p_paddr = 0;
		header->p_filesz = 0;
		header->p_memsz = 0;
		header->p_flags = 0; //no flags for note segments
		header->p_align = 4;
		LOG_MSG2( "   Writing this PT_NOTE at offset %d\n", iOffset );
		iDataSave->WriteL( iOffset, header, sizeof(Elf32_Phdr));
		iOffset += sizeof(Elf32_Phdr);
		noProgramTblEntries++;
	}

	if (iElfOptions->CreateProcessSegments())
	{
		LOG_MSG( "  iElfOptions->CreateProcessSegmentInfo is True => creating P Hdr Proc\n" );

		// create program header with p_type=PT_NOTE and p_offset pointing to...
		header->p_type = PT_NOTE;
		header->p_offset = processInfoOffset;
		LOG_MSG2( "   header->p_offset = processInfoOffset=%d\n", processInfoOffset );
		header->p_vaddr = 0;
		header->p_paddr = 0;
		header->p_filesz = 0;
		header->p_memsz = 0;
		header->p_flags = 0; //no flags for note segments
		header->p_align = 4;
		LOG_MSG2( "   Writing this PT_NOTE at offset %d\n", iOffset );
		iDataSave->WriteL( iOffset, header, sizeof(Elf32_Phdr));
		iOffset += sizeof(Elf32_Phdr);
		noProgramTblEntries++;
	}

	if( iElfOptions->CreateExecSegments() )
	{
		LOG_MSG2( "  iElfOptions->CreateExecInfo is True => creating P Hdr Proc at offset %d\n",
			executableInfoOffset );

		// create program header with p_type=PT_NOTE and p_offset pointing to...
		header->p_type = PT_NOTE;
		header->p_offset = executableInfoOffset;
		LOG_MSG2( "   header->p_offset = executableInfoOffset=%d\n", executableInfoOffset );
		header->p_vaddr = 0;
		header->p_paddr = 0;
		header->p_filesz = 0;
		header->p_memsz = 0;
		header->p_flags = 0; //no flags for note segments
		header->p_align = 4;
		LOG_MSG2( "   Writing this PT_NOTE at offset %d\n", iOffset );
		iDataSave->WriteL( iOffset, header, sizeof(Elf32_Phdr));
		iOffset += sizeof(Elf32_Phdr);
		noProgramTblEntries++;
	}

	if( registerInfoOffsets.Count() )
	{
		// create program header with p_type=PT_NOTE and p_offset pointing to Register info

		for (TUint i = 0; i<registerInfoOffsets.Count(); i++)
		{
			header->p_type = PT_NOTE;
			header->p_offset = registerInfoOffsets[i];
			LOG_MSG2( "    => header->p_offset Register Seg P Hdr offset ->%d\n", header->p_offset );
			header->p_vaddr = 0;
			header->p_paddr = 0;
			header->p_filesz = 0;
			header->p_memsz = 0;
			header->p_flags = 0; //no flags for note segments
			header->p_align = 4;
			LOG_MSG2( "   Writing this PT_NOTE at offset %d\n", iOffset );
			iDataSave->WriteL( iOffset, header, sizeof(Elf32_Phdr));
			iOffset += sizeof(Elf32_Phdr);
			noProgramTblEntries++;
		}
	}

	//LOG_MSG( "   Writing String PT_NOTE\n" );
	header->p_type = PT_NOTE;
	header->p_offset = stringInfoOffset;
	LOG_MSG2( "   Creating String table at offset->%d\n", header->p_offset );
	LOG_MSG2( "   header->p_offset = stringInfoOffset=%d\n", stringInfoOffset );
	header->p_vaddr = 0;
	header->p_paddr = 0;
	header->p_filesz = 0;
	header->p_memsz = 0;
	header->p_flags = 0; //no flags for string segment
	header->p_align = 4;
	LOG_MSG2( "   Writing this PT_NOTE at offset %d\n", iOffset );
	iDataSave->WriteL( iOffset, header, sizeof(Elf32_Phdr));
	noProgramTblEntries++;

	CleanupStack::PopAndDestroy(header);

	// update the Elf32 header and write it at the beginning of the file
	elfHdr->e_ident[EI_MAG0] = ELFMAG0;
	elfHdr->e_ident[EI_MAG1] = ELFMAG1;
	elfHdr->e_ident[EI_MAG2] = ELFMAG2;
	elfHdr->e_ident[EI_MAG3] = ELFMAG3;
	elfHdr->e_ident[EI_CLASS] = ELFCLASS32;
	elfHdr->e_ident[EI_DATA] = ELFDATA2LSB;
	elfHdr->e_ident[EI_VERSION] = 0;
	elfHdr->e_ident[EI_PAD] = 0;
	elfHdr->e_type = ET_CORE;
	elfHdr->e_machine = EM_ARM;
	elfHdr->e_version = EV_CURRENT;
	elfHdr->e_entry = 0;
	elfHdr->e_phoff = programTableOffset;
	LOG_MSG2( "   Program header offset elfHdr->e_phoff = programTableOffset=%d\n", programTableOffset );
	elfHdr->e_shoff = 0;
	elfHdr->e_flags = 0;
	LOG_MSG2( "   elfHdr->e_ehsize = %d\n", sizeof(Elf32_Ehdr) );
	elfHdr->e_ehsize = sizeof(Elf32_Ehdr);
	LOG_MSG2( "   elfHdr->e_phentsize = %d\n", sizeof(Elf32_Phdr) );
	elfHdr->e_phentsize = sizeof(Elf32_Phdr);
	elfHdr->e_phnum = noProgramTblEntries;
	LOG_MSG2( "   Program header entries elfHdr->e_phnum = noProgramTblEntries=%d\n", noProgramTblEntries );
	elfHdr->e_shentsize = 0;
	elfHdr->e_shnum = 0;
	elfHdr->e_shstrndx = 0;

	//LOG_MSG( "   Writing ELF header at offset 0\n" );
	iDataSave->WriteL( 0, elfHdr, sizeof(Elf32_Ehdr));

    CleanupStack::PopAndDestroy(&registerInfoOffsets);

    CleanupStack::PopAndDestroy(&codeSegOffsets);
    CleanupStack::PopAndDestroy(&dataSegOffsets);
    CleanupStack::PopAndDestroy(&dataSegs);

    CleanupStack::PopAndDestroy();//&codeSegCleanup);
    CleanupStack::PopAndDestroy();//&threadCleanup);
    CleanupStack::PopAndDestroy();//&processCleanup);

	CleanupStack::PopAndDestroy(elfHdr);
    CleanupStack::PopAndDestroy(iStrInfoTbl);
    CleanupStack::PopAndDestroy(&armExcRegs);

	LOG_MSG("  iDataSave->CloseL();\n");
	iDataSave->CloseL();
}

/**
Checks the value of the crash cancel property. If set to cancel, it closes the
data save and then leaves with KErrAbort.
Otherwise it updates the crash progress with the argument value.

@param aProgress Progress description.
@leave KErrAbort if crash cancel property has been set
*/
void CSymbianElfFormatter::UpdateCrashProgressL( const TDesC &aProgress )
    {

    LOG_MSG("CSymbianElfFormatter::UpdateCrashProgress()\n");
    TInt cancelCrash;
    _LIT(KNoProgress, "");
    RProperty::Get( KCoreDumpServUid, ECancelCrash, cancelCrash );

    if( cancelCrash )
        {
        RProperty::Set( KCoreDumpServUid, ECrashProgress, KNoProgress );

		LOG_MSG("  iDataSave->CloseL();\n");
		iDataSave->CloseL();

		LOG_MSG("  UpdateCrashProgress() -> User::Leave( KErrAbort )\n");
        User::Leave( KErrAbort );
        }

    RProperty::Set( KCoreDumpServUid, ECrashProgress, aProgress );

    }
