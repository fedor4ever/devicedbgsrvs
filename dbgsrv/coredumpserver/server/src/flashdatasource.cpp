// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <d32btrace.h>
#include <crashlogwalker.h>
#include <optionconfig.h>
#include "flashdatasource.h"

// Required for interface to Debug Security Server via RSecuritySvrSession
#include "coredumpsession.h"

using namespace Debug;

CFlashDataSource::CFlashDataSource(RSecuritySvrSession &aSource) :
	iSecSess(aSource),
	iLockData(NULL),
	iRomBuildInfo(NULL),
	iTraceDump(NULL),
	iVarSpecData(NULL),
    iWalker(iFlashBuf),
	iScmConfig(NULL),
	iCrashFound(EFalse)
	{
	//no implementation
	}

/**
 * This looks at the crash logs header and keeps note of where all the different types of data reside
 * @return one of the OS wide system codes
 */
TInt CFlashDataSource::ProcessCrashHeaderL(TInt aCrashStart)
	{
	LOG_MSG("CFlashDataSource::ProcessCrashHeader()");

	//record where the crash started
	TInt iCrashStart = aCrashStart;

	User::LeaveIfError(ReadCrashLog(iCrashStart, Debug::KMaxCoreHeaderSize));
	User::LeaveIfError(iWalker.ReadLogHeader(0));

	iHdr = iWalker.GetCrashHeader();
	iOffsetsHdr = iWalker.GetOffsetsHeader();

	if(iOffsetsHdr.iId == ESCMOffsetsHeader)
		{
		iOffsetsPresent = ETrue;
		}

	return KErrNone;
	}

/**
 * attempts to read the scm config block from start of flash to see if its there
 * @return The configuration block if found. Check for NULL and ownership is passed to caller
 */
SCMConfiguration* CFlashDataSource::GetSCMConfigFromFlashL()
	{
	LOG_MSG("CFlashDataSource::GetSCMConfigFromFlashL()");

	//Always at start of flash
	TInt scmLocation = 0;
	Debug::SCMConfiguration* config = new Debug::SCMConfiguration();
	CleanupStack::PushL(config);

	User::LeaveIfError(ReadCrashLog(scmLocation, SCMConfiguration::KScmConfigMaxSize));
	CleanupStack::Pop();	
	//Put a reader on the buffer
	TByteStreamReader rdr(const_cast<TUint8*>(iFlashBuf.Ptr()));
	if(config->Deserialize(rdr) == KErrNone)
		{

		LOG_MSG3("CFlashDataSource::GetSCMConfigFromFlashL()  Config found. It is [%d] [0x%X] bytes in size",
					config->GetSize(), config->GetSize());

		LOG_MSG("CFlashDataSource::GetSCMConfigFromFlashL() #2");
		return config;
		}
	else
		{
		LOG_MSG("CFlashDataSource::GetSCMConfigFromFlashL() No config found");
		delete config;
		return NULL;
		}
	}

/**
 * This looks at the entire log partition looking for where the crash of this ID
 * begins
 * @param aCrashId ID we are looking for
 * @return TInt where crash starts
 * @leave One of the OS wide codes
 */
TInt CFlashDataSource::GetCrashStartFromCrashIDL(TInt aCrashId)
	{
	LOG_MSG2("CFlashDataSource::GetCrashStartFromCrashIDL(TInt aCrashId)	 ID = %d", aCrashId);

	TInt crashStart = 0;
	TInt readStatus = KErrNone;

	//Start looking where the config ends if it exists
	Debug::SCMConfiguration* config = GetSCMConfigFromFlashL();
	if(config)
		{
		crashStart = config->GetSize();
		}

	delete config;

	while(readStatus == KErrNone)
		{
		User::LeaveIfError(ReadCrashLog(crashStart, Debug::KMaxCoreHeaderSize));

		readStatus = iWalker.ReadLogHeader(0); //Look at header from start of buffer

		if(readStatus != KErrNone)
			{
			CLTRACE("CFlashDataSource::GetCrashStartFromCrashIDL ReadCrashLogL leaving with KErrNotFound #1");
			User::Leave(KErrNotFound);
			}
		else if(iWalker.GetCrashHeader().iCrashId == aCrashId)
			{
			return crashStart;
			}

		const TCrashInfoHeader currentHdr = iWalker.GetCrashHeader();

		TUint32 flashPadding = currentHdr.iFlashBlockSize - (crashStart + currentHdr.iLogSize)%currentHdr.iFlashBlockSize;
		crashStart += (currentHdr.iLogSize + flashPadding);
		}

	CLTRACE("CFlashDataSource::GetCrashStartFromCrashIDL ReadCrashLogL leaving with KErrNotFound #2");
	User::Leave(KErrNotFound);

	return 0;  //avoid comp warnings
	}

/**
 * This walks the crash log verifying it is not corrupt and builds up a map of the log so that we can get the
 * data required faster later on when the appropriate call is made.
 * @param Crash ID to look at
 * @leave One of the system wide error codes
 */
void CFlashDataSource::AnalyseCrashL(TInt aCrashId)
	{
	LOG_MSG2("CFlashDataSource::AnalyseCrashL(TInt aCrashId = [%d])", aCrashId);

	//Reset the state
	iOffsetsPresent = EFalse;
	iCrashMemoryMap.Reset();
	iCodeSegMap.Reset();
	iHdrProcd = EFalse;

    if(iCurrentCodeSegSet)
    	{
		delete iCurrentCodeSegSet;
		iCurrentCodeSegSet = NULL;
    	}

    if(iLockData)
    	{
    	delete iLockData;
    	iLockData = NULL;
    	}

    if(iRomBuildInfo)
    	{
    	delete iRomBuildInfo;
    	iRomBuildInfo = NULL;
    	}

	TInt crashStart = GetCrashStartFromCrashIDL(aCrashId);
	ProcessCrashHeaderL(crashStart);

	TInt dataSizeRead = 0;
	TInt flashPos = crashStart;

	//either read the defined amt or the log size, whichever is less
	TInt rdSize = READ_SIZE;

	/**
	 *	Our conditions to keep reading  or decide log is corrupt are:
	 *	1. Have we read logSize bytes?
	 *			No - Keep Going
	 *			Yes - Finished
	 *			More - Corrupt
	 *  2. There is no more data of interest to be read. Is the number of bytes read = log size?
	 */

	TInt recommendedReadSize = 0;

	while(dataSizeRead != iHdr.iLogSize)
		{
		//We need to decide how much to read here. It is possible that we need to read greater than the log size
		//as the reader can't know in advance the exact size of a struct, so it has to assume the worst case scenario
		//of the largest size it could be
		rdSize = (rdSize < READ_SIZE) ? READ_SIZE : rdSize;	//resize this to be large

		TInt bytesRemaining = iHdr.iLogSize - dataSizeRead;
		if(rdSize > bytesRemaining)
			{
			rdSize = bytesRemaining;
			}

		//always be at least the rec read size
		rdSize = (rdSize < recommendedReadSize) ? recommendedReadSize : rdSize;

		User::LeaveIfError(ReadCrashLog(flashPos, rdSize));

		//Get what we can out of this buffer read
		SCMStructId id = ESCMFirst;
		TInt bufferPos = 0;
		MByteStreamSerializable* data = NULL;

		do
			{
			//Have we got to end of crash?
			if(dataSizeRead == iHdr.iLogSize)
				{
				break;
				}

			data = iWalker.GetNextDataTypeL(bufferPos, id, recommendedReadSize);

			//Analyse what we got
			if(data)
				{
				CleanupStack::PushL(data);
				dataSizeRead += data->GetSize();

				//check we are within range
				if(dataSizeRead > iHdr.iLogSize)
					{
					LOG_MSG4("CFlashDataSource::AnalyseCrashL: Corrupt -->    dataSizeRead [%d] > iHdr.iLogSize [%d] at line [%d]", dataSizeRead, iHdr.iLogSize, __LINE__);
					User::Leave(KErrCorrupt);
					}

				HelpAnalyseCrashL(data, id, flashPos + bufferPos - data->GetSize());
				CleanupStack::PopAndDestroy(data);
				}
			else
				{
				//if data was NULL, that means there wasnt enough room in the buffer to read the next one
				flashPos += bufferPos;
				}
			}
		while(data);
		}
	}

