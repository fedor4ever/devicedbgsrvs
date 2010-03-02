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

#include <e32base.h>

// User includes
#include "logging.h"
#include "dbgtrcportmgr.h"
#include "ostbaserouter.h"
#include "dbgtrcsrvsession.h"
#include "dbgtrcsrvserver.h"


// Type definitions

// Constants
#define TRK_SID 0x200170BB
// Enumerations

// Classes referenced

#ifdef EKA2
//Only include these in the secured platform 
#include "dbgtrccmdcodes.h"

const TUint KRangeCount = 2; 

const TInt KOpCodeRanges[KRangeCount] = 
{	
	KCapabilityCustomCheck, 
	EDbgTrcCmdCodeLast,
};


const TUint8 KElementsIndex[KRangeCount] =
{
	CPolicyServer::ECustomCheck, 	//Custom check for the Trk SID 	 0 		- ETrkTcbCmdCodeLast
	CPolicyServer::ENotSupported, 	//Not Supported					 		 ETrkTcbCmdCodeLast-End
};

const CPolicyServer::TPolicyElement KPolicyElements[] = 
{ 
	{_INIT_SECURITY_POLICY_S0(TRK_SID), CPolicyServer::EFailClient},
};

const CPolicyServer::TPolicy KDbgTrcServerPolicy =
{
	CPolicyServer::EAlwaysPass, //specifies all connect attempts should pass
	KRangeCount,
	KOpCodeRanges,
	KElementsIndex, 	// what each range is compared to 
	KPolicyElements 	// what policies range is compared to
};
 	
#endif


//
// CDbgTrcSrvServer (source)
//

//
// CDbgTrcSrvServer::CDbgTrcSrvServer()
//
// Constructor
//
CDbgTrcSrvServer::CDbgTrcSrvServer()
#ifdef  EKA2
 	:CPolicyServer(CActive::EPriorityHigh, KDbgTrcServerPolicy),
#else	
   	:CServer2(CActive::EPriorityHigh), 
#endif
   	iSessionCount(0),
   	iShutdown()
{
	LOG_MSG("CDbgTrcSrvServer::CDbgTrcSrvServer");

}

//
// CDbgTrcSrvServer::~CDbgTrcSrvServer()
//
// Destructor
//
CDbgTrcSrvServer::~CDbgTrcSrvServer()
{
	LOG_MSG("CDbgTrcSrvServer::~CDbgTrcSrvServer");

	// In order to prevent access violations as the server
	// shuts down, we inform all sessions that the server is
	// about to be destroyed
	iSessionIter.SetToFirst();
	//
	CDbgTrcSrvSession* session = static_cast<CDbgTrcSrvSession*>(iSessionIter++);
	while (session)
	{
		session->HandleServerDestruction();
		session = static_cast<CDbgTrcSrvSession*>(iSessionIter++);
	}
	
	SafeDelete(iOstRouter);	
}

//
// CDbgTrcSrvServer::ConstructL()
//
// Second level construction
//
void CDbgTrcSrvServer::ConstructL()
{
	LOG_MSG("CDbgTrcSrvServer::ConstructL");

	// Calling StartL here won't actually allow any connections
	// to be created until the active scheduler has a chance to run. However, the 
	// call below will leave should there already be a started TrkTcb server instance.
	//
	StartL(KDbgTrcServerName);
	iShutdown.ConstructL();
	// now create the ost router which will be the main worker object for all the sessions.
	iOstRouter = COstBaseRouter::NewL();
}

//
// CDbgTrcSrvServer::NewLC()
//
// Creates an instance of CDbgTrcSrvServer.
//
CDbgTrcSrvServer* CDbgTrcSrvServer::NewLC()
{
	LOG_MSG("CDbgTrcSrvServer::NewLC");

	CDbgTrcSrvServer* self = new(ELeave) CDbgTrcSrvServer();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

//
// CDbgTrcSrvServer::NewSessionL()
//
// Creates a new session.
// Called by the IPC framework everytime a client creates a new session.
// Returns a CDbgTrcSrvSession object.
//
CSession2* CDbgTrcSrvServer::NewSessionL(const TVersion& aVersion,const RMessage2& /*aMessage*/) const
{	
	LOG_MSG("CDbgTrcSrvServer::NewSessionL");

	// Check client version is correct
	const TVersion dbgTrcServerVersion(KDbgTrcServerMajorVN, KDbgTrcServerMinorVN, KDbgTrcServerBuildVN);
	if	(!User::QueryVersionSupported(dbgTrcServerVersion, aVersion))
		User::Leave(KErrNotSupported);
	
	// Create new session
	CDbgTrcSrvSession* session = CDbgTrcSrvSession::NewL(iOstRouter);
	return session;
}

//
// CDbgTrcSrvServer::RunError()
//
// Handles all the errors when handling a client request.
// Called by the IPC framework whenever a leave occurs when handling a client request.
// Returns KErrNone to complete the error handling.
//
TInt CDbgTrcSrvServer::RunError(TInt aError)
{
	LOG_MSG("CDbgTrcSrvServer::RunError");

	// A bad descriptor error implies a badly programmed client, so panic it;
	// otherwise report the error to the client
	if	(aError == KErrBadDescriptor)
	{
		Message().Panic(KServerIntiatedSessionPanic, EDbgTrcServerInitiatedClientPanicBadDescriptor);
	}
	else
	{
		Message().Complete(aError);
	}

	// The leave will result in an early return from CServer2::RunL(), skipping
	// the call to request another message. So do that now in order to keep the
	// server running.
	ReStart();

	// Indicate that we've handled the error fully
	return KErrNone;
}
	
#ifdef EKA2	
//
// CDbgTrcSrvServer::CustomSecurityCheckL()
//
// The only security check that is done is to check for the cleint securid 
// Returns EFail or EPass
//
CPolicyServer::TCustomResult CDbgTrcSrvServer::CustomSecurityCheckL(const RMessage2& aMsg, TInt& /*aAction*/, TSecurityInfo& /*aMissing*/)
{
	LOG_MSG("CDbgTrcSrvServer::CustomSecurityCheckL");

	CPolicyServer::TCustomResult returnValue = CPolicyServer::EPass;	

// Check for an appropriate capability, for now the plat sec check is disabled.
// We need to see if we really need to do or not.

/* 	
	TSecureId ClientSID = aMsg.SecureId();

	if ((KMetroTrkAppUid == ClientSID) || (KMetroTrkExeUid == ClientSID))
	{
		returnValue = CPolicyServer::EPass;
	}				
*/
	return(returnValue);
}
#endif

/**
Decrements the server's count of how many sessions are connected to it and
starts the shutdown timer if there are no sessions connected
*/
void CDbgTrcSrvServer::SessionClosed()
{
	if(--iSessionCount < 1)
	{
		iShutdown.Start();
	}
}

/**
Increments the servers count of how many sessions are connected to it and
cancels the shutdown timer if it is running
*/
void CDbgTrcSrvServer::SessionOpened()
{
	iSessionCount++;
	iShutdown.Cancel();
}
