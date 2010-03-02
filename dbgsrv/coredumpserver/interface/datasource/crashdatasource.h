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
// Definition of Data Source API that supplies crash information.
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef CRASH_DATA_SOURCE_H
#define CRASH_DATA_SOURCE_H

#include <e32std.h>
#include <e32base.h>
#include <scmdatatypes.h>
#include <crashdata.h>
#include <processdata.h>
#include <executabledata.h>
#include <threaddata.h>

#ifdef __MARM__
#include <rm_debug_api.h>
#endif

/**
Definition of the Data Source API. This API is used by formatters to obtain crash data from
the Core Dump Server. The Core Dump Server derives and implements this API.
*/
class CCrashDataSource : public CBase
{
protected:

	/** Standard second-phase constructor. */
	virtual void ConstructL() = 0;

public:

	/**
	This call returns information on all available registers. It does not
	return the register contents. This is used to establish which registers
	a caller could ask for, in particular for Co Processor registers.
	The callee (not the caller) allocates aRegisterList.
	This method is used by a formatter to establish which registers it can ask for,
	and thus need only be called once per session.

    @param aRegisterList Returned register list with available registers.

    @see TRegisterData.
	@see RRegisterList.
	*/
	virtual void GetRegisterListL( RRegisterList  &	aRegisterList ) = 0;

	/**
	Ask the Core Dump server for some register data. The caller allocates
	the array and fills in the details of the registers for which it would like
	the contents.
	@param aThreadId Thread to read registers from.
	@param aRegisterList Returned register list with current values.

    @see TRegisterData.
	@see GetRegisterListL().
	@see RRegisterList.
	*/
	virtual void ReadRegistersL( const TUint64 aThreadId, RRegisterList &aRegisterList ) = 0;

	/**
 	Read data from target relative to a particular thread.
	The caller assumes ownership of the resulting data descriptor.

 	@param aThreadId Memory read is relative to this thread parameter
	@param aAddress Virtual address to read from
    @param aLength Number of bytes to read
	@param aData Descriptor for read data
	*/
	virtual void ReadMemoryL(
                   const TUint64    aThreadId,
                   const TUint32    aAddress,
                   const TUint32    aLength,
                   TDes8          & aData ) = 0;

	/**
 	Read the current process list.
	The caller assumes ownership of resulting data.
	@param aData Array of currently running processes .

	@see RProcessPointerList.
	@see CProcessInfo.
	*/
	virtual void GetProcessListL( RProcessPointerList & aData )
		{
		TUint totalProcessListDescSize;
		GetProcessListL( aData, totalProcessListDescSize );
		};

	/**
 	Read the current process list.
	The caller assumes ownership of the resulting data.
	This call is only useful if the total descriptor size required for
	transferring across the client-server boundary must be known.

 	@param aData	Array of currently running processes.
	@param aTotalProcessListDescSize Total descriptor size required
	to transfer the list across the client-server interface.
	@leave One of the OS wide codes
    @see RProcessPointerList.
	@see CProcessInfo.
	*/
	virtual void GetProcessListL( RProcessPointerList & aData,
						         TUint & aTotalProcessListDescSize ) = 0;

	/**
 	Read the current executable list. The caller assumes ownership of the resulting data.
	@param aData Array of current executables.
 	@leave One of the OS wide codes
	@see RExecutablePointerList.
	@see CExecutableInfo
	*/
	virtual void GetExecutableListL( RExecutablePointerList & aData )
		{
		TUint totalExecutableListDescSize;
		GetExecutableListL( aData, totalExecutableListDescSize );
		};

	/**
 	Read the current executable list.
	The caller assumes ownership of the resulting data.
	This call is only useful if the total descriptor size required for
	transferring across the client-server boundary must be known.

 	@param aData	Array of current executables.
	@param aTotalExecutableListDescSize	Total descriptor size required
	to transfer the list across the client-server interface.
	@leave One of the OS wide codes
	@see RExecutablePointerList.
	@see CExecutableInfo
	*/
	virtual void GetExecutableListL( RExecutablePointerList & aData,
						         TUint & aTotalExecutableListDescSize ) = 0;

 	/**
	Read the current thread list.
	This call is only useful if the total descriptor size required for
	transferring across the client-server boundary must be known.

	@param aProcessId If this argument is -1, all the threads in the
	system are returned. Otherwise the threads under the process with the id
	aProcessId are returned.
 	@param aThreadList Array of currently running threads.
	@param aTotalThreadListDescSize Size in bytes of the descriptor
	required to transfer the data over the client server interface.
 	@leave One of the OS wide codes
  	@see CThreadInfo
    @see RThreadPointerList.
	@see GetThreadListL
	*/
	virtual void GetThreadListL( const TUint64 aProcessId,
						RThreadPointerList & aThreadList,
						TUint & aTotalThreadListDescSize ) = 0;

