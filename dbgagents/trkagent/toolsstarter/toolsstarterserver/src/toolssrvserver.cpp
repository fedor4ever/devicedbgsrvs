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

#include "toolssrvserver.h"

// System includes
#include <e32base.h>

// User includes
#include "toolssrvsession.h"



// Type definitions

// Constants
#define METROTRK_SID 0x200170BB
// Enumerations

// Classes referenced

#ifdef EKA2
//Only include these in the secured platform 
#include "ToolsCmdCodes.h"

const TUint KRangeCount = 2; 

const TInt KOpCodeRanges[KRangeCount] = 
{	
	KCapabilityCustomCheck, 
	EToolsCmdCodeLast,
};


const TUint8 KElementsIndex[KRangeCount] =
{
	CPolicyServer::ECustomCheck, 	//Custom check for the MetroTrk SID 	 0 - EToolsCmdCodeLast
	CPolicyServer::ENotSupported, 	//Not Supported					 		 EToolsCmdCodeLast-End
};

const CPolicyServer::TPolicyElement KPolicyElements[] = 
{ 
	{_INIT_SECURITY_POLICY_S0(METROTRK_SID), CPolicyServer::EFailClient},
};

const CPolicyServer::TPolicy KToolsServerPolicy =
{
	CPolicyServer::EAlwaysPass, //specifies all connect attempts should pass
	KRangeCount,
	KOpCodeRanges,
	KElementsIndex, 	// what each range is compared to 
	KPolicyElements 	// what policies range is compared to
};
 	
#endif


//
// CToolsSrvServer (source)
//

//
// CToolsSrvServer::CToolsSrvServer()
//
// Constructor
//
CToolsSrvServer::CToolsSrvServer()
#ifdef  EKA2
 	:CPolicyServer(CActive::EPriorityHigh, KToolsServerPolicy),
#else	
   	:CServer2(CActive::EPriorityHigh),
#endif
iSessionCount(0)
{
}

//
// CToolsSrvServer::~CToolsSrvServer()
//
// Destructor
//
CToolsSrvServer::~CToolsSrvServer()
{
	// In order to prevent access violations as the server
	// shuts down, we inform all sessions that the server is
	// about to be destroyed
	iSessionIter.SetToFirst();
	//
	CToolsSrvSession* session = static_cast<CToolsSrvSession*>(iSessionIter++);
	while (session)
	{
		session->HandleServerDestruction();
		session = static_cast<CToolsSrvSession*>(iSessionIter++);
	}
	if(iLaunchManager)
	    delete iLaunchManager;
	
}

//
// CToolsSrvServer::ConstructL()
//
// Second level construction
//
void CToolsSrvServer::ConstructL()
{
	// Calling StartL here won't actually allow any connections
	// to be created until the active scheduler has a chance to run. However, the 
	// call below will leave should there already be a started Tools server instance.
	//
	StartL(KToolsServerName);
	//Launching the tools
	iLaunchManager = CToolsLaunchMgr::NewL();
    if (iLaunchManager)
        iLaunchManager->Launch();
}

//
// CToolsSrvServer::NewLC()
//
// Creates an instance of CToolsSrvServer.
//
CToolsSrvServer* CToolsSrvServer::NewLC()
{
	CToolsSrvServer* self = new(ELeave) CToolsSrvServer();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

//
// CToolsSrvServer::NewSessionL()
//
// Creates a new session.
// Called by the IPC framework everytime a client creates a new session.
// Returns a CToolsSrvSession object.
//
CSession2* CToolsSrvServer::NewSessionL(const TVersion& aVersion,const RMessage2& /*aMessage*/) const
{	
	// Check client version is correct
	const TVersion ToolsServerVersion(KToolsServerMajorVN, KToolsServerMinorVN, KToolsServerBuildVN);
	if	(!User::QueryVersionSupported(ToolsServerVersion, aVersion))
		User::Leave(KErrNotSupported);
	
	// Create new session
	CToolsSrvSession* session = CToolsSrvSession::NewL();
	return session;
}

//
// CToolsSrvServer::RunError()
//
// Handles all the errors when handling a client request.
// Called by the IPC framework whenever a leave occurs when handling a client request.
// Returns KErrNone to complete the error handling.
//
TInt CToolsSrvServer::RunError(TInt aError)
{
	// A bad descriptor error implies a badly programmed client, so panic it;
	// otherwise report the error to the client
	if	(aError == KErrBadDescriptor)
	{
		Message().Panic(KServerIntiatedSessionPanic, EToolsServerInitiatedClientPanicBadDescriptor);
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
// CToolsSrvServer::CustomSecurityCheckL()
//
// The only security check that is done is to check for the cleint securid 
// Returns EFail or EPass
//
CPolicyServer::TCustomResult CToolsSrvServer::CustomSecurityCheckL(const RMessage2& aMsg, TInt& /*aAction*/, TSecurityInfo& /*aMissing*/)
{
	CPolicyServer::TCustomResult returnValue = CPolicyServer::EFail;	
	
	TSecureId ClientSID = aMsg.SecureId();
	// We only permit MetroTrk to use this server. So we check
	// for MetroTrk's SIDs. If its not MetroTrk's SIDs, then the security check fails.
	if ((KTrkServerUid == ClientSID) || (KTraceServerUid == ClientSID) || (KToolsClientUid == ClientSID))
	{
		returnValue = CPolicyServer::EPass;
	}			
	return(returnValue);
}
#endif


/**
Decrements the server's count of how many sessions are connected to it and
starts the shutdown timer if there are no sessions connected
*/
void CToolsSrvServer::SessionClosed()
{
    if(--iSessionCount < 1)
    {
        
    }
}

/**
Increments the servers count of how many sessions are connected to it and
cancels the shutdown timer if it is running
*/
void CToolsSrvServer::SessionOpened()
{
    iSessionCount++;
    
}
/*
 * To get the USB connection status
 * 
 */
TConnectionStatus CToolsSrvServer::GetUsbConnStatus()
{
   return(iLaunchManager->GetUsbConnStatus());
    
}
