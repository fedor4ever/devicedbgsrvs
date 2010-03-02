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



/**
 @file
 @internalTechnology
*/

#ifndef __COREDUMP_MONITOR_H__
#define __COREDUMP_MONITOR_H__

#include <datawrapper.h>
#include <e32base.h> 
#include <rm_debug_api.h>
#include <coredumpinterface.h>
#include <coredumpserverapi.h>
#include <crashdatasave.h>
#include <bafindf.h> 
#include <scmconfigitem.h>

#include "t_common_defs.h"

/**
 * This class is responsible for the core dump monitor tests. This is an extension
 * of the SCM and the only way we can access and test it
 */
class CCoreDumpMonitorWrapper : public CDataWrapper
	{
	public:
	
		~CCoreDumpMonitorWrapper();
	
		static CCoreDumpMonitorWrapper* NewL();
	
		virtual TBool DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);
		virtual TAny* GetObject() { }
		virtual void SetObjectL(TAny* aObject);
	
	protected:
		void ConstructL();
	
	private:
		
		CCoreDumpMonitorWrapper();
	
		void PrepareTestL();
		void TestUntrustedAccessToCDSConfigL();
		void TestTrustedAccessToCDSConfigL();
		void TestTraceBufferAccessL();
		void TestReadDefaultConfigValuesL();
		void DoConfigureL(const TUint32& aIndex, const TUint32& aUID,
				const COptionConfig::TParameterSource& aSource,
				const COptionConfig::TOptionType& aType, const TDesC& aPrompt,
				const TUint32& aNumOptions, const TDesC& aOptions,
				const TInt32& aVal, const TDesC& aStrValue, const TUint aInstance);
		
		
	    void TestValidateNewestCrashL();
	    
	private:
		
		RCoreDumpSession iCoreDumpSession;
		RCrashInfoPointerList iCrashList;
		RFs iFsSession;
	};
#endif __COREDUMP_MONITOR_H__

//eof