	/**
 	Read the current thread list.
	The caller assumes ownership of the resulting data.

	@param aProcessId If this argument is -1, all the threads in the
	system are returned. Otherwise the threads under the process with the id
	aProcessId are returned.
 	@param aThreadList	Array of currently running threads.
	@leave One of the OS wide codes
	@see CThreadInfo
	@see RThreadPointerList
	*/
  	virtual void GetThreadListL( const TUint64 aProcessId,
								 RThreadPointerList & aThreadList )
		{
		TUint totalThreadListDescSize;
		GetThreadListL(	aProcessId, aThreadList, totalThreadListDescSize );
		};

    /**
	Obtain a list of the code segments for a process.

	@param aTid Thread identifier to obtain code segments for.
	@param aCodeSegs Array of code segments.
	@param aTotalCodeSegListDescSize Size in bytes of the descriptor
	required to transfer the data over the client server interface.
 	@leave One of the OS wide codes
	@see RCodeSegPointerList
	@see TSegmentInfo
	*/
    virtual void GetCodeSegmentsL(const TUint64 aTid,
									RCodeSegPointerList &aCodeSegs,
									TUint &aTotalCodeSegListDescSize ) = 0;

    /**
	Obtain a list of the code segments for a process.

	@param aTid Thread identifier to obtain code segments for.
	@param aCodeSegs Array of code segments.
    @leave One of the OS wide codes
	@see RCodeSegPointerList
	@see TSegmentInfo
	*/
    virtual void GetCodeSegmentsL(const TUint64 aTid, RCodeSegPointerList &aCodeSegs)
        {
        TUint totalThreadListDescSize;
        GetCodeSegmentsL(aTid, aCodeSegs, totalThreadListDescSize);
        };

    /**
     * Returns the size of the trace buffer that is available. Should be used in conjunction with ReadTraceBufferL to
     * determine the size of the buffer you need to pass in.
     * @return Trace Buffer Size
     * @leave One of the OS wide codes
     * @see ReadTraceBufferL
     */
    virtual TUint GetAvailableTraceSizeL() {User::Leave(KErrNotSupported); return 0;};

	/**
	 * Reads trace data from trace buffer at time of crash. If the resulting
	 * descriptor is of size zero then no trace data is available. Use GetAvailableTraceSizeL to see what size buffer to supply.
	 * @param aTraceData Descriptor to store trace data
	 * @param aPos Position in the trace buffer from which to read. It will read to the end or until the supplied descriptor is full, whichever happens first.
	 * @leave One of the OS wide codes
	 * @see GetAvailableTraceSizeL
	 */
	virtual void ReadTraceBufferL(TDes8 &aTraceData, TUint aPos = 0 ) {User::Leave(KErrNotSupported); };

	/**
	 * Reads locks data
	 * @param aLockData Descriptor to store trace data
	 * @leave One of the OS wide codes
	 * @see TSCMLockData
	 */
	virtual void GetLocksL(TSCMLockData& aLockData) {User::Leave(KErrNotSupported); };

	/**
	 * Reads the ROM Build Info
	 * @param aRomHeader Contains the ROM header info after the call if succesful
	 * @leave One of the OS wide codes
	 * @see TRomHeaderData
	 */
	virtual void GetROMBuildInfoL(TRomHeaderData& aRomHeader) {User::Leave(KErrNotSupported); };

	/**
	 * Gets the size of any variant specific information that might be available. Should
	 * be used in conjunction with GetVariantSpecificDataL to determine the size of descriptor
	 * required to be passed in	 *
	 * @return One of the system wide error codes
	 * @param aDataSize Stores the datasize after the call
	 * @see GetVariantSpecificData
	 */
	virtual TInt GetVariantSpecificDataSize(TUint& aDataSize) = 0;

	/**
	 * Gets any variant specific data that might be available
	 * @return One of the system wide error codes
	 * @param aVarSpecData Reference to descriptor to store data. Must be large enough
	 *			to store the data. If this returns size zero, no data was available
	 * @see GetVariantSpecificDataSize
	 */
	virtual TInt GetVariantSpecificData(TDes8& aVarSpecData) = 0;

#ifdef __MARM__

	/**
	 * If the stack for the given mode is available, this will return the size of it
	 * @return The size of the stack
	 * @param aMode Processer mode of interest
	 * @leave One of the OS wide codes
	 * @see Debug::TArmProcessorModes
	 * @see GetExceptionStackL
	 */
	virtual TUint GetExceptionStackSizeL(const Debug::TArmProcessorModes aMode) {User::Leave(KErrNotSupported); };