/**
 * Helper for crash analysis. Takes a pointer to a data type, an ID to tell us what type it is, and where
 * in the flash it has been found. If it is a type of interest, we record its position
 * @param aData Data Type of interest
 * @param aStructId ID to describe data type
 * @param aPosFound Where it was found in the flash
 * @leave
 */
void CFlashDataSource::HelpAnalyseCrashL(const MByteStreamSerializable* aData, TInt aStructId, TInt aPosFound)
	{

	switch(aStructId)
		{
		case ESCMMemory :
			{
			//we want to keep track of this objects
			TObjectLocation map;
			map.iOffset = aPosFound;
			map.iObjectId = static_cast<const TMemoryDump*>(aData)->iPid;

			iCrashMemoryMap.AppendL(map);

			break;
			}
		case ESCMCodeSegSet :
			{
			//The set contains the PID for the code segs to follow - copy to our cached version
			if(iCurrentCodeSegSet)
				{
				delete iCurrentCodeSegSet;
				iCurrentCodeSegSet = NULL;
				}

			iCurrentCodeSegSet = new TCodeSegmentSet();
			*iCurrentCodeSegSet = *(static_cast<const TCodeSegmentSet*>(aData));

			break;
			}
		case ESCMCodeSeg :
			{
			//ensure we have a code segment set for this
			__ASSERT_ALWAYS(iCurrentCodeSegSet, User::Leave(KErrCorrupt));

			//we want to keep track of these objects
			TObjectLocation map;
			map.iOffset = aPosFound;
			map.iObjectId = iCurrentCodeSegSet->iPid;

			iCodeSegMap.AppendL(map);

			break;
			}
		case ESCMRomHeader :
			{
			if(iRomBuildInfo)
				{
				delete iRomBuildInfo;
				iRomBuildInfo = NULL;
				}

			iRomBuildInfo = new TRomHeaderData();
			*iRomBuildInfo = *(static_cast<const TRomHeaderData*>(aData));

			break;
			}
		case ESCMLocks:
			{
			if(iLockData)
				{
				delete iLockData;
				iLockData = NULL;
				}

			iLockData = new TSCMLockData();
			*iLockData = *(static_cast<const TSCMLockData*>(aData));

			break;
			}
		case ESCMTCrashInfo :
			{
			iHeaderStart = aPosFound;

			break;
			}
		case ESCMTraceData :
			{
			if(iTraceDump)
				{
				delete iTraceDump;
				iTraceDump = NULL;
				}

			iTraceDump = new TTraceDump();
			*iTraceDump = *(static_cast<const TTraceDump*>(aData));

			break;
			}
		case ESCMVariantData:
			{
			if(iVarSpecData)
				{
				delete iVarSpecData;
				iVarSpecData = NULL;
				}

			iVarSpecData = new TVariantSpecificData();
			*iVarSpecData = *(static_cast<const TVariantSpecificData*>(aData));

			break;
			}
		case ESCMOffsetsHeader :
		case ESCMProcessData :
		case ESCMThreadData :
		case ESCMThreadStack :
		case ESCMRegisterValue :
		case ESCMRegisterSet :
		case ESCMRawData :
			{
			//No problem if we arent interested in the object, carry on, nothing to see here
			break;
			}
		default :
			{
			LOG_MSG2("CFlashDataSource::HelpAnalyseCrashL --> Dont recognise struct ID = [%d] therefore corrupt", aStructId);

			//if we dont recognise the object, then oddness is afoot and the mist of corruption reigns thick
			User::Leave(KErrCorrupt);
			}
		}

	}

/**
 * This Reads the portion of the crash log of interest into our buffer for processing later
 *
 * @param aPos Position of crash log to begin reading
 * @param aSize amount of data to read
 * @return KErrArgument if aSize exceeds the maximum read allowed or one of the other OS wide codes
 */
TInt CFlashDataSource::ReadCrashLog(const TInt aPos, const TUint aSize)
	{
	//ensure parameters are ok
	if(aPos < 0)
		{
		return KErrArgument;
		}

	TInt err = ReallocFlashBuf(aSize);
	if(err != KErrNone)
		{
		return err;
		}

	iWalker.UpdateBuffer(iFlashBuf);

	iFlashBuf.Zero();

	//Read the data from the flash buffer
	return iSecSess.ReadCrashLog(aPos, iFlashBuf, aSize);
	}

/**
 * Two Phase Constructor for CFlashDataSource
 * @param aSource Reference to Security Server session
 * @return Pointer to new CFlashDataSource object
 * @leave
 */
CFlashDataSource* CFlashDataSource::NewL(RSecuritySvrSession &aSource)
{
    CFlashDataSource *self = new(ELeave) CFlashDataSource(aSource);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
}

/**
 * second-phase constructor
 */
void CFlashDataSource::ConstructL()
    {
    //Create flash buffer. Will be resized as required
    iFlashBuf.CreateL(KInitialBufferSize);
    iFlashBuf.FillZ();

	iScmConfig = new (ELeave) Debug::SCMConfiguration();
	iScmConfig->SetDefaultConfig();

    }

/**
 * Destructor
 */
CFlashDataSource::~CFlashDataSource()
    {
	LOG_MSG( "server_crashdatasource.cpp::CFlashDataSource::~CFlashDataSource()\n" );
	iFlashBuf.Close();
	iFirstBlockBuf.Close();

	iCodeSegMap.Close();
	iCrashMemoryMap.Close();
	iThreadOwners.Close();

	if(iScmConfig)
		{
		delete iScmConfig;
		iScmConfig = NULL;
		}

	if(iCurrentCodeSegSet)
		{
		delete iCurrentCodeSegSet;
		iCurrentCodeSegSet = NULL;
		}

	if(iLockData)
		{
		delete iLockData;
		iLockData = NULL;
		}

	if(iRomBuildInfo)
		{
		delete iRomBuildInfo;
		iRomBuildInfo = NULL;
		}

	if(iTraceDump)
		{
		delete iTraceDump;
		iTraceDump = NULL;
		}

	if(iVarSpecData)
		{
		delete iVarSpecData;
		iVarSpecData = NULL;
		}

    }

