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
// Tests the Core Dump Server and the Debug Security Server
// by debugging the same executable. This is not possible but should fail
// gracefully. This is done as follows:
// 1 Start a client of the DSS called CRunModeAgent
// 2 Start a test debug application (crashapp.exe)
// 3 Actively attach to crashapp.exe
// 4 Start coredumpscript.exe, which starts the Core Dump Server 
// and then loads the saved config. If the configuration has been set
// to observe the crashapp.exe, the DSS should not allow this.
//

#include <e32base.h>
#include <e32cons.h>
#include <e32debug.h>
#include <rm_debug_api.h>


class CRunModeAgent : public CBase
	{
public:
	static CRunModeAgent* NewL();
	~CRunModeAgent();
	void ClientAppL();

private:
	CRunModeAgent();
	void ConstructL();
	void AttachToDSS();
	void SetupAndAttachToDSS();

private:
	Debug::RSecuritySvrSession iServSession;
	};



using namespace Debug;

/**
This is the version of the security server that we have developed our code against
and it comes from the interface definition of the DSS at the time of compilation.
*/
const TVersion securityServerVersion( 
				KDebugServMajorVersionNumber, 
				KDebugServMinorVersionNumber, 
				KDebugServPatchVersionNumber );

CRunModeAgent::CRunModeAgent()
//
// CRunModeAgent constructor
//
	{
	}

CRunModeAgent* CRunModeAgent::NewL()
//
// CRunModeAgent::NewL
//
	{
	CRunModeAgent* self = new(ELeave) CRunModeAgent();

  	self->ConstructL();

	return self;
	}

CRunModeAgent::~CRunModeAgent()
//
// CRunModeAgent destructor
//
	{
	iServSession.Close();
	}


void CRunModeAgent::ConstructL()
//
// CRunModeAgent::ConstructL
//
	{
	// nothing to do here
	}


void CRunModeAgent::SetupAndAttachToDSS()
	{
	TInt err = iServSession.Connect(securityServerVersion);
	if (err != KErrNone)
		{
		User::Panic(_L("Can't open server session"), err);
		}
	}


_LIT( KCrashAppFileName,"z:\\sys\\bin\\crashapp.exe");
_LIT( KCoreDumpServerClientProc,"z:\\sys\\bin\\coredumpscript.exe");


void CRunModeAgent::ClientAppL()
	{

	RDebug::Printf( "CRunModeAgent::ClientAppL\n" );
	
	SetupAndAttachToDSS();


	RProcess iCrashProcess;
	RDebug::Printf( "Creating crashing application\n" );
	TInt err = iCrashProcess.Create( KCrashAppFileName, KNullDesC );
	RDebug::Printf( "  iCrashProcess.Create( KCrashAppFileName ) returned %d\n", err  );
	User::After( 2000000 );
	iCrashProcess.Resume();
	
	//attach to process actively
	err = iServSession.AttachExecutable( KCrashAppFileName, EFalse );
	RDebug::Printf( "  iServSession.AttachExecutable( KCrashAppFileName, EFalse ) returned %d\n", err  );
	User::After( 2000000 );

	RProcess iCoreDumpServerClientProc;
	RDebug::Printf( "Creating Core Dump Server Client Proc\n" );
	err = iCoreDumpServerClientProc.Create( KCoreDumpServerClientProc, KNullDesC );
	RDebug::Printf( "  iCoreDumpServerClientProc.Create( KCoreDumpServerClientProc ) returned %d\n", err  );
	User::After( 2000000 );
	iCoreDumpServerClientProc.Resume();
	User::After( 5000000 );
	
	iServSession.DetachExecutable( KCrashAppFileName );

	}



GLDEF_C TInt E32Main()
	{
	TInt ret = KErrNone;

	
	CTrapCleanup* trap = CTrapCleanup::New();
	if (!trap)
		return KErrNoMemory;

   	
	CRunModeAgent *RunModeAgent = NULL;
	TRAPD(err, RunModeAgent = CRunModeAgent::NewL());
	if (RunModeAgent != NULL && err == KErrNone)
		{
        __UHEAP_MARK;
	    TRAP(ret,RunModeAgent->ClientAppL());
	    __UHEAP_MARKEND;

	    delete RunModeAgent;
		}
       
	delete trap;

	return ret;
	}
