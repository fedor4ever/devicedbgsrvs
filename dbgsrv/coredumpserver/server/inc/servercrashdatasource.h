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

/**
 @file
 @publishedPartner
 @released
*/

#ifndef SERVER_CRASH_DATA_SOURCE_H
#define SERVER_CRASH_DATA_SOURCE_H

#include <e32std.h>
#include <e32base.h>
#include <rm_debug_api.h>
#include <crashdatasource.h>

using namespace Debug;

const TVersion KServerCrashDataSourceVersion(2,1,0);

/**
Implementation of the CCrashDataSource interface by the core dump server, which is used by
formatters to gather crash data. The data is obtained from the Debug Security Server.
@see CCrashDataSource

*/
class CServerCrashDataSource : public CCrashDataSource
{
public:
    static CServerCrashDataSource *NewL(RSecuritySvrSession &aSource);
	virtual ~CServerCrashDataSource();

protected:
	virtual void ConstructL();

private:
    CServerCrashDataSource(RSecuritySvrSession &aSource);

public:

	/**
	See CCrashDataSource::GetRegisterListL()
	*/
	virtual void GetRegisterListL( RRegisterList &aRegisterList );//const

	/**
	See CCrashDataSource::ReadRegistersL()
	*/
	virtual void ReadRegistersL( const TUint64 aThreadId, RRegisterList &aRegisterList );

	/**
	See CCrashDataSource::ReadMemoryL()
	*/
	virtual void ReadMemoryL(
                   const TUint64    aThreadId,
                   const TUint32    aAddress,
                   const TUint32    aLength,
                   TDes8          & aData );

	/**
	See CCrashDataSource::GetProcessListL()
	*/
	virtual void GetProcessListL(	RProcessPointerList & aData,
									TUint & aTotalProcessListDescSize );

	/**
	See CCrashDataSource::GetExecutableListL()
	*/
	virtual void GetExecutableListL(	RExecutablePointerList & aData,
									TUint & aTotalExecutableListDescSize );

	/**
	See CCrashDataSource::GetThreadListL()
	*/
	virtual void GetThreadListL(	const TUint64 aProcessId,
									RThreadPointerList & aThreadList,
									TUint & aTotalThreadListDescSize );

	/**
	See CCrashDataSource::GetCodeSegmentsL()
	*/
    virtual void GetCodeSegmentsL(const TUint64 aTid, RCodeSegPointerList &aCodeSegs, TUint &aTotalCodeSegListDescSize);

    /**
	 * @see CCrashDataSource::ReadTraceBufferL
     */
    virtual void ReadTraceBufferL(TDes8 &aTraceData, TUint aPos = 0);

    /**
	 * @see CCrashDataSource::GetAvailableTraceSizeL
	 */
    virtual TUint GetAvailableTraceSizeL();

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

private:
    void SetRegValuesL(	const TUint64 aThreadId, RRegisterList & aRegisterList );

	void PrintRegs( RRegisterList  &	aRegisterList );

	enum TListLevel
		{
		EListGlobal,
		EListThread,
		EListProcess
		};

	void DoGetListL( const TListId aListId,
				    const TThreadId aThreadId,
					const TProcessId aProcessId,
					RBuf8 & aBuffer,
					TUint32 & aSize );

	TTagHeader * GetDebugHeader( const TTagHeaderId aTagHdrId, const TDesC8 &aDFBlock );
	TTag* GetTag(const TTagHeader* aTagHdr, const TInt aElement);

private:
    RSecuritySvrSession	&iSecSess;

	RRegisterList iRegisterList;
	RBuf8 iThreadListBuffer;
	RBuf8 iProcListBuffer;
	RBuf8 iExecutableListBuffer;

	TUint32 iLastThreadListSize;
	TUint32 iLastProcListSize;
	TUint32 iLastExecutableListSize;
	TUint32 iLastRegListSize;
	TUint32 iMaxMemReadSize;

	TUint32 iSpare0;
	TUint32 iSpare1;
	TUint32 iSpare2;
	TUint32 iSpare3;

};

#endif // TEST_CRASH_DATA_SOURCE_H