void CFlashDataSource::GetRegisterListL( RRegisterList & aRegisterList )
	{
	//Not used
	User::Leave(KErrNotSupported);
	}

/**
 * Reads registers from flash for a given thread
 * @param aThreadId Thread of interest
 * @param aRegisterList Appends any registers found to this list
 * @leave OS wide code
 */
void CFlashDataSource::ReadRegistersL( const TUint64 aThreadId, RRegisterList  & aRegisterList )
	{
    LOG_MSG("CFlashDataSource::ReadRegistersL()");

	// Delete any objects in the array, since we will replace them.
	aRegisterList.Reset();

    /**
     * We keep 3 sets of registers in the flash at different points.
     * The first is the FullCPU register set for the thread that has crashed.
     * The second are the supervisor registers for a given thread (may not all be available)
     * The third are the user registers for a given thread (may not all be available)
     * These corrospond to 3 different configuration options
     */

	//See if it is the crashed threads full CPU set
	if(aThreadId == iHdr.iTid)
		{
	   	//Put regs into list
	   	HelpReadRegistersL(iOffsetsHdr.iCTFullRegOffset, iHdr.iLogSize, aThreadId, aRegisterList);
		return;
		}

	//Now we look at SVR registers
	HelpReadRegistersL(iOffsetsHdr.iSysSvrRegOffset, iOffsetsHdr.iSysUsrRegOffset, aThreadId, aRegisterList);

	//And finally the USR registers
	HelpReadRegistersL(iOffsetsHdr.iSysUsrRegOffset, iOffsetsHdr.iSysSvrRegOffset, aThreadId, aRegisterList);

	}

/**
 * This takes in a register list and looks at our flash buffer. It reads any registers it can out of the buffer
 * and appends them to the list.
 * @param aRegStartPoint Position in flash to begin searching from
 * @param aMaxReadPoint Position beyond which we shall not read
 * @param aThreadId Thread whose registers we want
 * @param aRegList List in which to store found registers
 * @leave KErrCorrupt if the buffer doesnt contain valid buffer data
 */
void CFlashDataSource::HelpReadRegistersL(TInt aRegStartPoint, TInt aMaxReadPoint, TUint64 aThreadId, RRegisterList& aRegList)
	{
	TRegisterValue* regVal;
	TBool stillMoreDataToBeHad = ETrue;
	TInt flashPos = aRegStartPoint;
	TInt rdSize = READ_SIZE;

	//cycle through the flash partition, rdSize bytes at a time until there is no more data of interest to be had
	while(stillMoreDataToBeHad)
		{
		User::LeaveIfError(ReadCrashLog(flashPos, rdSize));

		//Get what we can out of this buffer read
		TInt pos, bufferSize = 0;
		SCMStructId id = ESCMLast;
		MByteStreamSerializable* data = NULL;

		TRAPD(err, data = iWalker.GetNextDataTypeL(pos, id, bufferSize));
		if(err != KErrNone)
			{
			return; //We dont mind if this returns corrupt. This is only relevant during initial analysis
			}

		while(data && stillMoreDataToBeHad)
			{
			CleanupStack::PushL(data);

			switch(id)
				{
				case ESCMRegisterSet :
					{
					break;
					}
				case ESCMRegisterValue :
					{
					regVal = static_cast<TRegisterValue*>(data);

			    	//See if its for the right thread
			    	if(regVal->iOwnId != aThreadId)
			    		{
			    		break;
			    		}

			    	TRegisterData newData;
			    	ConvertFlashFormatRegL(newData, *regVal);
			    	aRegList.AppendL(newData);

					break;
					}

				//Dont care bout these
				case ESCMFirst :
				case ESCMOffsetsHeader :
				case ESCMTCrashInfo :
				case ESCMThreadData :
				case ESCMThreadStack :
				case ESCMMemory :
				case ESCMCodeSegSet :
				case ESCMCodeSeg :
				case ESCMRawData :
				case ESCMLast :
				default :
					{
					//No more registers mean no more interest
					stillMoreDataToBeHad = EFalse;
					}
				}

			CleanupStack::PopAndDestroy(data);

			//ignore beyond the max point
			if(pos + flashPos == aMaxReadPoint)
				{
				return;
				}
				
			if(stillMoreDataToBeHad)
				{
				//Get next type
				TRAPD(err, data = iWalker.GetNextDataTypeL(pos, id, bufferSize));
				if(err != KErrNone)
					{
					//We dont mind if this returns corrupt. This is only relevant during initial analysis
					return;
					}
				}
			}

		//if data was null but there is still more data to be had, that means we need to read more data
		//from the flash to give to our walker. it will need a buffer size of at least bufferSize, starting
		//from flashPos + pos
		flashPos += pos;
		rdSize = (bufferSize > rdSize) ? bufferSize : rdSize;

		}
	}

/**
 * Helper to convert between register value formats
 * @param aData Output format of the register data
 * @param aOriginalVal register format as it appears in flash
 */
void CFlashDataSource::ConvertFlashFormatRegL(TRegisterData& aData, const TRegisterValue& aOriginalVal)
	{

	aData.iRegClass = aOriginalVal.iClass;
	aData.iSubId = aOriginalVal.iSubId;
	aData.iId = aOriginalVal.iType;
	aData.iSize = aOriginalVal.iSize;
	aData.iAvailable = ETrue;

	switch(aOriginalVal.iSize)
		{
		case 0:
			{
			aData.iValue8 = aOriginalVal.iValue8;
			break;
			}
		case 1:
			{
			aData.iValue16 = aOriginalVal.iValue16;
			break;
			}
		case 2:
			{
			aData.iValue32 = aOriginalVal.iValue32;
			break;
			}
		case 3:
			{
			aData.iValue64 = aOriginalVal.iValue64;
			break;
			}
		//funny data, run for the hills
		default: User::Leave(KErrCorrupt);
		}
	}

