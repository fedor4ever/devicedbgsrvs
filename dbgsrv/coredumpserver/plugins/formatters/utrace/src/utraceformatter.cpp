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

#include <d32btrace.h>

#include "utraceformatter.h"


/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CUtraceFormatter* CUtraceFormatter::NewL()
	{
    LOG_MSG("->CUtraceFormatter::NewL()\n");
    CUtraceFormatter* self = CUtraceFormatter::NewLC();
	CleanupStack::Pop(self);
    return self;
	}

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CUtraceFormatter* CUtraceFormatter::NewLC()
	{
    LOG_MSG("->CUtraceFormatter::NewLC()\n");
    CUtraceFormatter* self = new(ELeave) CUtraceFormatter();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/**
dtor
*/
CUtraceFormatter::~CUtraceFormatter()
	{
    LOG_MSG("->~CDexcFormatter()\n");
    iConfigList.ResetAndDestroy();
	}

/**
2nd stage construction
*/
void CUtraceFormatter::ConstructL()
	{
	LOG_MSG("->CDexcFormatter::ConstructL()\n");
	
	//Configure the plugin
	COptionConfig * config = NULL;
	
	iConfigList.ReserveL( (TInt) ETraceGuard );
	
	_LIT(KTraceSize, "How much trace data to dump? (Kb)");
	
	config = COptionConfig::NewL(   (TInt) ETraceSize,    
									KUtraceFormatterUID,
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETInt,
									KTraceSize,
									1,
									KNullDesC,
									100,
									KNullDesC);
	
	TInt error = iConfigList.Append(config);
	
	if(error != KErrNone)
		{
		delete config;
		LOG_MSG2("\tCDexcFormatter::ConstructL() - unable to store COptionConfig object in param list: %d", error);
		}

	}

/**
Called by CDS to ask for the configuration parameters that the writer needs (see Plugin Configuration) 
@return actual number of implemented config parameters
*/
TInt CUtraceFormatter::GetNumberConfigParametersL()
	{
	TInt count = iConfigList.Count();
	LOG_MSG2("->CUtraceFormatter::GetNumberConfigParametersL() : returns:%d\n", count );
	return count;
	}

/**
Called by CDS to ask for configuration parameter prompt (see Plugin Configuration) 

@param aIndex indicates which parameter to return
@return pointer to COptionConfig object representing the requested config parameter. Caller doesn't take ownership of the object!
@leave KErrBadHandle if index is out of bounds
@see COptionConfig
*/
COptionConfig* CUtraceFormatter::GetConfigParameterL(const TInt aIndex)
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
void CUtraceFormatter::SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue )
	{
	
	if( ( aIndex < 0 ) || ( aIndex >= iConfigList.Count() ) )
		{
			User::Leave( KErrBadHandle );
		}
	
	COptionConfig & config = *(iConfigList[aIndex]);

	if( ( aIndex != (TInt)ETraceSize ) )
		{
			LOG_MSG( "\tLeaving : aIndex != ETraceSize " );
			User::Leave( KErrArgument );
		}
	
	//For the moment, we care not for the aDescValue
	if(aValue > 0)
		{
		config.Value(aValue);
		config.ValueL(KNullDesC);
		}
	
	}

/**
Sets pointer to the writer plugin.
Configures formatter's dump data save plugin.

@param aDataSave pointer to writer plugin object.
@leave KErrNoMemory if aDataSave is NULL 
*/
void CUtraceFormatter::ConfigureDataSaveL(CCrashDataSave* aDataSave)
	{
    LOG_MSG2("\tConfigureDataSaveL(%d)", aDataSave);
    User::LeaveIfNull(aDataSave);
    iDataSave = aDataSave;
	}

/**
Sets pointer to the crash data server.
Configures formatter's crash data source plugin.

@param aDataSource pointer to core dump server object.
@leave KErrNoMemory if aDataSource is NULL 
*/
void CUtraceFormatter::ConfigureDataSourceL(CCrashDataSource* aDataSource)
	{
    LOG_MSG2("\tConfigureDataSourceL(%d)", aDataSource);
    User::LeaveIfNull(aDataSource);
    iDataSource = aDataSource;
	}

void CUtraceFormatter::GetDescription(TDes& aPluginDescription)
	{
    LOG_MSG("\tCDexcFormatter::GetDescription()");
	aPluginDescription.Copy(KPluginDescription());
	}

void CUtraceFormatter::CrashEventL(TCrashInfo* aCrashInfo)
	{
	LOG_MSG("->CUtraceFormatter::CrashEventL\n");
	
	User::LeaveIfNull(aCrashInfo);
    User::LeaveIfNull(iDataSource);
    User::LeaveIfNull(iDataSave);
    
    TInt traceDataToCapture = iConfigList[ (TInt)ETraceSize ]->Value();
	if(traceDataToCapture == 0)
		{
		LOG_MSG("CUtraceFormatter::WriteTraceBufferL - not configured to dump trace data");
		return;
		}
    
    //update notifier
    UpdateCrashProgressL( KUTraceCrashStart );
    
    iDataSave->OpenL( DumpFileNameLC(aCrashInfo->iTime ));
    CleanupStack::PopAndDestroy(); //filename
    
	TUint traceAvailable = 0;	
	TRAPD(err, traceAvailable = iDataSource->GetAvailableTraceSizeL());
	if(err != KErrNone)
		{
		LOG_MSG2("Unable to get available trace size. Err = [%d] ", err);
		User::Leave(err);
		}
	
	//Write the data to the file
	LOG_MSG2("\tWriting Trace data: iDataSave->WriteL() at Offset =0, size =%d bytes", traceAvailable);
	TUint size = 0;
	
	TRAP(err, size = HelpWriteTraceBufferL(traceAvailable, traceDataToCapture * 1024, 0));
	if(err == KErrNotSupported)
		{
		LOG_MSG("Unable to use data source to dump trace so reverting to the live trace framework");
		TRAP(err, size = HelpWriteTraceBufferWithTraceFrameWorkL(traceDataToCapture * 1024, 0));
		}
	
	if(err != KErrNone)
		{
		//This error is not fatal but worth logging
		LOG_MSG2("Trace writing didnt succeed. Error = [%d]", err);
		}	

	LOG_MSG2("\tTrace data and info succesfully written - [%d] bytes", size);
	
	iDataSave->CloseL();
	}

