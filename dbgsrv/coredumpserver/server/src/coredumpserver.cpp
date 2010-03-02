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
// core_dump_server.cpp
//

// System includes
#include <e32cmn.h>
#include <e32std.h>
#include <e32svr.h>

// Project includes
#include "coredumpserver.h"
#include "coredumpsession.h"
#include "flashdatasource.h"

/**
This is the version of the security server that we have developed our code against
and it comes from the interface definition of the DSS at the time of compilation.
*/
const TVersion securityServerVersion( 
				KDebugServMajorVersionNumber, 
				KDebugServMinorVersionNumber, 
				KDebugServPatchVersionNumber );

/*
* Called to panic the server (in event of partial construction or serious error)
*/
void CCoreDumpServer::PanicServer(TCoreDumpServerPanic aPanic)
	{
        //are we using this??
	User::Panic(KCoreDumpServerName, aPanic);
	}
	
CCoreDumpServer* CCoreDumpServer::NewL()
{
	CCoreDumpServer* self = CCoreDumpServer::NewLC();
    CleanupStack::Pop();
    return self;
}
	
/*
* Symbian OS 1st stage construction
*/
CCoreDumpServer* CCoreDumpServer::NewLC()
	{
	//RDebug::Print( _L("CCoreDumpServer* CCoreDumpServer::NewLC()\n"));
	CCoreDumpServer* self = new (ELeave) CCoreDumpServer();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}
	
/*
* Destructor
*/
CCoreDumpServer::~CCoreDumpServer()
	{
    //LOG_MSG("->CCoreDumpServer::~CCoreDumpServer\n");
    iSecSess.Close();
	}

/*
* Called when a new session is to be created
*/
CSession2* CCoreDumpServer::NewSessionL( const TVersion& aVersion, 
									   const RMessage2& /*aMessage*/ ) const
	{

	//RDebug::Printf( "->CCoreDumpServer::NewSessionL()\n" );
    
    RSecuritySvrSession &session = const_cast<RSecuritySvrSession&>(iSecSess);
    
	THandleInfo dssSessionInfo;
	session.HandleInfo( & dssSessionInfo );

	if( dssSessionInfo.iNumOpenInProcess > 1 )
		{
        //if there is already a session with DSS this means that we started it already for someone else
		RDebug::Print( _L("CCoreDumpServer::NewSessionL() : Error : Session to DSS already in use by CDS\n"));
		User::Leave( KErrAlreadyExists );
		}

	// Check the client-side API version number against the server version number.
	TVersion serverVersion(	KCoreDumpServMajorVersionNumber,
							KCoreDumpServMinorVersionNumber,
							KCoreDumpServBuildVersionNumber);

	LOG_MSG4( "CCoreDumpServer::NewSessionL() : Server version: major=%d,minor=%d,build=%d\n",
		KCoreDumpServMajorVersionNumber, KCoreDumpServMinorVersionNumber, KCoreDumpServBuildVersionNumber );

	LOG_MSG4( "  Client API built against : major=%d,minor=%d,build=%d\n", 
		aVersion.iBuild, aVersion.iMajor, aVersion.iMinor );

	if( !User::QueryVersionSupported( serverVersion, aVersion ) )
		{
		// This server version is incompatible with the version of the server the 
		// client-side API was built against
		RDebug::Printf( "CCoreDumpServer::NewSessionL() : Leaving due to incompatible versions\n" );
		User::Leave( KErrNotSupported );
		}

	//LOG_MSG("CCoreDumpServer::NewSessionL - creating new core dump session\n");
	return CCoreDumpSession::NewL(session);
	}

/*
* Called when a session is opened
*/
void CCoreDumpServer::SessionOpened()
	{
	//LOG_MSG("->CCoreDumpServer::SessionOpened()\n");
	++iNumSessions;
	}


/*
* Called when a session is closed 
*/
void CCoreDumpServer::SessionClosed()
	{
	//LOG_MSG("->CCoreDumpServer::SessionClosed()\n");
	if(--iNumSessions < 1) // No more sessions connected to the server, shut down the server
		{
	    //LOG_MSG("CCoreDumpServer::SessionClosed() -> CActiveScheduler::Stop()\n");
		CActiveScheduler::Stop();
		}
	}
   

/*
* Called when the server active object's RunL() leaves
*/
TInt CCoreDumpServer::RunError(TInt aError)
	{
	Message().Complete(aError);
    
	// Call Restart(), as RunL() left before server got chance to re-issue its request
	ReStart();

	return KErrNone;
	}

// PRIVATE MEMBER FUNCTIONS

/*
* C++ Constructor
*/
CCoreDumpServer::CCoreDumpServer( TInt aPriority )
	: CServer2( aPriority ), 
	  iNumSessions( 0 )
	{
	// No implementation required
	}
	

/*
* Symbian OS 2nd stage construction. Called from CCoreDumpServer::NewLC()
*/	
void CCoreDumpServer::ConstructL()
	{
	//LOG_MSG("->CCoreDumpServer::ConstructL()\n" );

	TInt err = iSecSess.Connect( securityServerVersion );

	if( KErrNone != err )
		{
		_LIT( KDssStartErrorMsg ,"Core Dump Server: Unable to Start/Connect to Debug Security Server");
		User::InfoPrint( KDssStartErrorMsg );

		LOG_MSG2("CCoreDumpServer::ConstructL() - unable to open security server session! err:%d\n", err);
		User::Leave(err);
		}

    err = iSecSess.ShareAuto();
    
	StartL( KCoreDumpServerName );
	} 
