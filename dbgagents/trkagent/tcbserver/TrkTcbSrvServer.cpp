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

#include "TrkTcbSrvServer.h"

// System includes
#include <e32base.h>
#include <e32debug.h>

// User includes
#include "TrkTcbSrvSession.h"
//#include "TrkTcbSrvStaticUtils.h"


// Type definitions

// Constants
#define TRK_SID 0x200170BB
// Enumerations

// Classes referenced

#ifdef EKA2
//Only include these in the secured platform 
#include "TrkTcbCmdCodes.h"

const TUint KRangeCount = 2; 

const TInt KOpCodeRanges[KRangeCount] = 
{	
	KCapabilityCustomCheck, 
	ETrkTcbCmdCodeLast,
};


const TUint8 KElementsIndex[KRangeCount] =
{
	CPolicyServer::ECustomCheck, 	//Custom check for the Trk SID 	 0 - ETrkTcbCmdCodeLast
	CPolicyServer::ENotSupported, 	//Not Supported					 		 ETrkTcbCmdCodeLast-End
};

const CPolicyServer::TPolicy KTrkTcbServerPolicy =
{
	CPolicyServer::EAlwaysPass, //specifies all connect attempts should pass
	KRangeCount,
	KOpCodeRanges,
	KElementsIndex, 	// what each range is compared to 
	NULL, //KPolicyElements 	// what policies range is compared to
};
 	
#endif


//
// CTrkTcbSrvServer (source)
//

//
// CTrkTcbSrvServer::CTrkTcbSrvServer()
//
// Constructor
//
CTrkTcbSrvServer::CTrkTcbSrvServer()
#ifdef  EKA2
 	:CPolicyServer(CActive::EPriorityHigh, KTrkTcbServerPolicy)
#else	
   	:CServer2(CActive::EPriorityHigh) 
#endif
{
}

//
// CTrkTcbSrvServer::~CTrkTcbSrvServer()
//
// Destructor
//
CTrkTcbSrvServer::~CTrkTcbSrvServer()
{
	// In order to prevent access violations as the server
	// shuts down, we inform all sessions that the server is
	// about to be destroyed
	iSessionIter.SetToFirst();
	//
	CTrkTcbSrvSession* session = static_cast<CTrkTcbSrvSession*>(iSessionIter++);
	while (session)
	{
		session->HandleServerDestruction();
		session = static_cast<CTrkTcbSrvSession*>(iSessionIter++);
	}
	
}

//
// CTrkTcbSrvServer::ConstructL()
//
// Second level construction
//
void CTrkTcbSrvServer::ConstructL()
{
	// Calling StartL here won't actually allow any connections
	// to be created until the active scheduler has a chance to run. However, the 
	// call below will leave should there already be a started TrkTcb server instance.
	//
	StartL(KTrkTcbServerName);
}

//
// CTrkTcbSrvServer::NewLC()
//
// Creates an instance of CTrkTcbSrvServer.
//
CTrkTcbSrvServer* CTrkTcbSrvServer::NewLC()
{
	CTrkTcbSrvServer* self = new(ELeave) CTrkTcbSrvServer();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

//
// CTrkTcbSrvServer::NewSessionL()
//
// Creates a new session.
// Called by the IPC framework everytime a client creates a new session.
// Returns a CTrkTcbSrvSession object.
//
CSession2* CTrkTcbSrvServer::NewSessionL(const TVersion& aVersion,const RMessage2& /*aMessage*/) const
{	
	// Check client version is correct
	const TVersion trkTcbServerVersion(KTrkTcbServerMajorVN, KTrkTcbServerMinorVN, KTrkTcbServerBuildVN);
	if	(!User::QueryVersionSupported(trkTcbServerVersion, aVersion))
		User::Leave(KErrNotSupported);
	
	// Create new session
	CTrkTcbSrvSession* session = CTrkTcbSrvSession::NewL();
	return session;
}

//
// CTrkTcbSrvServer::RunError()
//
// Handles all the errors when handling a client request.
// Called by the IPC framework whenever a leave occurs when handling a client request.
// Returns KErrNone to complete the error handling.
//
TInt CTrkTcbSrvServer::RunError(TInt aError)
{
	// A bad descriptor error implies a badly programmed client, so panic it;
	// otherwise report the error to the client
	if	(aError == KErrBadDescriptor)
	{
		Message().Panic(KServerIntiatedSessionPanic, ETrkTcbServerInitiatedClientPanicBadDescriptor);
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
// CTrkTcbSrvServer::CustomSecurityCheckL()
//
// The only security check that is done is to check for the cleint securid 
// Returns EFail or EPass
//
CPolicyServer::TCustomResult CTrkTcbSrvServer::CustomSecurityCheckL(const RMessage2& aMsg, TInt& /*aAction*/, TSecurityInfo& /*aMissing*/)
{
	CPolicyServer::TCustomResult returnValue = CPolicyServer::EFail;	
	
	TSecureId ClientSID = aMsg.SecureId();
	// We only permit Trk to use this server. So we check
	// for Trk's SIDs. If its not Trk's SIDs, then the security check fails.
	if ((KTrkSrvUid == ClientSID) || (KTrkAppUid == ClientSID) || (KTrkExeUid == ClientSID))
	{
		returnValue = CPolicyServer::EPass;
	}			
	return(returnValue);
}
#endif