/**
Purpose:
Read memory if dumped into the flash. The parameters relate to those when
the system was live and running. If this memory region has not been dumped
this leaves with a KErrNotFound

@param aThreadId - The id of the thread relative to which the read should take place
@param aAddress - The virtual address to read from
@param aLength - The number of bytes to read
@param aData - The buffer to read into
@leave KErrNotFound if memory not available or one of the system wide codes
*/
void CFlashDataSource::ReadMemoryL(
                   const TUint64    aThreadId,
                   const TUint32    aAddress,
                   const TUint32    aLength,
                   TDes8          & aData )
	{
	LOG_MSG4("CFlashDataSource::ReadMemoryL ThreadID = [%ld] aAddress = [0x%X] aLength = [%d]", aThreadId, aAddress, aLength);

	if(aLength == 0)
		{
		return;
		}

	//check arguments
	if(aData.MaxLength() < aLength)
		{
		User::Leave(KErrOverflow);
		}

	const TUint64 procId = GetThreadOwnerL(aThreadId);
	TInt errFound = KErrNone;

	//Look through our memory map to see if we have anything for this thread
	TInt cnt = iCrashMemoryMap.Count() - 1;
	for(; cnt >= 0; cnt-- )
		{
		TObjectLocation& m = iCrashMemoryMap[cnt];

		if(m.iObjectId == procId)
			{
			TInt sizeToRead = TMemoryDump::KSCMMemDumpMaxSize;

			//we must read this struct to check if we are interested in it
			User::LeaveIfError(ReadCrashLog(m.iOffset, sizeToRead));

			TInt pos = 0;
			SCMStructId id = ESCMLast;

			MByteStreamSerializable* data = iWalker.GetNextDataTypeL(pos, id, sizeToRead);

			if(id != ESCMMemory)
				{
				delete data;
				errFound = KErrCorrupt;
				//Still keep looking through the memory list
				continue;
				}

			TMemoryDump* memDump = static_cast<TMemoryDump*>(data);
			CleanupStack::PushL(memDump);

			//have we dumped this memory
			if(aAddress >= memDump->iStartAddress   && (aAddress + aLength) <= (memDump->iStartAddress + memDump->iLength))
				{
				//Now we need to read in the Raw data structure
				User::LeaveIfError(ReadCrashLog(m.iOffset + memDump->GetSize(), TRawData::KSCMRawDataMaxSize + memDump->iLength));

				TInt bufSize = 0;
				TRawData* rawData = iWalker.GetRawDataTypeL(pos, bufSize, aData, aAddress - memDump->iStartAddress);
				if(!rawData)
					{
					//descriptor not big enough
					errFound = KErrOverflow;
					CleanupStack::PopAndDestroy();

					//Still keep looking through the memory list
					continue;
					}

				CleanupStack::PushL(rawData);

				//Now we need to copy into the buffer supplied
				//aData.Copy(rawData->iData.Mid(memDump->iStartAddress - aAddress , aLength));

				CleanupStack::PopAndDestroy(2);

				return;
				}

			CleanupStack::PopAndDestroy();
			}
		}

	if(errFound == KErrNone)
		User::Leave(KErrNotFound);
	else
		User::Leave(errFound);

	}

void CFlashDataSource::GetProcessListL( RProcessPointerList & aProcessList,
											  TUint & aTotalProcessListDescSize )
    {
    LOG_MSG("CFlashDataSource::GetProcessListL()");

	// Delete any objects in the array, since we will replace them.
	aProcessList.ResetAndDestroy();
	aTotalProcessListDescSize = 0;

    //Find out where in flash to read from
    TInt flashPos = iOffsetsHdr.iPLstOffset;
    TInt rdSize = READ_SIZE;
    TBool stillMoreDataToBeHad = ETrue;

	//cycle through the flash partition, rdSize bytes at a time until there is no more data of interest to be had
	while(stillMoreDataToBeHad)
		{
		User::LeaveIfError(ReadCrashLog(flashPos, rdSize));

		//Get what we can out of this buffer read
		TInt pos, bufferSize = 0;
		SCMStructId id = ESCMLast;
		MByteStreamSerializable* data = NULL;
		TProcessData* proc = NULL;

		TRAPD(err, data = iWalker.GetNextDataTypeL(pos, id, bufferSize));
		if(err != KErrNone)
			{
			return; //We dont mind if this returns corrupt. This is only relevant during initial analysis
			}

		while(data && stillMoreDataToBeHad)
			{
			CleanupStack::PushL(data);

			switch(id)
				{
				case ESCMProcessData :
					{
					proc = static_cast<TProcessData*>(data);

			    	//Now we need to widen the name descriptor to 16 bits
			    	RBuf name16;
			    	name16.CreateL(proc->iNamesize);
			    	name16.CleanupClosePushL();

			    	name16.Copy(proc->iName);

			    	//create the new process object
			    	CProcessInfo* pi = CProcessInfo::NewL(proc->iPid, name16);
			    	aProcessList.AppendL(pi);

			    	aTotalProcessListDescSize += pi->Size();

			    	CleanupStack::PopAndDestroy();

					break;
					}
				case ESCMFirst :
				case ESCMOffsetsHeader :
				case ESCMTCrashInfo :
				case ESCMThreadData :
				case ESCMThreadStack :
				case ESCMRegisterValue :
				case ESCMRegisterSet :
				case ESCMMemory :
				case ESCMCodeSegSet :
				case ESCMCodeSeg :
				case ESCMRawData :
				case ESCMLast :
				default :
					{
					//No more registers mean no more interest
					stillMoreDataToBeHad = EFalse;
					}
				}

			CleanupStack::PopAndDestroy(data);

			//Get next type
			TRAPD(err, data = iWalker.GetNextDataTypeL(pos, id, bufferSize));
			if(err != KErrNone)
				{
				//We dont mind if this returns corrupt. This is only relevant during initial analysis
				return;
				}

			}

		if(data)
			{
			delete data;
			}

		//if data was null but there is still more data to be had, that means we need to read more data
		//from the flash to give to our walker. it will need a buffer size of at least bufferSize, starting
		//from flashPos + pos
		flashPos += pos;
		rdSize = (bufferSize > rdSize) ? bufferSize : rdSize;

		}
	}

void CFlashDataSource::GetExecutableListL( RExecutablePointerList & aExecutableList,
											  TUint & aTotalExecutableListDescSize )
    {
    //TODO
    User::Leave(KErrNotSupported);
	}

/**
 * Creates an ownership relation between a thread and a process. As when the board crashes we cant
 * find this out from the system, we need to generate it as we go
 * @param aOwningProcessId The owning process ID
 * @param aThreadId The child thread ID
 */
void CFlashDataSource::AssignOwner(TInt64 aOwningProcessId, TInt64 aThreadId)
	{
	TChildOwnerPair pair(aOwningProcessId, aThreadId);
	iThreadOwners.Append(pair);
	}

/**
 * Obtain the thread list. If aProcessId is (TUint64 -1), we obtain the entire system
 * thread list. If aProcessId is positive we get the thread list for that process
 *
 * @param aProcessId Process to gather threads for
 * @param aThreadList Thread list stored here
 * @param aTotalThreadListDescSize list size
 * @leave One of the OS wide system codes
 */
