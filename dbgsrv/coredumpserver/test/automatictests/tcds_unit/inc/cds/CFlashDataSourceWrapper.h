/**
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/



/**
 @file CFlashDataSourceWrapper.h
 @internalTechnology
 @prototype 
*/

#ifndef __CFLASHDATASOURCEWRAPPER_H__
#define __CFLASHDATASOURCEWRAPPER_H__

#include <datawrapper.h>
#include <rm_debug_api.h>
#include <scmdatatypes.h>
#include <scmbytestreamutil.h>

#include "flashdatasource.h"

using namespace Debug;

/**
This is the version of the security server that we have developed our code against
and it comes from the interface definition of the DSS at the time of compilation.
*/
const TVersion securityServerVersion( 
				KDebugServMajorVersionNumber, 
				KDebugServMinorVersionNumber, 
				KDebugServPatchVersionNumber );

const TInt CRASH_ID = 1;

const TUint KMaxCacheSize = 32;
/**
 * This class is responsible for testing our CFlashDataSource class
 */
class CFlashDataSourceWrapper : public CDataWrapper
	{
	
public:
	
	~CFlashDataSourceWrapper();

	static CFlashDataSourceWrapper* NewL();

	virtual TBool DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);
	virtual TAny* GetObject() { return iObject; }
	virtual void SetObjectL(TAny* aObject);

protected:
	void ConstructL();

private:
	
	/** Constructor */
	CFlashDataSourceWrapper();	
	
	/** Tests CFlashDataSource::NewL */
	void DoCmdCrashFlashDataSource_NewL_TestL();
	
	/** Tests CFlashDataSource::ReadCrashLog 
	 *  Ensures the crash Log can be read correctly into our flash buffer
	 */
	void DoCmd_ReadCrashLog_L();
	
	/** Tests CFlashDataSource::GetFlashBuffer 
	 *  Ensures that the flash buffer is correctly returned by getter
	 */
	void DoCmd_GetFlashBuffer_L();
	
	/** Tests CFlashDataSource::ProcessCrashHeader 
	  * Ensures we can correctly process the crash header
	  */
	void DoCmd_ProcessCrashHeader1_L();
	
	/** Tests CFlashDataSource::ProcessCrashHeader 
	  * Ensures we can recognise a corrupt header
	  */
	void DoCmd_ProcessCrashHeader2_L();
	
	/** Tests CFlashDataSource::GetProcessListL 
	 *  Writes known process list to flash and makes sure we can get it back
	 */
	void DoCmd_GetProcessListL1_Test_L();
	
	/** Tests CFlashDataSource::GetProcessListL 
	 *  Ensure we recognise corrupt process list
	 */
	void DoCmd_GetProcessListL2_Test_L();
	
	/** Tests CFlashDataSource::GetThreadList 
	 *  Writes known system wide thread list to flash and makes sure we can get it back
	 */
	void DoCmd_GetThreadListL1_Test_L();
	
	/** Tests CFlashDataSource::GetThreadList 
	 *  Ensure we recognise corrupt thread list
	 */
	void DoCmd_GetThreadListL2_Test_L();
	
	/** Tests CFlashDataSource::GetThreadList 
	 *  Looks at a process specific thread list
	 */
	void DoCmd_GetThreadListL3_Test_L();
	
	/** Tests CFlashDataSource::ReadRegistersL
	 *  Writes known registers to flash and ensures the correct ones are returned 
	 */
	void DoCmd_ReadRegistersL1_Test_L();
	
	/** Tests CFlashDataSource::ReadMemoryL 
	 *  Writes known memory to flash and ensures the correct ones are returned  
	 */
	void DoCmd_ReadMemoryL1_Test_L();
	
	/** Negative tests for read memory 
	 *  Ensures we cant read memory we havent dumped 
	 */
	void DoCmd_ReadMemoryL2_Test_L();
	
	/** Tests CFlashDataSource::GetCodeSegmentsL
	 *  Writes known code segments to flash and ensures we can get them back
	 */
	void DoCmd_GetCodeSegmentsL1_Test_L();
	
	/** Tests CFlashDataSource::GetCodeSegmentsL
	 *  Ensure we recognise corrupt data
	 */
	void DoCmd_GetCodeSegmentsL2_Test_L();

	/**
	 *  Tests CFlashDataSource::GetCodeSegmentsL
	 *  Ensure we do not recieve duplicate segments back 
	 *  even when we write duplicates to flash
	 */
	void DoCmd_GetCodeSegmentsL3_Test_L();

	/** Tests CFlashDataSource::ReadTraceBufferL
	 *  Ensure we can read back the correct test data
	 */
	void DoCmd_ReadTraceBufferL1_Test_L();
	
	/** Tests data checksums */
	void DoCmd_CalculateChecksum_TestL();
	
	/** Tests we can get back the correct trace size */
	void DoCmd_TraceDataSizeL_TestL();
	
	/** Tests we can handle the size not being found */
	void DoCmd_TraceDataSizeNotFoundL_TestL();
	
	/** Tests we can read trace with an offset */
	void DoCmd_ReadTraceBufferL2_Test_L();
	
	void PrepareTestL();
	
private:
	
	/** Object to be tested */
	CFlashDataSource* iObject;
	
	/** Handle to debug security server */
	RSecuritySvrSession iSecSess;
	
	/** These are our crash headers */
	TCrashInfoHeader iInfHdr;
	TCrashOffsetsHeader iOffsetsHdr;
	TRegisterSet iContextHdr;
	
	};
#endif // __CFLASHDATASOURCEWRAPPER_H__
