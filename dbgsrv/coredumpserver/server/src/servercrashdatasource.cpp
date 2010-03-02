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
#include <e32btrace.h> 
#include <d32btrace.h>

#include "servercrashdatasource.h"

// Required for interface to Debug Security Server via RSecuritySvrSession
#include "coredumpsession.h"

CServerCrashDataSource::CServerCrashDataSource(RSecuritySvrSession &aSource)
    : iSecSess(aSource)
{
//no implementation
}

CServerCrashDataSource* CServerCrashDataSource::NewL(RSecuritySvrSession &aSource)
{
    CServerCrashDataSource *self = new(ELeave) CServerCrashDataSource(aSource);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
}


TTag* CServerCrashDataSource::GetTag(const TTagHeader* aTagHdr, const TInt aElement)
	{
	TUint8* ptr = (TUint8*)aTagHdr + sizeof(TTagHeader);
	TUint8* blockEnd = ptr + (aTagHdr->iNumTags * sizeof(TTag));

	while(ptr < blockEnd)
		{
		TTag* tag = (TTag*)ptr;
		if(tag->iTagId == aElement)
			{
			return tag;
			}
		ptr += sizeof(TTag);
		}
	return NULL;
	}

// second-phase constructor
void CServerCrashDataSource::ConstructL()
    {
	LOG_MSG("->CServerCrashDataSource::ConstructL()\n" );
	// Get the debug func block and make appropriate changes to our structures
    TInt err;
	TUint32 bufsize = 0;
	LOG_MSG( "  -> iSecSess->GetDebugFunctionalityBufSize( &bufsize )\n" );
    err = iSecSess.GetDebugFunctionalityBufSize( &bufsize );

	if( (err != KErrNone) || (0 == bufsize) )
		{
		// No debug functionality block, cannot do much without that.
		LOG_MSG2("CServerCrashDataSource::ConstructL() - unable to get debug functionality block! err:%d\n", err);
		User::Leave( KErrNotSupported );
		}

    RBuf8 DFBlock;
    DFBlock.CreateL(bufsize);
    DFBlock.CleanupClosePushL();

	LOG_MSG2( "  -> HBufC8::NewLC( bufsize=%d )\n", bufsize );


	LOG_MSG("CServerCrashDataSource::ConstrucL -> GetDebugFunctionality()\n" );
    err = iSecSess.GetDebugFunctionality(DFBlock);
	if( KErrNone != err )
		{
		LOG_MSG( "CServerCrashDataSource::ConstructL() : ERROR !* : Could not retrieve debug functionality block\n" );
		User::Leave( KErrNotSupported );
		}

	LOG_MSG( "  -> GetDebugHeader( EDF_TagHdrId_RegistersCore )\n" );
	TTagHeader * hdr = GetDebugHeader( ETagHeaderIdRegistersCore, DFBlock );

	if( hdr == NULL )
		{
		LOG_MSG( "Could not retrieve ETagHeaderIdRegistersCore register debug block\n" );
		}
	else if( 0 == hdr->iNumTags )
		{
		LOG_MSG( "Zero tags found for ETagHeaderIdRegistersCore register debug block\n" );
		}
	else
		{

		iRegisterList.ReserveL( (TInt)hdr->iNumTags );

		// Skip the header to get to the tags
		TUint8 * ptr = ((TUint8 *) hdr) + sizeof( TTagHeader );
		TTag * tag = (TTag *) ptr;

		TRegisterData regData;
		TRegisterData * reg = &regData;

		// Process all the register tags
		for( TInt regIdx = 0; regIdx < hdr->iNumTags; regIdx ++ )
			{
			reg->iRegClass  = 0;			// Core = 0
			reg->iId        = tag->iTagId;	//
			reg->iSubId     = 0;			//
			reg->iSize      = 2;			// Should all be 32 bits == 2.
			reg->iAvailable = ETrue;
			reg->iValue64	= 0;
			iRegisterList.Append( *reg );
			tag++;
			}
		}

	hdr = GetDebugHeader( ETagHeaderIdMemory, DFBlock );
	if( hdr == NULL )
		{
		LOG_MSG( "Could not retrieve ETagHeaderIdMemory. Cannot read memory\n" );
		iMaxMemReadSize = 0;
		}
	else if( 0 == hdr->iNumTags )
		{
		LOG_MSG( "Zero tags found for ETagHeaderIdMemory register debug block\n" );
		iMaxMemReadSize = 0;
		}
	else
		{
		TTag* tag = GetTag( hdr, EMemoryMaxBlockSize );
		if( tag )
			{
			//LOG_MSG2( " EMemoryMaxBlockSize =0x%X\n", tag->iValue );
			iMaxMemReadSize = tag->iValue;
			}
		}

	iLastThreadListSize = 1;
	iLastProcListSize = 1;
	iLastRegListSize = 1;

	CleanupStack::PopAndDestroy(&DFBlock);
    }