void CFlashDataSource::GetThreadListL( const TUint64 aProcessId,
											 RThreadPointerList & aThreadList,
											 TUint & aTotalThreadListDescSize )
	{
    LOG_MSG("CFlashDataSource::GetThreadListL()");

	// Delete any objects in the array, since we will replace them.
    aThreadList.ResetAndDestroy();
	aTotalThreadListDescSize = 0;

    //Find out where in flash to read from
    TInt flashPos = iOffsetsHdr.iTLstOffset;
    TInt rdSize = READ_SIZE;
    TBool stillMoreDataToBeHad = ETrue;

	//cycle through the flash partition, rdSize bytes at a time until there is no more data of interest to be had
	while(stillMoreDataToBeHad)
		{
		User::LeaveIfError(ReadCrashLog(flashPos, rdSize));

		//Get what we can out of this buffer read
		TInt pos, bufferSize = 0;
		SCMStructId id = ESCMLast;
		MByteStreamSerializable* data = NULL;
		TThreadData* thread = NULL;

		TRAPD(err, data = iWalker.GetNextDataTypeL(pos, id, bufferSize));
		if(err != KErrNone)
			{
			return; //We dont mind if this returns corrupt. This is only relevant during initial analysis
			}

		while(data && stillMoreDataToBeHad)
			{
			CleanupStack::PushL(data);

			switch(id)
				{
				case ESCMThreadData :
					{
					thread = static_cast<TThreadData*>(data);

					AssignOwner(thread->iOwnerId, thread->iTid);

					if(aProcessId == thread->iOwnerId || aProcessId == ((TUint64)-1))
						{
				    	//Now we need to widen this descriptor to 16 bits
				    	RBuf name16;
				    	name16.CreateL(thread->iNamesize);
				    	name16.CleanupClosePushL();
				    	name16.Copy(thread->iName);

				    	//Get the thread info object and add it to the thread list
			    		CThreadInfo* tinfo = CThreadInfo::NewL(thread->iTid,
			    												name16,
			    												thread->iOwnerId,
			    												thread->iPriority,
			    												thread->iSvcSP,
			    												thread->iSvcStack,
			    												thread->iSvcStacksize,
			    												thread->iUsrStack,
			    												thread->iUsrStacksize);

						// note - to minimize changes elsewhere last cpu id not added
			    		// to CThreadInfo c'tor - rather
			    		//Until the SMP work is done: tinfo->SetLastCpuId(thread->iLastCpu);
			    		tinfo->SetHeapBase(thread->iSvcHeap);
			    		tinfo->SetHeapSize(thread->iSvcHeapSize);
			    		aThreadList.AppendL(tinfo);
			    		aTotalThreadListDescSize += tinfo->Size();

			    		CleanupStack::PopAndDestroy();
						}

					break;
					}
				case ESCMFirst :
				case ESCMOffsetsHeader :
				case ESCMTCrashInfo :
				case ESCMProcessData :
				case ESCMThreadStack :
				case ESCMRegisterValue :
				case ESCMRegisterSet :
				case ESCMMemory :
				case ESCMCodeSegSet :
				case ESCMCodeSeg :
				case ESCMRawData :
				case ESCMLast :
				case ESCMTraceData :
				default :
					{
					//No more registers mean no more interest
					stillMoreDataToBeHad = EFalse;
					}
				}

			CleanupStack::PopAndDestroy(data);

			//Get next type
			TRAPD(err, data = iWalker.GetNextDataTypeL(pos, id, bufferSize));
			if(err != KErrNone)
				{
				//We dont mind if this returns corrupt. This is only relevant during initial analysis
				return;
				}

			}

		if(data)
			{
			delete data;
			}

		//if data was null but there is still more data to be had, that means we need to read more data
		//from the flash to give to our walker. it will need a buffer size of at least bufferSize, starting
		//from flashPos + pos
		flashPos += pos;
		rdSize = (bufferSize > rdSize) ? bufferSize : rdSize;

		}
	}

/**
 * Gets the owning process of this thread id
 * @param aThreadId Thread ID of interest
 * @return Owning process ID
 * @leave One of the OS wide codes
 */
TUint64 CFlashDataSource::GetThreadOwnerL(TUint64 aThreadId)
	{
	for(TInt cnt = 0; cnt < iThreadOwners.Count(); cnt++)
		{
		TChildOwnerPair pair = iThreadOwners[cnt];
		if(pair.iChildId == aThreadId)
			{
			return pair.iOwnerId;
			}
		}

	User::Leave(KErrNotFound);

	return 0;   //avoid warnings
	}

/**
 * Obtains the code segment list for a given thread.
 * @param aTid Thread identifier to obtain code segments for.
 * @param aCodeSegList Array of code segments.
 * @param aTotalCodeSegListDescSize Size in bytes of the descriptor
 * @leave One of the OS wide system codes
 */
void CFlashDataSource::GetCodeSegmentsL( const TUint64 aTid, RCodeSegPointerList &aCodeSegList, TUint & aTotalCodeSegListDescSize )
	{
	TUint64 ownerId = 0;
	TRAPD(err, ownerId = GetThreadOwnerL(aTid));

	if(KErrNone == err)
		{
		GetCodeSegmentsFromPIDL(ownerId, aCodeSegList, aTotalCodeSegListDescSize);
		}
	}

/**
 * Obtains the code segment list for a given thread.
 * @param aPid Process identifier to obtain code segments for.
 * @param aCodeSegList Array of code segments.
 * @param aTotalCodeSegListDescSize Size in bytes of the descriptor
 * @leave One of the OS wide system codes
 */
void CFlashDataSource::GetCodeSegmentsFromPIDL( const TUint64 aPid, RCodeSegPointerList &aCodeSegList, TUint & aTotalCodeSegListDescSize )
	{
	LOG_MSG("CFlashDataSource::GetCodeSegmentsL()");

	//sort out arguments
	aTotalCodeSegListDescSize = 0;
	aCodeSegList.ResetAndDestroy();

	//Look through our code seg map to see if we have anything for this process
	TInt cnt = iCodeSegMap.Count() - 1;

	for(; cnt >= 0; cnt-- )
		{
		//Look through
		TObjectLocation location = iCodeSegMap[cnt];

		//see if we have code segments for this thread ID
		if(location.iObjectId == aPid)
			{
			TInt bufferSize = TCodeSegment::KSCMCodeSegMaxSize;

			//we must read this struct to check if we are interested in it
			User::LeaveIfError(ReadCrashLog(location.iOffset, bufferSize));

			TInt pos = 0;
			SCMStructId id = ESCMLast;

			MByteStreamSerializable* data = iWalker.GetNextDataTypeL(pos, id, bufferSize);
			CleanupStack::PushL(data);

			if(id != ESCMCodeSeg || !data)
				{
				CleanupStack::PopAndDestroy(data);
				break;
				}

			TCodeSegment* seg = static_cast<TCodeSegment*>(data);

			//take our flash structure and turn it into what we need
			TCodeSegInfo* segInfo = new TCodeSegInfo();
			CleanupStack::PushL(segInfo);

			segInfo->iXIP = seg->iXip;
			segInfo->iType = seg->iCodeSegType;

			TModuleMemoryInfo memInfo = seg->iCodeSegMemInfo;

			segInfo->iCodeSize = memInfo.iCodeSize;
			segInfo->iCodeRunAddr = memInfo.iCodeBase;
			segInfo->iRoDataSize = memInfo.iConstDataSize;
			segInfo->iRoDataRunAddr = memInfo.iCodeBase + memInfo.iCodeSize;
			segInfo->iDataSize = memInfo.iInitialisedDataSize + memInfo.iUninitialisedDataSize;
			segInfo->iDataRunAddr = memInfo.iInitialisedDataBase;

			//if the code seg is XIP then these are the same
			if(segInfo->iXIP)
				{
				segInfo->iCodeLoadAddr = segInfo->iCodeRunAddr;
				segInfo->iRoDataLoadAddr = segInfo->iRoDataRunAddr;
				segInfo->iDataLoadAddr = segInfo->iDataRunAddr;
				}
			else
				{
				segInfo->iRoDataLoadAddr = 0; // as per current serverdatasource (dont worry for the moment)
				segInfo->iCodeLoadAddr = 0; // as per current serverdatasource (dont worry for the moment)
				segInfo->iDataLoadAddr = 0; // as per current serverdatasource (dont worry for the moment)
				}

			TFileName name;
			name.Copy(seg->iName);

			segInfo->iName = name;

			// crashed process code segments may have been added twice to the list as they are added
			// for the crashed process and for all processes

			TBool found = EFalse;
			for(TInt k=0;k<aCodeSegList.Count();k++)
				{
				const TCodeSegInfo& si = * aCodeSegList[k];
				if(si.iDataRunAddr ==  segInfo->iDataRunAddr && si.iDataSize == segInfo->iDataSize)
					{
					found = ETrue;

					LOG_MSG4("XXX CFlashDataSource::GetCodeSegmentsFromPIDL FOUND DUPLICATE MATCH k[%d]segInfo->iDataRunAddr[%x] segInfo->iDataSize[%d]"
							, k
							, segInfo->iDataRunAddr
							, segInfo->iDataSize );

					break;
					}
				}

			if(!found)
				{
				aCodeSegList.AppendL(segInfo);
				CleanupStack::Pop(segInfo);
				}
			else
				{
				CleanupStack::PopAndDestroy(segInfo);
				}

			CleanupStack::PopAndDestroy(data);
			}
		}

	}

