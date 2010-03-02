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

#include "trksrvserver.h"

// System includes
#include <e32base.h>
#include <e32debug.h>

// User includes
#include "trksrvsession.h"


// Type definitions

// Constants
#define METROTRK_SID 0x200170BB
// Enumerations

// Classes referenced

//Only include these in the secured platform 
#include "trksrvcmdcodes.h"

const TUint KRangeCount = 2; 

const TInt KOpCodeRanges[KRangeCount] = 
{	
	KCapabilityCustomCheck, 
	ETrkCmdCodeLast,
};


const TUint8 KElementsIndex[KRangeCount] =
{
	CPolicyServer::ECustomCheck, 	//Custom check for the MetroTrk SID 	 0 - ETrkCmdCodeLast
	CPolicyServer::ENotSupported, 	//Not Supported					 		 ETrkCmdCodeLast-End
};

const CPolicyServer::TPolicy KTrkServerPolicy =
{
	CPolicyServer::EAlwaysPass, //specifies all connect attempts should pass
	KRangeCount,
	KOpCodeRanges,
	KElementsIndex, 	// what each range is compared to 
	NULL 	// what policies range is compared to
};



//
// CTrkSrvServer (source)
//

//
// CTrkSrvServer::CTrkSrvServer()
//
// Constructor
//
CTrkSrvServer::CTrkSrvServer(CTrkDebugManager* aTrkDebugMgr)
#ifdef  EKA2
 	:CPolicyServer(CActive::EPriorityHigh, KTrkServerPolicy),
#else	
   	:CServer2(CActive::EPriorityHigh), 
#endif
	iTrkDebugMgr(aTrkDebugMgr)
{
}

//
// CTrkSrvServer::~CTrkSrvServer()
//
// Destructor
//
CTrkSrvServer::~CTrkSrvServer()
{
	// In order to prevent access violations as the server
	// shuts down, we inform all sessions that the server is
	// about to be destroyed
	iSessionIter.SetToFirst();
	//
	CTrkSrvSession* session = static_cast<CTrkSrvSession*>(iSessionIter++);
	while (session)
	{
		session->HandleServerDestruction();
		session = static_cast<CTrkSrvSession*>(iSessionIter++);
	}	
}

//
// CTrkSrvServer::ConstructL()
//
// Second level construction
//
void CTrkSrvServer::ConstructL()
{
	// Calling StartL here won't actually allow any connections
	// to be created until the active scheduler has a chance to run. However, the 
	// call below will leave should there already be a started Trk server instance.
	//
	StartL(KTrkServerName);
}

//
// CTrkSrvServer::NewLC()
//
// Creates an instance of CTrkSrvServer.
//
CTrkSrvServer* CTrkSrvServer::NewLC(CTrkDebugManager* aTrkDebugMgr)
{
	CTrkSrvServer* self = new(ELeave) CTrkSrvServer(aTrkDebugMgr);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

//
// CTrkSrvServer::NewSessionL()
//
// Creates a new session.
// Called by the IPC framework everytime a client creates a new session.
// Returns a CTrkSrvSession object.
//
CSession2* CTrkSrvServer::NewSessionL(const TVersion& aVersion,const RMessage2& /*aMessage*/) const
{	
	// Check client version is correct
	const TVersion trkServerVersion(KTrkServerMajorVN, KTrkServerMinorVN, KTrkServerBuildVN);
	if	(!User::QueryVersionSupported(trkServerVersion, aVersion))
		User::Leave(KErrNotSupported);
	
	// Create new session
	CTrkSrvSession* session = CTrkSrvSession::NewL(iTrkDebugMgr);
	return session;
}

//
// CTrkSrvServer::RunError()
//
// Handles all the errors when handling a client request.
// Called by the IPC framework whenever a leave occurs when handling a client request.
// Returns KErrNone to complete the error handling.
//
TInt CTrkSrvServer::RunError(TInt aError)
{
	// A bad descriptor error implies a badly programmed client, so panic it;
	// otherwise report the error to the client
	if	(aError == KErrBadDescriptor)
	{
		Message().Panic(KServerIntiatedSessionPanic, ETrkServerInitiatedClientPanicBadDescriptor);
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
// CTrkSrvServer::CustomSecurityCheckL()
//
// The only security check that is done is to check for the cleint securid 
// Returns EFail or EPass
//
CPolicyServer::TCustomResult CTrkSrvServer::CustomSecurityCheckL(const RMessage2& aMsg, TInt& /*aAction*/, TSecurityInfo& /*aMissing*/)
{
	CPolicyServer::TCustomResult returnValue = CPolicyServer::EFail;	
	
	TSecureId ClientSID = aMsg.SecureId();
	// We only permit MetroTrk to use this server. So we check
	// for MetroTrk's SIDs. If its not MetroTrk's SIDs, then the security check fails.
	if ((KTrkAppUid == ClientSID) || (KTrkExeUid == ClientSID))
	{
		returnValue = CPolicyServer::EPass;
	}			
	return(returnValue);
}
#endif
