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

/**
 @file
 @internalTechnology
 @released
*/

#ifndef FLASH_CRASH_DATA_SOURCE_H
#define FLASH_CRASH_DATA_SOURCE_H

#include <e32std.h>
#include <e32base.h>
#include <rm_debug_api.h>
#include <scmconfig.h>
#include <scmdatatypes.h>
#include <crashdatasource.h>
#include <crashlogwalker.h>
#include <optionconfig.h>

using namespace Debug;

const TInt KMaxBufferSize = 5000; //max size we will store in the buffer reading from crash flash
const TInt KInitialBufferSize = 0; //Size with which we shall initialise buffer

const TVersion KFlashCrashDataSourceVersion(1,2,0);

/**
 * Implementation of the CCrashDataSource interface by the core dump server, which is used by
 * formatters to gather crash data. The data is obtained from the flash partition via the Debug Security
 * Server.
 * @see CCrashDataSource
 *
 */
class CFlashDataSource : public CCrashDataSource
	{
	public:
	    static CFlashDataSource *NewL(RSecuritySvrSession &aSource);
		virtual ~CFlashDataSource();
		Debug::SCMConfiguration& SCMConfiguration();
		TInt ReadCrashLog(const TInt aPos, const TUint aSize);

		static const TInt READ_SIZE = 0x6000; // Can be Tuned

	protected:
		virtual void ConstructL();

	private:
		CFlashDataSource(RSecuritySvrSession &aSource);

	public:
		//Inherited from CCrashDataSource

		/**
		 * @see CCrashDataSource::GetRegisterListL
		 */
		virtual void GetRegisterListL( RRegisterList &aRegisterList );

		/**
		 * @see CCrashDataSource::ReadRegistersL
		 */
		virtual void ReadRegistersL( const TUint64 aThreadId, RRegisterList &aRegisterList );

		/**
		 * @see CCrashDataSource::ReadMemoryL
		 */
		virtual void ReadMemoryL(const TUint64 aThreadId,
	                   			const TUint32 aAddress,
	                   			const TUint32 aLength,
	                   			TDes8& aData );

		/**
		 * @see CCrashDataSource::GetProcessListL
		 */
		virtual void GetProcessListL(	RProcessPointerList & aData,
										TUint & aTotalProcessListDescSize );

		/**
		 * @see CCrashDataSource::GetExecutableListL
		 */
		virtual void GetExecutableListL(	RExecutablePointerList & aData,
										TUint & aTotalExecutableListDescSize );

		/**
		 * @see CCrashDataSource::GetThreadListL
		 */
		virtual void GetThreadListL(	const TUint64 aProcessId,
										RThreadPointerList & aThreadList,
										TUint & aTotalThreadListDescSize );

		/**
		 * @see CCrashDataSource::GetCodeSegmentsL
		 */
	    virtual void GetCodeSegmentsL(const TUint64 aTid, RCodeSegPointerList &aCodeSegs, TUint &aTotalCodeSegListDescSize);

		/**
		 * @see CCrashDataSource::GetCodeSegmentsFromPIDL
		 */
	    void GetCodeSegmentsFromPIDL( const TUint64 aPid, RCodeSegPointerList &aCodeSegList, TUint & aTotalCodeSegListDescSize );

	    /**
		 * @see CCrashDataSource::GetAvailableTraceSizeL
		 */
	    virtual TUint GetAvailableTraceSizeL();

		/**
		 * @see CCrashDataSource::ReadTraceBufferL
		 */
	    virtual void ReadTraceBufferL(TDes8 &aTraceData, TUint aPos = 0);

		/**
		 * @see CCrashDataSource::GetLocksL
		 */
		virtual void GetLocksL(TSCMLockData& aLockData);

		/**
		 * @see CCrashDataSource::GetROMBuildInfoL
		 */
		virtual void GetROMBuildInfoL(TRomHeaderData& aRomHeader);

		/**
		 * @see CCrashDataSource::GetExceptionStackSizeL
		 */
		virtual TUint GetExceptionStackSizeL(const Debug::TArmProcessorModes aMode);

		/**
		 * @see CCrashDataSource::GetExceptionStackL
		 */
		virtual void GetExceptionStackL(const Debug::TArmProcessorModes aMode, TDes8& aStack, TUint aStartReadPoint);

		/**
		 * @see CCrashDataSource::GetVersion
		 */
		virtual TVersion GetVersion() const;

		/**
		 * @see CCrashDataSource::GetDataSourceFunctionalityBufSize
		 */
		virtual TInt GetDataSourceFunctionalityBufSize(TUint& aBufSize);

		/**
		 * @see CCrashDataSource::GetDataSourceFunctionality
		 */
		virtual TInt GetDataSourceFunctionality(TDes8& aFuncBuffer);

		/**
		 * @see CCrashDataSource::GetVariantSpecificDataSizeL
		 */
		virtual TInt GetVariantSpecificDataSize(TUint& aDataSize);

		/**
		 * @see CCrashDataSource::GetVariantSepcificDataL
		 */
		virtual TInt GetVariantSpecificData(TDes8 &aVarSpecData);

	public:

		TInt ProcessCrashHeaderL(TInt aCrashStart);
		void AnalyseCrashL(TInt aCrashStart);

		void CalculateChecksumL(TInt aStart, TInt aLength, TScmChecksum& aChecksum);

	    TInt GetCrashLogSize() const {return iHdr.iLogSize;}
	    TInt GetFlashAlignment() const {return iHdr.iFlashAlign;}
	    TInt GetFlashBlockSize() const {return iHdr.iFlashBlockSize;}
	    TInt GetFlashPartitionSize() const {return iHdr.iFlashPartitionSize;}
	    TThreadId GetCrashedThreadId() const {return iHdr.iTid;}

	    void AssignOwner(TInt64 aOwningProcessId, TInt64 aThreadId);
	    inline TDes8& GetFlashBuffer(){return iFlashBuf; }
	    TInt GetCrashStartFromCrashIDL(TInt aCrashId);

	    Debug::SCMConfiguration* GetSCMConfigFromFlashL();
	    void ReadSCMConfigL(RConfigParameterList& aScmConfigList);

	    void ModifySCMConfigItemL(TUint32 aDataType, TInt32 aPriority);
	    void WriteSCMConfigL();
	    void PrintFlashBuf();
	    void ReleaseFirstBlockBuf();

		inline const TRmdArmExcInfo& GetCrashContext() const {return  iWalker.GetCrashContext();};
		inline const TCrashInfoHeader& GetCrashHeader() const {return  iWalker.GetCrashHeader();};
		inline const TCrashOffsetsHeader& GetOffsetsHeader() const {return  iWalker.GetOffsetsHeader();};

	private:

	    void SetRegValuesL(	const TUint64 aThreadId, RRegisterList & aRegisterList );
		void HelpReadRegistersL(TInt aRegStartPoint, TInt aMaxReadPointconst, TUint64 aThreadId, RRegisterList& aRegList);
		void ConvertFlashFormatRegL(TRegisterData& aData, const TRegisterValue& aOriginalVal);
		void ReadRawFlashL(TDes8& aDestinationBuffer, TUint aReadPos, TUint aReadSize);
	    void HelpAnalyseCrashL(const MByteStreamSerializable* aData, TInt aStructId, TInt aPosFound);
	    TUint64 GetThreadOwnerL(TUint64 aThreadId);
	    TInt ReallocFlashBuf(const TUint aSize);

	private:

		/** Session to the debug security server */
	    RSecuritySvrSession	&iSecSess;

	    /** This buffer contains a given amount of a data from the crash log at any one time */
	    RBuf8 iFlashBuf;

	    /** Tells us if the header is present */
	    TBool iOffsetsPresent;

	    /** This stores the crash offsets log header (if available)*/
	    TCrashOffsetsHeader iOffsetsHdr;

	    /** Stores the core crash header */
	    TCrashInfoHeader iHdr;

	    /** These are structs we wish to cache */
	    TCodeSegmentSet* iCurrentCodeSegSet;
	    TSCMLockData* iLockData;
	    TRomHeaderData* iRomBuildInfo;
	    TTraceDump* iTraceDump;
		TVariantSpecificData* iVarSpecData;

	    /** This struct contains the offset in the flash for a given object (thread/process)  */
	    struct TObjectLocation
		    {
		    TInt64 iObjectId;
		    TInt32 iOffset;
		    };

	    //This is an array of all the memory we have dumped in a given crash
	    RArray<TObjectLocation> iCrashMemoryMap;

	    //This array stores the code segments we have stored
	    RArray<TObjectLocation> iCodeSegMap;

	    TCrashLogWalker iWalker;

	    TInt iHeaderStart;

	    struct TChildOwnerPair
	    	{
	    	TUint64 iOwnerId;
	    	TUint64 iChildId;

	    	TChildOwnerPair(TUint64 aOwner, TUint64 aChild)
	    		{
	    		iOwnerId = aOwner;
	    		iChildId = aChild;
	    		}
	    	};

	    RArray<TChildOwnerPair> iThreadOwners;

	    /** Boolean to mark if we have processed the crash header or not */
	    TBool iHdrProcd;

	    /** Scm checksum calculator */
	    TScmChecksum iScmCheckSum;

	    Debug::SCMConfiguration* iScmConfig;

	    RBuf8	iFirstBlockBuf;
	    TBool iCrashFound;

	public:
		static void CleanupCodeSegList(TAny *aArray);
		static void CleanupProcessList(TAny *aArray);
		static void CleanupThreadList(TAny *aArray);

	};

#endif // FLASH_CRASH_DATA_SOURCE_H