/**
 * Gets the size of the trace buffer that is available
 */
TUint CFlashDataSource::GetAvailableTraceSizeL()
	{
	CLTRACE("Start CFlashDataSource::GetAvailableTraceSizeL()");

	if(iTraceDump)
		{
		return iTraceDump->iSizeOfMemory;
		}
	else
		{
		User::Leave(KErrNotFound);
		}

	return 0; //comp warnings
	}

/**
 Reads data from the trace buffer into the supplied descriptor
 @param aTraceData The buffer to be read into
 @param aPos Position in trace buffer to read from
 @leave KErrArgument if aPos is greater than the memory available or one of the other system wide codes
*/
void CFlashDataSource::ReadTraceBufferL(TDes8 &aTraceData, TUint aPos)
	{
    CLTRACE2("CFlashDataSource::ReadTraceBufferL() from aPos = [%d] into a buffer of [%d]", aPos, aTraceData.MaxLength());

    aTraceData.Zero();

    if(!iTraceDump)
    	User::Leave(KErrNotFound);

    CLTRACE1("traceDump->iNumberOfParts=%d", iTraceDump->iNumberOfParts);
    TUint nextRawDataPos = iOffsetsHdr.iTraceOffset + iTraceDump->GetSize();
    TInt currentTracePos = 0;

	//Because the trace is a circular buffer it must be read in parts and therefore dumped on the flash
	//in parts - ie. TRawData structs. Must read each of these and append to trace data
    for(TInt part = 0; part < iTraceDump->iNumberOfParts; part++)
		{
		User::LeaveIfError(ReadCrashLog(nextRawDataPos, TRawData::KSCMRawDataMaxSize));

		TInt nextRead = 0;
		SCMStructId id = ESCMLast;
		TInt nextBufSize = 0;

		MByteStreamSerializable* theRawness = iWalker.GetNextDataTypeL(nextRead, id, nextBufSize);
		if(theRawness && id == ESCMRawData)
			{
			TRawData* raw = static_cast<TRawData*>(theRawness);
			CleanupStack::PushL(theRawness);

			//Read the trace into the trace buffer
			TUint rdSize = raw->iLength;
			TUint rdPos = nextRawDataPos + TRawData::KSCMRawDataMaxSize;

			if(aPos >= (currentTracePos + raw->iLength))
				{
				//no dumpage
				}
			else
				{
				if(aPos >= currentTracePos)
					{
					rdSize = currentTracePos + raw->iLength - aPos;
					rdPos += aPos - currentTracePos;
					}

				ReadRawFlashL(aTraceData,  rdPos, rdSize);
				}

			nextRawDataPos += raw->GetSize();
			currentTracePos += raw->iLength;

			CleanupStack::PopAndDestroy(theRawness);
			}
		else
			{
			User::Leave(KErrNotFound);
			}
		}
	}

/**
 * This allows us to read directly from the flash into a buffer of our choosing
 * @param aDestinationBuffer This is the buffer we will read that data into. It will be appended on to the end of this descriptor
 * @param aReadPos Point in flash from which to read
 * @param aReadSize Amount of data to read. If this is greater than the space in aDestinationBuffer, not all the data will be read
 * @leave one of the system wide codes
 */
void CFlashDataSource::ReadRawFlashL(TDes8& aDestinationBuffer, TUint aReadPos, TUint aReadSize)
	{
	TUint8* destination = const_cast<TUint8*>(aDestinationBuffer.Ptr());
	TUint spaceInBuffer = aDestinationBuffer.MaxLength() - aDestinationBuffer.Length();
	TUint rdSize = (aReadSize > spaceInBuffer) ? spaceInBuffer : aReadSize;

	//Give it a ptr starting at the end of the desc so it doesnt overwrite any data already there (ReadCrashLog always writes the data from 0)
	TPtr8 ptr(destination + aDestinationBuffer.Length(), aDestinationBuffer.MaxLength() - aDestinationBuffer.Length());

	User::LeaveIfError(iSecSess.ReadCrashLog(aReadPos, ptr, rdSize));

	aDestinationBuffer.SetLength(aDestinationBuffer.Length() + ptr.Length());
	}

/**
 * Read the SCM configuration data from flash buffer
 */
