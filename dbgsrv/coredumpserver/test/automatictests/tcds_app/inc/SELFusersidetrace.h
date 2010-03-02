/**
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @file SELFusersidetrace.h
 @internalTechnology
*/
#if (!defined __SELF_USER_SIDE_TRACE_H__)
#define ___SELF_USER_SIDE_TRACE_H__
#include <TestExecuteStepBase.h>
#include <symbianelfdefs.h>
#include <bafindf.h>
#include <crashdefs.h>

#include "tcoredumpserversuitestepbase.h"
#include "tcoredumpcommon.h"


_LIT(KSELFUserSideTrace,"SELFUserSideTrace");

//Maximum Number of DHDR expected in the SELF File
const TInt MAXNUMBEROFDHDR = 200;

_LIT( KCreateTraceData, "How much Trace data to capture (Kb)" );


class CSELFUserSideTrace : public CTe_coredumpserverSuiteStepBase
	{
public:
	CSELFUserSideTrace();
	~CSELFUserSideTrace();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepL();
	virtual TVerdict doTestStepPostambleL();

// Please add/modify your class members here:
private:

	//session object to the Core Dump Server
	RCoreDumpSession iCoreDumpSession;
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
	//stores the DHDR Buffers
	TUint8* iDHDRBuffer[MAXNUMBEROFDHDR];
	//actual SELF DHDR strcuture Sym32_dhdr
	Sym32_dhdr* idhdr[MAXNUMBEROFDHDR];
	//offset to the String section
	TInt iOffsetStringSection;
	//size of String section
	TInt iSizeofStringSection;
	//flag to determine whether there is trace or not
	TBool iTraceFlag;
	
	void ClientAppL();
	
	//Tests
	void HandleCrashL();
	
	void DoTestTraceSectionSELFFileL(TBool );
	
	void ValidateHeaderELFFileL();
	
	void ValidateProgramHeaderELFFileL();
	
	void ValidateTraceSectionELFFileL(TBool );
	
	void ValidateTraceBufferL(const TDesC8&  );

	
	//Utility functions
	
	void DoCmdLoadPluginsL();
	
	void DoCmdConfigurePluginsL( );
	
	void ConfigureSELF(TBool );

	void MonitorProgressL();
	
    void DoConfigureL( const TUint32& aIndex, 
					  const TUint32& aUID, 
					  const COptionConfig::TParameterSource& aSource, 
					  const COptionConfig::TOptionType& aType, 
					  const TDesC& aPrompt, 
					  const TUint32& aNumOptions,
					  const TDesC& aOptions,
					  const TInt32& aVal, 
					  const TDesC& aStrValue,
                      const TUint aInstance );
    
    void CleanupMethod(const TDesC& );


	};



#endif
