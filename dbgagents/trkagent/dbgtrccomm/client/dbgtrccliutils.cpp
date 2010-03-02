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


#include "dbgtrccliutils.h"

// System includes
#include <e32std.h>

// User includes
#include "dbgtrcclidefs.h"



//
// DbgTrcCliUtils (source)
//

//
// DbgTrcCliUtils::Panic()
//
// Panic the client
//
void DbgTrcCliUtils::Panic(TDbgTrcCliPanic aPanic)
{
	_LIT(KDbgTrcCliPanic, "DBGTRCCLI");
	User::Panic(KDbgTrcCliPanic, aPanic);
}

//
// DbgTrcCliUtils::Fault()
//
// Panic the client, indicating some form of logic error or terminal
// fault.
//
void DbgTrcCliUtils::Fault(TDbgTrcCliFault aFault)
{
	_LIT(KDbgTrcCliFault, "DBGTRCCLIFAULT");
	User::Panic(KDbgTrcCliFault, aFault);
}

//
// DbgTrcCliUtils::StartDbgTrcServer()
//
// Starts the DbgTrc server.
// @return KErrNone if successful, 
//		   KErrAlreadyExists if the server is already running, otherwise a system-wide error.
TInt DbgTrcCliUtils::StartDbgTrcServer()
{

	TInt ret = KErrNone;

	//
	// Create a new server process. Simultaneous launching
	// of two such processes should be detected when the second one attempts to
	// create the server object, failing with KErrAlreadyExists.
	//
	RProcess server;
	ret = server.Create(DbgTrcCliDefs::ServerImageName(), KNullDesC, DbgTrcCliDefs::ServerUid());

	// Did we manage to create the thread/process?
	if	(ret != KErrNone)
		return ret;

	// Wait to see if the thread/process died during construction
	TRequestStatus serverDiedRequestStatus;
#if defined(EKA2)
	server.Rendezvous(serverDiedRequestStatus);
#else
	server.Logon(serverDiedRequestStatus);
#endif
	
	if	(serverDiedRequestStatus != KRequestPending)
	{
		// Abort startup
		server.Kill(KErrNone);
	}
	else
	{
		// Logon OK - start the server
		server.Resume();
	}
	
	User::WaitForRequest(serverDiedRequestStatus);

	// we can't use the 'exit reason' if the server panicked as this
	// is the panic 'reason' and may be '0' which cannot be distinguished
	// from KErrNone
	
	ret = (server.ExitType() == EExitPanic) ? KErrGeneral : serverDiedRequestStatus.Int();
	
	server.Close();

	return ret;
}
