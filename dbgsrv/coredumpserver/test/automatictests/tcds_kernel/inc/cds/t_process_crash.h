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
 @file t_process_crash.h
 @internalTechnology
 @prototype
*/

#ifndef __CPROCESSCRASHWRAPPER_H__
#define __CPROCESSCRASHWRAPPER_H__
#include <e32property.h>
#include <datawrapper.h>
#include <e32base.h>
#include <f32file.h>
#include <bafindf.h >
#include <e32btrace.h>
#include <e32std.h>
#include <hal.h>

#include <coredumpinterface.h> //core dump interface API
#include <optionconfig.h>
#include <coredumpserverapi.h>
#include <crashdatasave.h>

#include <scmdatatypes.h>
#include <symbianelfdefs.h>
#include <scmconfigitem.h>

#include "t_common_defs.h"

const TInt MAXNUMBEROFDHDR = 200;

enum TValidateString {EValExecutable, EValThread};

class CProcessCrashWrapper;
/**
 * This class implements an Active Object to test the asynchronous ProcessCrashLog
 */
class CAsyncProcessCrash : public CActive
	{
public:

	static CAsyncProcessCrash* NewL(CProcessCrashWrapper* );

	~CAsyncProcessCrash( );

	void IssueProcessCrashRequest(TUint aCrashId);

private:

	void ConstructL();

	CAsyncProcessCrash(CProcessCrashWrapper* );

  // from CActive
	void RunL();

	TInt RunError(TInt aError);

	void DoCancel();

	CProcessCrashWrapper* iProcessCrashWrapper;

	TUint iCrashId;
  	} ;

/**
 * This class is responsible for processing the crash information
 */
class CProcessCrashWrapper : public CDataWrapper
	{

public:

	~CProcessCrashWrapper();

	static CProcessCrashWrapper* NewL();

	virtual TBool DoCommandL(const TTEFFunction& aCommand,
			const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);
	virtual TAny* GetObject()
		{
		}
	virtual void SetObjectL(TAny* aObject);

protected:
	void ConstructL();

private:
	/** Constructor */
	CProcessCrashWrapper();

	void PrepareTestL();
	void DoCmdNewL();
	void DoCmdReadCrashInfoL();
	void DoCmdCheckMultipleCrashesL();
	void DoCmdDeleteAllCrashL();
	void DoCmdLoadPluginsL();
	void DoCmdUnLoadPluginsL();
	void DoCmdProcessCrashL(const TDesC&);
	void DoCmdConfigurePluginsL(const TDesC& );
	void DoCmdConfigureSCMZeroPriorityL();
	void DoConfigureL(const TUint32& aIndex, const TUint32& aUID,
			const COptionConfig::TParameterSource& aSource,
			const COptionConfig::TOptionType& aType, const TDesC& aPrompt,
			const TUint32& aNumOptions, const TDesC& aOptions,
			const TInt32& aVal, const TDesC& aStrValue, const TUint aInstance);

	void DoCmdValidateSingleELFFileL();
	void DoCmdValidateAsyncProcessCrashLogL();
	void DoCmdValidateAsyncProcessAndLiveCrashCrashLogL(const TDesC& );
	void GenerateLiveCrashL(const TDesC& );
	void ProcessSELFFileCreatedL();
	void DoProcessSELFLiveandKernelL();
    void DoCmdValidateMultipleELFFileL();
    void DoCmdValidateCpuIDL();
    void DoCmdValidateHeapSELFFileL();
    void DoCmdValidateTraceSELFFileL();
    void DoCmdPerformanceMeasureSELFFileL();
    void DoCmdValidateCorruptCrashL();
	void DoCmdConfigureSELFPluginWithInvalidnessL();

	void ValidateHeaderELFFileL();
	void ValidateProgramHeaderELFFileL();
    void ValidateSymInfoSectionELFFileL();
    void ValidateThreadInfoSectionELFFileL(TBool aValidateCpuId, const TDesC& aThreadCrashed, TBool aCheck);
    void ValidateRegisterInfoSectionELFFileL();
    void ValidateHeapSectionELFFileL(TBool );
    void ValidateTraceSectionELFFileL(TBool );
    void ValidateExceptionL(TInt ,TInt , TBool );
	void ValidateStringL(TValidateString , TInt , const TDesC& , TBool );
	void ValidateTraceBufferL(const TDesC8&  );
	void ValidatePerformanceELFFile();

    void GenerateElfFileL(TUint aCrashId, TBool aTiming = EFalse);
    void MonitorProgressL();
	void ConfigureSELF(TBool , TInt );
	void CleanupMethod(const TDesC& );

	void HelpStartTestTimer(void) { iStartTick = User::NTickCount(); iStopTick = 0; };
	void HelpStopTestTimer(void) { iStopTick = User::NTickCount(); };
	TInt HelpGetTestTicks(void) { return (iStopTick - iStartTick); };
	TInt HelpTicksPerSecond();

	void PrintErrorCondition(TInt );

private:

	/** Object to be tested */
	//session object to the Core Dump Server
	RCoreDumpSession iCoreDumpSession;

	//TCrashInfo structure for crash info
	RCrashInfoPointerList iCrashList;

	//Active Object to handle async processing
	CAsyncProcessCrash* iProcessCrash;

	//File Server session
	RFs iFsSession;
	//File Handle for the ELF File
	RFile iSELFFile;
	//SELF File name
	TBufC<KMaxFileName> iSELFFileName;
	//stores the SELF Program Header
	TUint8* iSELFPHHeader;
	//Number of Program Header Entries
	TInt iPHEntries;
	//Program Header Offset
	TInt iPHOffset;
	//storing crashed thread id
	TInt64 iCrashedThreadId;
	//storing the thread Heap base address
	TUint32	iHeapBase;
	//thread heap size
	TUint32	iHeapSize;
	//thread name that crashed
	RBuf iThreadCrashed;
	//type of crash User/System
	TBool iSystemCrash;

	//stores the DHDR Buffers
	TUint8* iDHDRBuffer[MAXNUMBEROFDHDR];
	//actual SELF DHDR strcuture Sym32_dhdr
	Sym32_dhdr* idhdr[MAXNUMBEROFDHDR];

	//offset to the String section
	TInt iOffsetStringSection;
	//size of String section
	TInt iSizeofStringSection;

	enum TSELFSegment {EHeap, ETrace, ECode, EThread};

	//start and stop tick counts
	TInt iStartTick;
	TInt iStopTick;

	friend class CAsyncProcessCrash;

	};
#endif // __CPROCESSCRASHWRAPPER_H__