void CFlashDataSource::ReadSCMConfigL(RConfigParameterList& aScmConfigList)
	{

	LOG_MSG("CFlashDataSource::ReadSCMConfigL");

	aScmConfigList.ResetAndDestroy();

	Debug::SCMConfiguration* theConfig = GetSCMConfigFromFlashL();

	if(!theConfig)
		{
		CLTRACE("(CFlashDataSource::ReadSCMConfigL) no config saved - using default");
		iScmConfig->SetDefaultConfig();
		}
	else
		{

		CLTRACE("(CFlashDataSource::ReadSCMConfigL) config FOUND");
		delete iScmConfig;
		iScmConfig = theConfig;

		// ok we have a config - now read to see if we have a crash following
		ReallocFlashBuf(Align4(TCrashInfoHeader::KSCMCrashInfoMaxSize));
		User::LeaveIfError(iSecSess.ReadCrashLog(Align4(iScmConfig->GetSize()), iFlashBuf, TCrashInfoHeader::KSCMCrashInfoMaxSize));

		TCrashInfoHeader header;
		TByteStreamReader r2(const_cast<TUint8*>((iFlashBuf).Ptr()));
		if( header.Deserialize(r2) == KErrNone)
			{
			CLTRACE("(CFlashDataSource::ReadSCMConfigL) HEADER found OK");

 			// we should now have a blocksize in the header
  			iCrashFound = ETrue;
  			// create a buffer big enough to hold the entire first block
  			iFirstBlockBuf.Close();
  			iFirstBlockBuf.Create(header.iFlashBlockSize);

  			// read the whole block into the buffer
  			TInt err = iSecSess.ReadCrashLog(0, iFirstBlockBuf, header.iFlashBlockSize);

  			if(err != KErrNone)
  				{
				CLTRACE1("(CFlashDataSource::ReadSCMConfigL) Error reading cache block %d", err);
				}
  			}
	 	else
  			{
  			CLTRACE("(CFlashDataSource::ReadSCMConfigL) NO crash header found");
 			}

		}

	TConfigItem* item = iScmConfig->ConfigList();
	TInt i = 0;
	while(item)
		{
		// transfer the TCongigItems into the array
    	COptionConfig* config =  COptionConfig::NewL( i,
    								   item->GetDataType(),
    								   COptionConfig::ESCMConfig,
    								   COptionConfig::ETInt,
    								   TConfigItem::GetSCMConfigOptionText(item->GetDataType()),
    								   0,
    								   KNullDesC,
    								   item->GetPriority(),
    								   KNullDesC );
        config->Instance(item->GetDataType());
        i++;
		aScmConfigList.AppendL(config);
		item = item->Next();
		}
	}

void CFlashDataSource::WriteSCMConfigL()
	{
	// if we have a crash found then use the appropriate buffer
	RBuf8& buf = iCrashFound ? iFirstBlockBuf : iFlashBuf;

	CLTRACE2("(CFlashDataSource::WriteSCMConfigL) - Writing config buf.Length() = %d iCrashFound %d",
			buf.Length(), iCrashFound );

	TByteStreamWriter writer(const_cast<TUint8*>(buf.Ptr()));
	iScmConfig->Serialize(writer);

	iSecSess.EraseCrashLog(0, 1);
	TUint32 size = 0;
	TInt res = iSecSess.WriteCrashConfig(0, buf, size);

	CLTRACE2("(CFlashDataSource::WriteSCMConfigL) RESULT res = %d size = %d", res, size);
	}

void CFlashDataSource::ModifySCMConfigItemL(TUint32 aDataType, TInt32 aPriority)
	{
	CLTRACE2("(CFlashDataSource::ModifySCMConfigItemL) - modifying aDataType %d with priority %d", aDataType, aPriority);

	//check for Priority value to be within 0-255 otherwise leave with KErrArgument
	if( (aPriority < 0) || (aPriority > 255) )
		{
			LOG_MSG( " ERROR: SCM Priority out of bounds, should be within 0-255\n");
			User::Leave(KErrArgument);
		}

	TInt err = iScmConfig->ModifyConfigItemPriority((Debug::TConfigItem::TSCMDataType)aDataType, aPriority);

	if( err == KErrNone)
		{
		// also write change through to flash
		WriteSCMConfigL();
		}
	else
		{
		 CLTRACE3("CFlashDataSource::ModifySCMConfigItemL) failed to modify err = %d aDataType = %d aPriority = %d"
				 , err, aDataType, aPriority );
		}
	}

Debug::SCMConfiguration& CFlashDataSource::SCMConfiguration()
	{
	return *iScmConfig;
	}

/**
 * Cleanup item implementation for Code Seg List
 * @param aArray pointer to the list that is supposed to be freed
*/
void CFlashDataSource::CleanupCodeSegList(TAny *aArray)
	{
    LOG_MSG("->CFlashDataSource::CleanupCodeSegList()\n");
	RCodeSegPointerList *codeSegList = static_cast<RCodeSegPointerList*> (aArray);
    codeSegList->ResetAndDestroy();
    codeSegList->Close();
    delete codeSegList;
	}

/**
 * Cleanup item implementation for process list.
 * @param aArray pointer to the list that is supposed to be freed
*/
void CFlashDataSource::CleanupProcessList(TAny *aArray)
	{
    LOG_MSG("->CFlashDataSource::CleanupProcessList()\n");
	RProcessPointerList *processList = static_cast<RProcessPointerList*> (aArray);
    processList->ResetAndDestroy();
    processList->Close();
    delete processList;
	}

/**
 * Cleanup item implementation for thread list.
 * @param aArray pointer to the list that is supposed to be freed
*/
void CFlashDataSource::CleanupThreadList(TAny *aArray)
	{
    LOG_MSG("->CFlashDataSource::CleanupThreadList()\n");
	RThreadPointerList *threadList = static_cast<RThreadPointerList*> (aArray);
    threadList->ResetAndDestroy();
    threadList->Close();
    delete threadList;
	}

/**
 * CalculateChecksum - calsulates the checksum for the given length of
 * the flash buffer (if this is the entire length it should not include
 * the checksum footer)
 * @param aLogSize - the total size of flash buf to check
 * @param aChecksum -  the checksum where the result will be placed - as this checksum is
 * commutative the aChecksum value may be reset or already hold data as required
 * @return void
*/
void CFlashDataSource::CalculateChecksumL(TInt aStart, TInt aLength, TScmChecksum& aChecksum)
	{
	if( aLength <= 0)
		{
		// invalid block length
		User::Leave(KErrGeneral);
		}

	const TInt KDefaultBlockSize = 0xFF;
	const TInt KBlocksize = aLength > KDefaultBlockSize ? KDefaultBlockSize : aLength;

	TInt remaining = aLength;
	TInt pos = aStart;

	while(remaining > KBlocksize)
		{
		//CLTRACE2("->CFlashDataSource::CalculateChecksum rem = %d blksz = %d\n", remaining, KBlocksize);
		User::LeaveIfError(ReadCrashLog(pos, KBlocksize));
		pos += KBlocksize;
		remaining -= KBlocksize;
		aChecksum.ChecksumBlock(iFlashBuf);
		}

	if(remaining > 0)
		{
		User::LeaveIfError(ReadCrashLog(pos, remaining));
		aChecksum.ChecksumBlock(iFlashBuf);

		pos += remaining;
		if(pos != aLength)
			{
			User::Leave(KErrCorrupt);
			}
		}
	}

/**
 * Retrieves ROM build information from flash partition if available
 * @param aRomHeader stores the rom build info
 * @leave one of the OS wide codes
 */
void CFlashDataSource::GetROMBuildInfoL(TRomHeaderData& aRomHeader)
	{
	if(iRomBuildInfo)
		{
		aRomHeader = *iRomBuildInfo;
		return;
		}

	User::Leave(KErrNotFound);
	}

/**
 * Retrieves System lock data
 * @param aLockData Contains the data for the system locks
 * @leave one of the OS wide codes
 */
void CFlashDataSource::GetLocksL(TSCMLockData& aLockData)
	{
	if(iLockData)
		{
		aLockData = *iLockData;
		return;
		}

	User::Leave(KErrNotFound);
	}

