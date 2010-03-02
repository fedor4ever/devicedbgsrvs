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

#ifndef __FLASHDUMP_H__
#define __FLASHDUMP_H__

#include <e32base.h>
#include <e32std.h>
#include <e32cons.h>
#include <rm_debug_api.h>
#include <f32file.h>

using namespace Debug;

/**
This is the version of the security server that we have developed our code against
and it comes from the interface definition of the DSS at the time of compilation.
*/
const TVersion KSecurityServerVersion( 
				KDebugServMajorVersionNumber, 
				KDebugServMinorVersionNumber, 
				KDebugServPatchVersionNumber );

//We have 1 meg on a h4
const TUint KCrashFlashPartitionSize = 0x100000;

class CCrashFlashDump : public CBase
	{
	public:
		static CCrashFlashDump* NewL();
		static CCrashFlashDump* NewLC();
		virtual ~CCrashFlashDump();
		
		void OpenFileL(const TDesC& aFileName);
		void DumpFlashToFileL();
		
	private:
		void ConstructL();
		CCrashFlashDump();
		
	private:
		
		RSecuritySvrSession iSecSess;
		RFs iFs;
		RFile iFile;
	};



#endif  // __FLASHDUMP_H__