//
// Returns the start of the sub block for this TTagHdrId_e 
//
TTagHeader * CServerCrashDataSource::GetDebugHeader( const TTagHeaderId aTagHdrId, const TDesC8 &aDFBlock )
	{
	TInt DFBlockSize = aDFBlock.Size();

	TTagHeader* headerPtr = (TTagHeader*) aDFBlock.Ptr();
	TUint8 * ptr = (TUint8 * ) aDFBlock.Ptr();

	while( headerPtr < ( (TTagHeader*)aDFBlock.Ptr() + DFBlockSize ) )
		{

		if( headerPtr->iTagHdrId != aTagHdrId )
			{
			ptr += sizeof(TTagHeader) + ( headerPtr->iNumTags * sizeof(TTag) );
			headerPtr = (TTagHeader*) ptr;
			}
		else
			{
			return ( headerPtr );
			}
		}

	return NULL;
	}

// destructor
CServerCrashDataSource::~CServerCrashDataSource()
    {
	LOG_MSG( "server_crashdatasource.cpp::CServerCrashDataSource::~CServerCrashDataSource()\n" );
	iRegisterList.Reset();
    iThreadListBuffer.Close();
    iProcListBuffer.Close();
    iExecutableListBuffer.Close();
    }

void CServerCrashDataSource::SetRegValuesL( const TUint64 aThreadId, RRegisterList &aRegisterList )
	{

	LOG_MSG2("->CServerCrashDataSource::SetRegValuesL(aThreadId=%Lu)\n", aThreadId); 

	TInt numberOfRegisters = aRegisterList.Count();

	RBuf8 ids;
	ids.CreateL( numberOfRegisters * sizeof(TFunctionalityRegister) );
    ids.CleanupClosePushL();

	TInt totalByteSize = 0; // Keeps track of the number of bytes that we are requesting

	for( TInt i=0; i < numberOfRegisters; i++ )
		{
		TRegisterInfo reg = (TRegisterInfo)( aRegisterList[i].iId ); 

		// iSize = (0 == 1 byte); (3 == 8 bytes)
		TInt byteSize = (aRegisterList[i].iSize) << 1;
		totalByteSize += byteSize;
		ids.Append( reinterpret_cast<const TUint8*>(&reg), sizeof(TRegisterInfo) );
		}

	
	RBuf8 registerValues;
	registerValues.CreateL( totalByteSize );
    registerValues.CleanupClosePushL();

	RBuf8 registerFlags;
	registerFlags.CreateL( numberOfRegisters );
    registerFlags.CleanupClosePushL();

    LOG_MSG("CServerCrashDataSource::SetRegValuesL - reading registers\n");
    User::LeaveIfError(iSecSess.ReadRegisters( aThreadId, ids, registerValues, registerFlags ));

	// Now copy the values back to the array and mark the availability from the flags

	TUint8* valuePtr = (TUint8*) registerValues.Ptr();

	for( TInt i=0; i < numberOfRegisters; i++ )
		{

		TRegisterData & reg = aRegisterList[i];

		switch( reg.iSize )
			{
			case 0:
				reg.iValue8 = *((TUint8 *)valuePtr);
				valuePtr += 1;
				break;
			case 1:
				reg.iValue16 = *((TUint16 *)valuePtr);
				valuePtr += 2;
				break;
			case 2:
				reg.iValue32 = *((TUint32 *)valuePtr);
				valuePtr += 4;
				break;
			case 3:
				reg.iValue64 = *((TUint64 *)valuePtr);
				valuePtr += 8;
				break;
			}

		if( EValid == registerFlags[i] ) 
			{
			reg.iAvailable = ETrue;
			}
		else
			{
			reg.iAvailable = EFalse;
			}
		}

    CleanupStack::PopAndDestroy(&registerFlags);
    CleanupStack::PopAndDestroy(&registerValues);
    CleanupStack::PopAndDestroy(&ids);
	}