TInt CFlashDataSource::ReallocFlashBuf(const TUint aSize)
  	{
  	//Reallocate the flash buffer to be the size required

  	if(aSize > iFlashBuf.MaxLength())
  		{
  		TInt err = iFlashBuf.ReAlloc(aSize);
  		if (KErrNone != err)
  			{
  			CLTRACE2("(CFlashDataSource::ReallocFlashBuf) - ERROR alloc on iFlashBuf err = %d. Trying to alloc aSize [0x%x] bytes", err, aSize);
  			return err;
  			}
  		}
  	else
  		{
		iFlashBuf.SetLength(aSize);
  		}

  	return KErrNone;
  	}

void CFlashDataSource::PrintFlashBuf()
	{
	CLTRACE1("(CFlashDataSource::PrintFlashBuf) iFlashBuf.Length = %d ", iFlashBuf.Length());
	for(TInt i=0;i<iFlashBuf.Length();i++)
		{
		CLTRACE4("iFlashBuf[%d] = char<%c> dec<%d> hex<%x>", i, iFlashBuf[i], iFlashBuf[i] ,iFlashBuf[i]);
		}
	}

void  CFlashDataSource::ReleaseFirstBlockBuf()
	{
	iFirstBlockBuf.Close();
	iCrashFound = EFalse;
	}

void CFlashDataSource::GetExceptionStackL(const Debug::TArmProcessorModes aMode, TDes8& aStack, TUint aStartReadPoint)
	{
	//Not implemented
	}

/**
 * @see CCrashDataSource::GetVersion
 */
TVersion CFlashDataSource::GetVersion() const
	{
	return KFlashCrashDataSourceVersion;
	}

/**
 * @see CCrashDataSource::GetDataSourceFunctionalityBufSize
 */
TInt CFlashDataSource::GetDataSourceFunctionalityBufSize(TUint& aBufSize)
	{
	return KErrNotSupported;
	}

/**
 * @see CCrashDataSource::GetDataSourceFunctionality
 */
TInt CFlashDataSource::GetDataSourceFunctionality(TDes8& aFuncBuffer)
	{
	return KErrNotSupported;
	}
/**
 * @see CCrashDataSource::GetExceptionStackSizeL
 */
TUint CFlashDataSource::GetExceptionStackSizeL(const Debug::TArmProcessorModes aMode)
  	{
  /*    TInt bufferSize = 0;
      TInt pos = 0;
      SCMStructId id = ESCMLast;

      TBool validStack = ETrue;
      TUint32 rdPoint = iOffsetsHdr.iExcStkOffset;

      while(validStack)
      	{
      	//Read in the thread stack, memory dump and raw data structs each time
      	User::LeaveIfError(ReadCrashLog(rdPoint, TThreadStack::MAX_SIZE + TMemoryDump::MAX_SIZE + TRawData::MAX_SIZE));

      	MByteStreamSerializable* data = iWalker.GetNextDataTypeL(pos, id, bufferSize);
      	CleanupStack::PushL(data);

      	//Make sure we have a valid TRawData or TMemory or TThreadStack
      	if(id == ESCMThreadStack)
      		{
      		TThreadStack* thread = static_cast<TThreadStack*>(data);

      		//The enums here dont match so gotta case
      		switch(aMode)
      			{
      			case Debug::EFiqMode :
      				{
      				if(thread->iStackType == TThreadStack::EFIQStack)
      					{
      					CleanupStack::PopAndDestroy(data);
      					return thread.iStackSize;
      					}
      				//otherwise fall through to default
      				}
      			case Debug::EIrqMode :
      				{
      				if(thread->iStackType == TThreadStack::EIRQStack)
      					{
      					CleanupStack::PopAndDestroy(data);
      					return thread.iStackSize;
      					}

      				//otherwise fall through to default
      				}
      			//Not supported for now
      			case Debug::ESvcMode :
      			case Debug::EAbortMode :
      			case Debug::EUndefMode :
      			case Debug::EMaskMode :
      			default:
      				{
      				CleanupStack::PopAndDestroy(data);
      				continue;
      				}
      			}

      		//Now is it the stack of interest
      		}
      	else
      		{
      		CleanupStack::PopAndDestroy(data);
      		User::Leave(KErrNotFound);
      		}
      	}

  	*/
  	return 0;
	}

/**
 * @see CCrashDataSource::GetVariantSpecificDataSize
 */
TInt CFlashDataSource::GetVariantSpecificDataSize(TUint& aDataSize)
	{
	CLTRACE("Start CFlashDataSource::GetVariantSpecificDataSize()");

	if(iVarSpecData)
		{
		aDataSize = iVarSpecData->iSize;
		return KErrNone;
		}
	else
		{
		return KErrNotSupported;
		}
	}

/**
 * @see CCrashDataSource::GetVariantSpecificData
 */
TInt CFlashDataSource::GetVariantSpecificData(TDes8 &aVarSpecData)
	{
	aVarSpecData.Zero();

	if(!iVarSpecData)
		{
		CLTRACE("CFlashDataSource::GetVariantSpecificData --> No variant specific data in flash");
		return KErrNotFound;
		}

	if(aVarSpecData.MaxSize() != iVarSpecData->iSize)
		{
		CLTRACE1("CFlashDataSource::GetVariantSpecificData --> Supplied descriptor was not big enough for data. Need at least [%d] bytes", iVarSpecData->iSize);
		return KErrBadDescriptor;
		}

	TUint rawDataPos = iOffsetsHdr.iVarSpecInfOffset + iVarSpecData->GetSize();

	TInt err = ReadCrashLog(rawDataPos, TRawData::KSCMRawDataMaxSize);
	if(KErrNone != err)
		{
		CLTRACE1("CFlashDataSource::GetVariantSpecificData -- Unable to read variant data from crash log: [%d]", err);
		return err;
		}
	
	SCMStructId id = ESCMLast;
	TInt nextBufferSizeIfNull = 0;
	TInt nextBufferRead = 0;

	//We now walk the data that we just buffered in the ReadCrashLog call (our data is at start of buffer)
	MByteStreamSerializable* theRawness = iWalker.GetNextDataTypeL(nextBufferRead, id, nextBufferSizeIfNull);
	if(theRawness && id == ESCMRawData)
		{
		TRawData* raw = static_cast<TRawData*>(theRawness);
		CleanupStack::PushL(theRawness);

		//Read the raw data into supplied buffer
		TRAP(err, ReadRawFlashL(aVarSpecData, rawDataPos + TRawData::KSCMRawDataMaxSize, raw->iLength));
		CleanupStack::PopAndDestroy(theRawness);
		if(KErrNone != err)
			{
			CLTRACE1("CFlashDataSource::GetVariantSpecificData -- Reading raw data fromn flash failed: [%d]", err);
			aVarSpecData.Zero();
			return err;
			}
		}
	else
		{
		//The read crash log gave it the correct values (size etc) so if its NULL it means it's not been found
		//(Perhaps this should be a KErrCorrupt as to get this far, the crash log has been telling us there is
		//var specific data. Don't want to make the parsing too strict though.)
		return KErrNotFound;
		}

	return KErrNone;
	}

//eof flashdatasource.cpp