	/**
	 * Reads the exception stack for a given exception mode into the descriptor. Will start reading
	 * aStartReadPoint bytes into the stack and will fill up the descriptor as much as possible. The intention
	 * is to make multiple calls of this so you don't have to allocate a large buffer at the start.
	 * @param aMode Processer mode of interest
	 * @param aStack Descriptor to put stack into
	 * @param aStartReadPoint Will start reading aStartReadPoint bytes into the stack
	 * @see GetExceptionStackSizeL
	 */
	virtual void GetExceptionStackL(const Debug::TArmProcessorModes aMode, TDes8& aStack, TUint aStartReadPoint) {User::Leave(KErrNotSupported); };
#endif   //MARM_ARMV5

	/**
	 * Returns the version of this implementation of the data source
	 * @return TVersion Data Source version
	 */
	virtual TVersion GetVersion() const  { return TVersion(1,0,0);};

	/**
	 * Returns the buffer size that will be required to read the Data Source
	 * functionality block
	 * @param aBufSize Size of the buffer required
	 * @return One of the OS wide codes
	 * @see GetDataSourceFunctionality
	 */
	virtual TInt GetDataSourceFunctionalityBufSize(TUint& aBufSize)   {return 0;};

	/**
	 * Returns the Data Source Functionality block. Should be used in conjunction with
	 * GetDataSourceFunctionalityBufSize to determine how big a buffer should be passed through.
	 *
	 * Information in the debug functionality block is represented as a concatenation
	 * of TTag objects and a TTagHeader.
	 *
	 * @param aBuffer Contains the functionality block
	 * @return One of the OS wide codes
	 * @see GetDataSourceFunctionalityBufSize
	 * @see Debug::TTag
	 * @see Debug::TTagType
	 * @see Debug::TTagHeader
	 */
	virtual TInt GetDataSourceFunctionality(TDes8& aFuncBuffer)   { return 0;};

	/** For future expansion
	@internalTechnology */
	TUint32 iSpare1;

	/** @internalTechnology */
	TUint32 iSpare2;

public:

	/**
	 * These tags define what kinds of core functionality is supported by a given implementation of
	 * the data source. TTag structures associated with the ETagHeaderIdCore sub-block will have iTagId values from this enumeration.
	 * See each enumerator for an explanation of how a TTag with that iTagId should be interpreted.
	 */
	enum TDataSourceFunctionalityCore
	{
		ECodeSegments = 0,        /**< Indicates whether getting code segments is supported */
		EExecutableList = 1,     /**< Indicates whether getting the executable list is supported */
		EProcessList = 2,        /**< Indicates whether getting the process list is supported */
		ERegisterList = 3,      /**< Indicates whether getting the register list is supported */
		EThreadList = 4,    /**< Indicates whether getting the thread list */
		EReadMemory = 5,      /**< Indicates whether reading memory is supported. */
		EReadRegisters = 6,         /**< Indicates whether reading registers is supported. */
		EReadTraceBuffer = 7,       /**< Indicates whether reading the trace buffer is supported. */
		ERomBuildInfo = 8,      /**< Indicates whether obtaining the ROM build info is supported */
		ESystemLocks = 9,  /**< Indicates whether obtaining the System Locks info is supported */
		EExceptionStacks = 10,  /**< Indicates whether obtaining the exception stacks is supported */
		/**
		  @internalTechnology
		  A debug agent should find the number of core tags from the DFBlock rather than this enumerator.
		  */
		ECoreLast
	};

	/**
	 * Information in the Data Source functionality block is represented as a concatenation
	 * of pairs of TDataSourceTagHeader structures and arrays of TTag objects.
	 * @see Debug::TTag
	 * @see GetDataSourceFunctionality
	  */
	struct TDataSourceTagHeader
	{
		/** Value identifying the contents of this TTagHeader, should be interpreted as an enumerator from TTagHeaderId.
		  @see TTagHeaderId
		  */
		TUint16	iTagHdrId;
		/** The number of TTag elements in the array associated with this TTagHeader. */
		TUint16 iNumTags;
	};

	/**
	 * Enumeration used to identify TDataSourceTagHeader structures, TDataSourceTagHeader::iTagHdrId elements take
	 * these enumerators as values.
	 * @see TDataSourceTagHeader
	 */
	enum TDataSourceTagHeaderId
	{
		ETagHeaderIdCore = 0,            /**< Identifies a TTagHeader with associated TTag elements with iTagId values from TDataSourceFunctionalityCore. */
		/**
		 * @internalTechnology
		 */
		ELast
	};

};

#endif // CRASH_DATA_SOURCE_H