/**
 * This gets trace from the data source. This is for use when you dont want trace from the live system and want 
 * the data source to decide how we obtain the trace. The data source may not support this, and leaving with
 * KErrNotSupported will mean that you should be using the live trace system to get the data
 * @param aTraceAvailable The amount of availbale trace data
 * @param aTraceWanted The amount of trace data we wish to dump
 * @param aOffset The position in the file to write the trace
 * @see HelpWriteTraceBufferWithTraceFrameWorkL
 */
TUint CUtraceFormatter::HelpWriteTraceBufferL(TUint aTraceAvailable, TUint aTraceWanted, TUint aOffset)
	{
	LOG_MSG4("CUtraceFormatter::HelpWriteTraceBufferL aTraceAvailable = [%d] aTraceWanted = [%d] aOffset = [%d]", aTraceAvailable, aTraceWanted, aOffset);
	
	TUint dataReq = (aTraceAvailable >= aTraceWanted) ? aTraceWanted : aTraceAvailable;
	TUint bufferSize = dataReq;
	RBuf8 theTraceData;	
	TBool bufferCreated = EFalse;
	
	//Try and allocate the buffer
	while(!bufferCreated)
		{
		LOG_MSG2("Trying to create a buffer of size [%d] bytes", bufferSize);
		
		if(bufferSize == 0)
			{
			User::LeaveNoMemory();
			}
		
		TInt err = theTraceData.Create(bufferSize);
		if(KErrNone == err)
			{
			bufferCreated = ETrue;
			theTraceData.CleanupClosePushL();
			}
		else
			{
			bufferSize = bufferSize/4;
			}
		}
	
	//Now we read the trace data bit by bit
	TUint sizeWritten = 0;
	while(sizeWritten < dataReq)
		{		
		iDataSource->ReadTraceBufferL(theTraceData, sizeWritten);
		
		LOG_MSG3("Writing [%d] bytes of raw trace data at [%d]", theTraceData.Size(), aOffset);
		iDataSave->WriteL( aOffset, const_cast<TUint8*>(theTraceData.Ptr()), theTraceData.Size());
		
		sizeWritten += theTraceData.Length();
		aOffset += theTraceData.Length();
		}
	
	CleanupStack::PopAndDestroy(&theTraceData);
	
	return sizeWritten;
	}

/**
 * Writes trace to the file using the trace framework. This is to use when you want trace from the live system.
 * @param aTraceWanted The amount of trace data we wish to dump
 * @param aOffset The position in the file to write the trace
 * @see HelpWriteTraceBufferL
 */
TUint CUtraceFormatter::HelpWriteTraceBufferWithTraceFrameWorkL(TUint aTraceWanted, TUint aOffset)
	{
	LOG_MSG3("CUtraceFormatter::HelpWriteTraceBufferWithTraceFrameWorkL aTraceWanted = [%d] aOffset = [%d]", aTraceWanted, aOffset);
    
    RBTrace trace;
    User::LeaveIfError(trace.Open());
    CleanupClosePushL(trace);
    
    TUint8 *data = NULL;
    TInt size = trace.GetData(data);
    
    if(!data)
    	{
    	User::Leave(KErrNotFound);
    	}
    
    TUint sizeToDump = (aTraceWanted >= size) ? size : aTraceWanted;
    
    LOG_MSG2("Adding [%d] bytes to the file", sizeToDump);
    iDataSave->WriteL( aOffset, data, sizeToDump);
    
    CleanupStack::PopAndDestroy(&trace);	
	
	return size;
	}

/**
Prepares the name of the dump file. Checks DataSave filename option, if it is provided by the user
it appends the time stamp, if not it uses default name appended with the time stamp. 
@param aStamp lower 32 bits of the time stamp
@return descriptor holding the final dump file name, caller is responsible to pop it from the cleanup stack
@leave err one of system wide error codes
 */
const TDesC& CUtraceFormatter::DumpFileNameLC(TUint64 aStamp)
{
    LOG_MSG("->CUtraceFormatter::DumpFileNameLC()\n");
    COptionConfig *option = iDataSave->GetConfigParameterL(CCrashDataSave::ECoreFilePath);
	const TDesC &userFileName = option->ValueAsDesc();
	HBufC *filename = HBufC::NewLC(userFileName.Length() + KUTraceDumpFileLength);
    if(userFileName.Length() > 0)
        {
	    *filename = userFileName;
        _LIT(KStampFormat, "%u");
        filename->Des().AppendFormat(KStampFormat, I64LOW(aStamp));
        }
    else
        {
        _LIT(KTxtFileFormat, "e:\\trace%u");
        filename->Des().Format(KTxtFileFormat, aStamp);
        }
    return *filename;
}

/**
Checks the value of the crash cancel property. If set to cancel, it closes the 
data save and then leaves with KErrAbort.
Otherwise it updates the crash progress with the argument value.

@param aProgress Progress description.
@leave KErrAbort if crash cancel property has been set
*/
void CUtraceFormatter::UpdateCrashProgressL( const TDesC &aProgress )
    {
    
    LOG_MSG("->CUtraceFormatter::UpdateCrashProgress()\n");
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
