/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <e32debug.h>

// system includes
#include <e32std.h>
#include <eikenv.h>

// local includes
#include "trkdummyrecognizer.h"
#ifdef __UI_FRAMEWORKS_V2__
#include <implementationproxy.h>
#endif //__UI_FRAMEWORKS_V2__

// constants
_LIT(KTrkConsoleExe, "trkconsole.exe");

const TUid KTrkConsoleExeUid={0x200159E2}; // 


TBool IsAlreadyRunning()
{
	RDebug::Print(_L("TrkLauncher::IsAlreadyRunning()"));

	_LIT(KTrkConsoleSearchPattern, "*TRKPROCESS*");
	_LIT(KTrkConsoleProcessPattern, "*");
	
	TFindProcess finder;
	TFullName fullName;
	TBool found = EFalse;
	finder.Find(KTrkConsoleProcessPattern);

	while (!found && finder.Next(fullName) == KErrNone)
	{
		fullName.UpperCase();
		
		if (fullName.Match(KTrkConsoleSearchPattern) != KErrNotFound)
		{
			found = ETrue;
			RDebug::Print(_L("process found Inside while"));
			break;
		}
	}

	if (found)
	{
		RDebug::Print(_L("TrkLauncher - Process found outside while"));
	}
	else
	{
		RDebug::Print(_L("TrkLauncher - Process was never found"));
	}
		
	return found;
}

void HandleStart()
{
	RDebug::Print(_L("TrkLauncher::HandleStart()"));
	
	if (!IsAlreadyRunning())
	{		
		TInt result;		
		RProcess p;
		
		//start TrkConsole	process
		result = p.Create(KTrkConsoleExe, TPtr(NULL, 0), TUidType(KNullUid, KNullUid, KTrkConsoleExeUid));
		if (result == KErrNone)
		{
			RDebug::Print(_L("TrkLauncher - Trk process resume"));
			p.Resume();
			RDebug::Print(_L("TrkLauncher - After Trk process resume"));
			p.Close();
			RDebug::Print(_L("TrkLauncher - After Trk process close"));
		}
		else
		{
			RDebug::Print(_L("TrkLauncher - Trk process not created with %d \n"),result);
			RDebug::Print(_L("TrkLauncher - Returning without process creation"));
		}
			
	}
}
	
#ifdef __UI_FRAMEWORKS_V2__

const TImplementationProxy ImplementationTable[]=
{
    IMPLEMENTATION_PROXY_ENTRY(0x1020814D, CTrkDummyRecognizer::NewL)
};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
{
    RDebug::Print(_L("TrkLauncher::ImplementationGroupProxy(aTableCount=%d)"), aTableCount);
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
}

CApaDataRecognizerType* CTrkDummyRecognizer::NewL()
{
	RDebug::Print(_L("TrkLauncher::CTrkDummyRecognizer::NewL()"));
    CApaDataRecognizerType *recog = new CTrkDummyRecognizer(); // no (ELeave) as return NULL is OK
   
    RDebug::Print(_L("TrkLauncher - Before HandleSart()"));
    HandleStart(); 
    
    return recog;
}

#else
	
EXPORT_C CApaDataRecognizerType* CreateRecognizer()
{
	CApaDataRecognizerType* recog = new CTrkDummyRecognizer(); // no (ELeave) as return NULL is OK

	HandleStart();

	return recog;
}
#endif //__UI_FRAMEWORKS_V2__

#ifndef EKA2

GLDEF_C TInt E32Dll(TDllReason /*aReason*/)
{
	RDebug::Print(_L("MCLauncher::E32Dll()"));
	return KErrNone;
}
#endif // EKA2