void CServerCrashDataSource::PrintRegs( RRegisterList & aRegisterList )
	{

	LOG_MSG( "CServerCrashDataSource::PrintRegs()\n" );

	if( aRegisterList.Count() > 0 )
		{


		for( TInt i = 0; i < aRegisterList.Count(); i ++ )
			{
			
			TRegisterData & reg = aRegisterList[i];

			RDebug::Printf( " reg[%d] iRegClass=%d, iId=%d, iSubId=%d, iSize=%d, iAvailable=%d\n", 
				i, reg.iRegClass, reg.iId, reg.iSubId, reg.iSize, reg.iAvailable );

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



void CServerCrashDataSource::GetRegisterListL( RRegisterList & aRegisterList )
	{

	LOG_MSG("CServerCrashDataSource::GetRegisterListL()\n");
	/* Copy the registers that were returned in the debug functionality block */

	aRegisterList.Reset();
    TInt count = iRegisterList.Count();
	for(TInt i = 0; i < count; ++i)
		{
		aRegisterList.AppendL( iRegisterList[i] );
		}
	}

void CServerCrashDataSource::ReadRegistersL( const TUint64 aThreadId, RRegisterList  & aRegisterList )
	{

	LOG_MSG2("CServerCrashDataSource::ReadRegistersL(aThreadId=%Lu)\n", aThreadId);

	if( aRegisterList.Count() )
		{
		LOG_MSG("CServerCrashDataSource::ReadRegistersL - register list already supplied\n" );
		SetRegValuesL( aThreadId, aRegisterList );
		}
	else
		{
		LOG_MSG("CServerCrashDataSource::ReadRegistersL - suppliying register list\n" );
		GetRegisterListL( aRegisterList );
		SetRegValuesL( aThreadId, aRegisterList );
		}

	//PrintRegs( aRegisterList );
	}


/**
Purpose:
Read memory from the security server. If the length of the data is greater
than the maximum length supported by the security server (as reported debug
functionality block tag EMemoryMaxBlockSize), break up the read into reads 
of this maximum size.

@pre Must be connected to the debug security server
@pre Must be attached to the process that owns aThreadId

@param aThreadId - The id of the thread relative to which the read should take place
@param aAddress - The virtual address to read from
@param aLength - The number of bytes to read
@param aData - The buffer to read into
@return Any error which may be returned by Security Server::ReadMemory()
*/
void CServerCrashDataSource::ReadMemoryL(
                   const TUint64    aThreadId,
                   const TUint32    aAddress,
                   const TUint32    aLength,
                   TDes8          & aData )
	{
    LOG_MSG4("->CServerCrashDataSource::ReadMemoryL(aThreadId=0x%X, aAddress=0x%X, aLength=%d)\n",
            I64LOW(aThreadId), aAddress, aLength);	
    TInt err;

	if( aLength <= iMaxMemReadSize )
		{
		err = iSecSess.ReadMemory( aThreadId, aAddress, aLength, aData, EAccess32, EEndLE8);
		}
    else
        {
        // If aLength is greater than iMaxMemReadSize, then break up the read into 
        // smaller packets and append them to aData

        RBuf8 readData;
        readData.CreateL( iMaxMemReadSize );

        TInt readLength = iMaxMemReadSize;
        TInt readSoFar = 0;
        do
            {
            LOG_MSG3( "  iSecSess->ReadMemory(addr=0x%X, length=%d)\n", aAddress+readSoFar, readLength );
            err = iSecSess.ReadMemory( aThreadId, aAddress+readSoFar, readLength, readData, EAccess32, EEndLE8 );
            if(err != KErrNone)
                break;

            readSoFar += readLength;
            aData.Append( readData );
            if( (aLength - readSoFar) < iMaxMemReadSize )
                {
                readLength = aLength - readSoFar;
                }
            }
        while( readSoFar < aLength );
        readData.Close();
        }
    User::LeaveIfError(err);
	}


void CServerCrashDataSource::GetProcessListL( RProcessPointerList & aProcessList,
											  TUint & aTotalProcessListDescSize )
    {
	//LOG_MSG( "CServerCrashDataSource::GetProcessListL()\n" );

	// Delete any objects in the array, since we will replace them. 
	aProcessList.ResetAndDestroy();
	aTotalProcessListDescSize = 0;

	if( iProcListBuffer.Size() != iLastProcListSize )
		{
		LOG_MSG2( "  iProcListBuffer.ReAlloc( %d)\n", iLastProcListSize );
		iProcListBuffer.ReAllocL( iLastProcListSize );
		}

	DoGetListL( EProcesses, (TUint)(-1), (TUint)(-1), iProcListBuffer, iLastProcListSize );

	iLastProcListSize = iProcListBuffer.Size();

	TUint8* ptr = (TUint8*)( iProcListBuffer.Ptr() );
	const TUint8* ptrEnd = ptr + iLastProcListSize;
	//LOG_MSG3( "  Data start,end=(0x%x, 0x%x)\n", ptr, ptrEnd );

	while( ptr < ptrEnd ) 
		{

	    TProcessListEntry *entry = (TProcessListEntry*)ptr;

		if( !entry )
			{
			LOG_MSG( "  ERROR !* : TProcessListEntry is NULL\n" );
			User::Leave(KErrBadHandle);
			}

		TUint64 id = entry->iProcessId;
		if( entry->iFileNameLength == 0 )
			{
			LOG_MSG4( "  Skipping process 0x%X%X : entry->iFileNameLength=%d", 
				I64HIGH(entry->iProcessId), I64LOW(entry->iProcessId), entry->iFileNameLength );
			ptr += Align4( entry->GetSize() );
			continue;
			}

		/*
		LOG_MSG4( "  process 0x%X%X has iFileNameLength=%d", 
			I64HIGH(entry->iProcessId), I64LOW(entry->iProcessId), entry->iFileNameLength );
		*/

        TPtrC entryName(&(entry->iNames[0]), entry->iFileNameLength);
       
        CProcessInfo *procInfoPtr = CProcessInfo::NewL(id, entryName );

		TInt err = aProcessList.Append( procInfoPtr );
        if(err != KErrNone)
            {
            delete procInfoPtr;
            User::Leave(err);
            }

		aTotalProcessListDescSize += procInfoPtr->Size();

		ptr += Align4( entry->GetSize() );
		}
	}


void CServerCrashDataSource::GetExecutableListL( RExecutablePointerList & aExecutableList,
											  TUint & aTotalExecutableListDescSize )
    {
	LOG_MSG( "CServerCrashDataSource::GetExecutableListL()\n" );

	// Delete any objects in the array, since we will replace them. 
	aExecutableList.ResetAndDestroy();
	aTotalExecutableListDescSize = 0;

	if( iExecutableListBuffer.Size() != iLastExecutableListSize )
		{
		LOG_MSG2( "  iExecutableListBuffer.ReAlloc( %d)\n", iLastExecutableListSize );
		iExecutableListBuffer.ReAllocL( iLastExecutableListSize );
		}

	DoGetListL( EExecutables, (TUint)(-1), (TUint)(-1), iExecutableListBuffer, iLastExecutableListSize );

	iLastExecutableListSize = iExecutableListBuffer.Size();

	TUint8* ptr = (TUint8*)( iExecutableListBuffer.Ptr() );
	const TUint8* ptrEnd = ptr + iLastExecutableListSize;
	//LOG_MSG3( "  Data start,end=(0x%x, 0x%x)\n", ptr, ptrEnd );

	while( ptr < ptrEnd ) 
		{

	    TExecutablesListEntry *entry = (TExecutablesListEntry*)ptr;

		if( !entry )
			{
			LOG_MSG( "  ERROR !* : TExecutablesListEntry is NULL\n" );
			User::Leave(KErrBadHandle);
			}

		if( entry->iNameLength == 0 )
			{
			LOG_MSG( "  Skipping executable : entry->iNameLength=0" );
			ptr += Align4( entry->GetSize() );
			continue;
			}

        TPtrC entryName(&(entry->iName[0]), entry->iNameLength);
       
        CExecutableInfo *executableInfoPtr = 
			CExecutableInfo::NewL(entryName, entry->iIsActivelyDebugged, entry->iIsPassivelyDebugged );

		TInt err = aExecutableList.Append( executableInfoPtr );
        if(err != KErrNone)
            {
            delete executableInfoPtr;
            User::Leave(err);
            }

		aTotalExecutableListDescSize += executableInfoPtr->Size();

		ptr += Align4( entry->GetSize() );
		}
	}


void CServerCrashDataSource::DoGetListL(const TListId aListId, 
										const TThreadId aThreadId,
										const TProcessId aProcessId,
										RBuf8 & aBuffer, 
										TUint32 & aSize )
	{

	LOG_MSG5( "->CServerCrashDataSource::DoGetList(aListId=%d, aThredId=%Lu, aProcessId=%Lu, aSize=%d\n", aListId, aThreadId.Id(), aProcessId.Id(), aSize );

	TInt ret;
	TListLevel listLevel;

	if( ((TUint)-1 == (TUint)aThreadId) && ((TUint)-1 == (TUint)aProcessId) )
		{
		listLevel = EListGlobal;
		ret = iSecSess.GetList( aListId, aBuffer, aSize );
		}
	else if( ((TUint)-1 != (TUint)aThreadId) && ((TUint)-1 == (TUint)aProcessId) )
		{
		listLevel = EListThread;
        LOG_MSG("CServerCrashDataSource::DoGetList - EListThread");
		ret = iSecSess.GetList( aThreadId, aListId, aBuffer, aSize );
		}
	else
		{
		listLevel = EListProcess;
        LOG_MSG("CServerCrashDataSource::DoGetList - EListProcess");
		ret = iSecSess.GetList( aProcessId, aListId, aBuffer, aSize );
		}
		 
	while( KErrTooBig == ret )
		{
		LOG_MSG2( "CServerCrashDataSource::DoGetListL - list too big, new size=%d\n", aSize );

		// Too big, and aSize should have been modified to the required new size
		// Since the list could have increased in size between calls, give it an 
		// extra margin.
		aSize += 256;

		aBuffer.ReAllocL( aSize );

		if( EListGlobal == listLevel )
			{
			ret = iSecSess.GetList( aListId, aBuffer, aSize );
			}
		else if( EListThread == listLevel )
			{
			ret = iSecSess.GetList( aThreadId, aListId, aBuffer, aSize );
			}
		else
			{
			ret = iSecSess.GetList( aProcessId, aListId, aBuffer, aSize );
			}

		}
    User::LeaveIfError(ret);
	}

/** Obtain the thread list. If aProcessId is negative, we obtain the entire system
thread list. If aProcessId is positive we get the thread list for that process */
void CServerCrashDataSource::GetThreadListL( const TUint64 aProcessId, 
											 RThreadPointerList & aThreadList,
											 TUint & aTotalThreadListDescSize )
	{
	LOG_MSG2( "->CServerCrashDataSource::GetThreadListL(aProcessId=%Lu)\n", aProcessId); 

	// Delete any objects in the array, since we will replace them. 
	aThreadList.ResetAndDestroy();
		
	aTotalThreadListDescSize = 0;

	if( iThreadListBuffer.Size() != iLastThreadListSize )
		{
		LOG_MSG2("CServerCrashDataSource::GetThreadListL -> iThreadListBuffer.ReAlloc(%d)\n", iLastThreadListSize );
		iThreadListBuffer.ReAllocL( iLastThreadListSize );
		}

	LOG_MSG( "CServerCrashDataSource::GetThreadListL -> DoGetListL()\n" );
	DoGetListL( EThreads, (TUint)-1, aProcessId, iThreadListBuffer, iLastThreadListSize );

	iLastThreadListSize = iThreadListBuffer.Size();
	

	CThreadInfo		 *  threadInfoPtr;
	RThread				thread;
	TThreadStackInfo	stackInfo;
	TThreadListEntry *	entry;
	TUint				usrStackSize;
	TLinAddr			usrStackAddr;
	TLinAddr			svcStackPtr;
	TLinAddr			svcStackBase;
	TUint				svcStackSize;

	TUint				priority;

	TUint8* ptr = (TUint8*)( iThreadListBuffer.Ptr() );
	const TUint8* ptrEnd = ptr + iLastThreadListSize;
	//LOG_MSG3( "  Data start,end=(0x%x, 0x%x)\n", ptr, ptrEnd );

	while( ptr < ptrEnd ) 
		{

		entry = (TThreadListEntry*)ptr;

		if( !entry )
			{
			LOG_MSG( "  ERROR !* : TThreadListEntry is NULL\n" );
			User::Leave(KErrBadHandle);
			}

		if( entry->iNameLength == 0 )
			{
			LOG_MSG4( "  Skipping Thread 0x%X%X : entry->iNameLength=%d", 
				I64HIGH(entry->iThreadId), I64LOW(entry->iThreadId), entry->iNameLength );
			ptr += Align4( entry->GetSize() );
			continue;
			}

		//LOG_MSG3( "  entry &=0x%X, size=%d\n", &(entry->iThreadId), entry->GetSize() );
		//LOG_MSG3( "  entry->iThreadId= 0x%X%X\n", I64HIGH(entry->iThreadId), I64LOW(entry->iThreadId) );
		//LOG_MSG3( "  found tid=%d, pid=%d\n", I64LOW(entry->iThreadId), I64LOW(entry->iProcessId) );
			
		if( entry->iSupervisorStackPtrValid )
			{
			svcStackPtr = entry->iSupervisorStackPtr;
			}
		else
			{
			svcStackPtr = 0;
			}

		if( entry->iSupervisorStackBaseValid )
			{
			svcStackBase = entry->iSupervisorStackBase;
			}
		else
			{
			svcStackBase = 0;
			}
		
		if( entry->iSupervisorStackSizeValid )
			{
			svcStackSize = entry->iSupervisorStackSize;
			}
		else
			{
			svcStackSize = 0;
			}		



		if( KErrNone == thread.Open( entry->iThreadId ) )
			{
			priority = (TUint)(thread.Priority());

			if( KErrNone == thread.StackInfo( stackInfo ) )
				{
				usrStackAddr = stackInfo.iLimit;
				usrStackSize = stackInfo.iBase - stackInfo.iLimit;
				}
			else
				{
				usrStackSize = 0;
				usrStackAddr = 0;
				}

			thread.Close();
			}
		else
			{
			usrStackSize = 0;
			usrStackAddr = 0;
			priority = 0;
			}

		/*
		LOG_MSG3( "  entry->iNameLength=%d, &(entry->iName[0])=0x%X\n", 
				entry->iNameLength, &(entry->iName[0]) );
		*/

		TPtrC entryName( &(entry->iName[0]), entry->iNameLength );
		//LOG_MSG2( "  -> threadInfoPtr = CThreadInfo::NewL( name.Size()=%d)\n", entryName.Size() );

		threadInfoPtr = CThreadInfo::NewL( 
										entry->iThreadId, 
										entryName,
										entry->iProcessId,
										(TUint)priority,
										svcStackPtr,
										svcStackBase,
										svcStackSize,
										usrStackAddr,
										usrStackSize );

		/*
		LOG_MSG3( "  threadInfoPtr->iSvcStackAddr=0x%X, threadInfoPtr->iSvcStackSize=0x%X\n", 
			threadInfoPtr->SvcStackAddr(), threadInfoPtr->SvcStackSize() );
		*/

		TInt err = aThreadList.Append( threadInfoPtr );
		if( err != KErrNone )
			{
			// We use this id so as not to use Push(), AppendL(), Pop()
			delete threadInfoPtr;
			User::Leave( err );
			}

		aTotalThreadListDescSize += threadInfoPtr->Size();

		/*
		LOG_MSG3( "  aTotalThreadListDescSize = %d after adding %d\n", 
				aTotalThreadListDescSize, threadInfoPtr->Size() );

		RBuf rPrintBuf;
		rPrintBuf.Create( threadInfoPtr->Name()); 
		RDebug::Printf( "  <- rPrintBuf.Create(), rPrintBuf.Length()=%d\n", rPrintBuf.Length() );
		char* cl = (char*) rPrintBuf.Collapse().PtrZ();
		RDebug::Printf("  name=%s\n", cl );
		rPrintBuf.Close();
		*/

		ptr += Align4( entry->GetSize() );
		//LOG_MSG2( "  ptr += Align4(entry->GetSize()) = 0x%X\n", ptr );

		} // while

	}

void CServerCrashDataSource::GetCodeSegmentsL( const TUint64 aTid, RCodeSegPointerList &aCodeSegList, TUint & aTotalCodeSegListDescSize )
	{
	LOG_MSG2("->CServerCrashDataSource::GetCodeSegmentsL(aTid=%Lu)\n", aTid );

    aCodeSegList.ResetAndDestroy();
    aTotalCodeSegListDescSize = 0;

	TUint32 size = KMaxFileName;
	RBuf8 buffer;
    buffer.CreateL(KMaxFileName);
    CleanupClosePushL(buffer);

	DoGetListL( ECodeSegs, aTid, (TUint)-1, buffer, size );
	LOG_MSG2( " DoGetListL( ECodeSegs ) returned buffer.Size()=0x%X\n", buffer.Size() );

	TUint8* ptr = (TUint8*)buffer.Ptr();
	const TUint8* ptrEnd = ptr + size;

	while(ptr < ptrEnd)
		{
		TCodeSegListEntry* entry = (TCodeSegListEntry*)ptr;

		LOG_MSG4( "  entry->CodeBase=0x%X, CodeSize=0x%X, ConstDataSize=0x%X\n", 
			entry->iCodeBase, entry->iCodeSize, entry->iConstDataSize );
		LOG_MSG4( "  InitDataBase=0x%X, InitDataSize=0x%X, UnintDataSize=0x%X\n", 
			entry->iInitialisedDataBase, entry->iInitialisedDataSize, entry->iUninitialisedDataSize );
		LOG_MSG3( "  IsXip=0x%X, CodeSegType=0x%X\n", entry->iIsXip, entry->iCodeSegType );
		
        TCodeSegInfo *codeSeg = new(ELeave) TCodeSegInfo;

		TPtr name(&(entry->iName[0]), entry->iNameLength, entry->iNameLength);
        codeSeg->iName = name;	

        codeSeg->iType = entry->iCodeSegType;
        codeSeg->iXIP = entry->iIsXip;

        codeSeg->iCodeSize = entry->iCodeSize;
        codeSeg->iCodeRunAddr = entry->iCodeBase;
        if( codeSeg->iXIP )
            {
            codeSeg->iCodeLoadAddr = codeSeg->iCodeRunAddr;
            }
        else
            {
            codeSeg->iCodeLoadAddr = 0; //TODO
            }

        codeSeg->iRoDataSize = entry->iConstDataSize;
        codeSeg->iRoDataRunAddr = entry->iCodeBase + entry->iCodeSize;
        if( codeSeg->iXIP )
            {
            codeSeg->iRoDataLoadAddr = codeSeg->iRoDataRunAddr;
            }
        else
            {
            codeSeg->iRoDataLoadAddr = 0; //TODO
            }

        codeSeg->iDataSize = entry->iInitialisedDataSize + entry->iUninitialisedDataSize;
        codeSeg->iDataRunAddr = entry->iInitialisedDataBase;
        if( codeSeg->iXIP )
            {
            codeSeg->iDataLoadAddr = codeSeg->iDataRunAddr;
            }
        else
            {
            codeSeg->iDataLoadAddr = 0; //TODO
            }

        TInt err = aCodeSegList.Append(codeSeg);
        if(err != KErrNone)
            {
            delete codeSeg;
            User::Leave(err);
            }

		aTotalCodeSegListDescSize += sizeof(TCodeSegInfo);
        ptr += Align4(entry->GetSize());
		}

    CleanupStack::PopAndDestroy(&buffer);
	}

/**
 * Returns the size of the trace data available
 * @return Trace Size
 * @leave One of the system wide codes
 */
TUint CServerCrashDataSource::GetAvailableTraceSizeL()
	{
	LOG_MSG("->CServerCrashDataSource::GetAvailableTraceSizeL()");
    
    RBTrace trace;
    User::LeaveIfError(trace.Open());
    
    TUint8 *data = NULL;
    TInt size = trace.GetData(data);

    trace.Close();  
    
    return size;
	}

/**
 Reads data from the trace buffer into the supplied descriptor. Not supported. Recommended 
 approach is to use the live trace framework.
 @param aTraceData The buffer to be read into
 @param aPos Ignored
*/
void CServerCrashDataSource::ReadTraceBufferL(TDes8 &aTraceData, TUint aPos)
	{
    LOG_MSG("->CServerCrashDataSource::ReadTraceBuffer() -- Not supported: Please use the live trace system");
    
    User::Leave(KErrNotSupported);
	}

/**
 * Gets locks data
 * @param aLockData stores lock data
 * @leave one of the OS wide codes
 */
void CServerCrashDataSource::GetLocksL(TSCMLockData& aLockData)
	{
	User::Leave(KErrNotSupported);
	}

/**
 * Gets the ROM build info if available
 * @param aRomHeader
 * @leave one of the OS wide codes
 */
void CServerCrashDataSource::GetROMBuildInfoL(TRomHeaderData& aRomHeader)
	{
	User::Leave(KErrNotSupported);
	}

/**
 * @see CCrashDataSource::GetExceptionStackSizeL
 */
TUint CServerCrashDataSource::GetExceptionStackSizeL(const Debug::TArmProcessorModes aMode)
	{
	User::Leave(KErrNotSupported);
	return 0; //comp warnings
	}

/**
 * @see CCrashDataSource::GetExceptionStackL
 */
void CServerCrashDataSource::GetExceptionStackL(const Debug::TArmProcessorModes aMode, TDes8& aStack, TUint aStartReadPoint)
	{
	User::Leave(KErrNotSupported);
	}

/**
 * @see CCrashDataSource::GetVersion
 */
TVersion CServerCrashDataSource::GetVersion() const
	{
	return KServerCrashDataSourceVersion;
	}

/**
 * @see CCrashDataSource::GetDataSourceFunctionalityBufSize
 */
TInt CServerCrashDataSource::GetDataSourceFunctionalityBufSize(TUint& aBufSize)
	{
	return KErrNotSupported;
	}

/**
 * @see CCrashDataSource::GetDataSourceFunctionality
 */
TInt CServerCrashDataSource::GetDataSourceFunctionality(TDes8& aFuncBuffer)
	{
	return KErrNotSupported;
	}
	
TInt CServerCrashDataSource::GetVariantSpecificDataSize(TUint& aDataSize) 
	{
	(void)aDataSize;
	return KErrNotSupported;
	}

TInt CServerCrashDataSource::GetVariantSpecificData(TDes8 &aVarSpecData)
	{
	(void)aVarSpecData;
	return KErrNotSupported;
	}

//eof
