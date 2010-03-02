// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
//  Utility functions to allow us to configure the System Crash Monitor

/**
 @file
 @internalTechnology
*/

#ifndef __SELF_WRAPPER_H__
#define __SELF_WRAPPER_H__

#include <datawrapper.h>
#include <e32base.h>
#include <coredumpinterface.h>
#include <optionconfig.h>
#include <scmconfigitem.h>
#include <symbianelfdefs.h>

#include "t_common_defs.h"

//ini file keys
_LIT(KKeyCrashSource, "source");
_LIT(KKeyCrashId, "crashid");
_LIT(KKeyCrashName, "filename");
_LIT(KKeyElfSection, "section");

//commands
_LIT(KCommandGenerateFile, "GenerateSelfFile");
_LIT(KCommandFindSection, "VerifySectionExists");
_LIT(KCommandVerifyNoSection, "VerifySectionDoesNotExist");

//sources
_LIT(KSourceSystemCrash, "systemcrash");

//sections
_LIT(KSectionsVariantData, "variantdata");

/**
 * Can only use in void functions
 * @param x Boolean
 * @param If we fail, pop and destroy y items
 */
#define T_SELF_ASSERT_TRUE(x, y) \
	if(!x){\
		ERR_PRINTF2(_L("Assertion Error in t_self.cpp line [%d]"), __LINE__);\
		SetBlockResult(EFail);\
		if(y>0)\
			CleanupStack::PopAndDestroy(y);\
		return; \
		}

/**
 * Use for TUint returning functions
 * @param x Boolean
 * @param If we fail, pop and destroy y items
 */
#define T_SELF_ASSERT_TRUE_RET(x, y, z) \
	if(!x){\
		ERR_PRINTF2(_L("Assertion Error in t_self.cpp line [%d]"), __LINE__);\
		SetBlockResult(EFail);\
		if(y>0)\
			CleanupStack::PopAndDestroy(y);\
		return z; \
		}
/**
 * This class is the test wrapper we use to configure the System Crash
 * Monitor
 */
class CSymbianElfWrapper : public CDataWrapper
	{
	public:
		~CSymbianElfWrapper();
		static CSymbianElfWrapper* NewL();

		virtual TBool DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);
		virtual TAny* GetObject() { return NULL; }
		virtual void SetObject(TAny* aObject) { (void)aObject; }

	protected:
		void ConstructL();

	private:
		CSymbianElfWrapper();
		void GenerateFileFromSystemCrashL(TDesC& aCrashName, TInt aCrashId);

		TUint GetSectionOffset(RFile& aElfFile, ESYM_NOTE_TYPE aNoteType);
		TUint GetSectionSize(RFile& aElfFile, ESYM_NOTE_TYPE aNoteType);
		void CacheElfHeader(RFile& aElfFile);

		void VerifySection(TDesC& aSectionToVerify, TDesC& aCrashName, TBool aExists);
		void ValidateElfFile(RFile& aElfFile);
		void VerifyVariantData(RFile& aElfFile, TBool aExists);

	private:
		RCoreDumpSession iCoreDumpSession;
		RFs iFsSession;
		Elf32_Ehdr iElfHeader;
	};

#endif __SCM_CONFIG_H__

//eof
