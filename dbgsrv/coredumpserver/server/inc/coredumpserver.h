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
//



/**
 @file
 @internalTechnology
 @released
*/


#ifndef CORE_DUMP_SERVER_H
#define CORE_DUMP_SERVER_H


// INCLUDES

#include <e32cons.h>
#include <s32file.h>
#include <rm_debug_api.h>

#include <coredumpserverapi.h>


using namespace Debug;

// CONSTANTS

// Default heap size for server thread.
const TUint KCoreDumpServerDefaultHeapSize=0x10000;

// ENUMS

// Server panic codes
enum TCoreDumpServerPanic
{
	ESvrCreateServer,
	ESvrStartServer,
	ESvrFileServer,
	EMainSchedulerError,
	EBadRequest,
};

// CLASS DECLARATIONS


/**
Define core dump server class. 
@see RCoreDumpSession
@see RSecuritySvrSession
*/
class CCoreDumpServer : public CServer2
{
	public:
		static CCoreDumpServer* NewLC();
		static CCoreDumpServer* NewL();
		static void RunServerL();
		static void PanicServer(TCoreDumpServerPanic aPanic);	

	public:
		virtual CSession2* NewSessionL( const TVersion& aVersion, const RMessage2& aMessage) const;
		~CCoreDumpServer();

		void SessionOpened();
		void SessionClosed();
		TInt RunError(TInt aError);

	private:	// Construction
		CCoreDumpServer(TInt aPriority = EPriorityHigh);

    protected:
		void ConstructL();

	private:	// Data members

		TInt	iNumSessions;

	RSecuritySvrSession	iSecSess;
};

#endif	// CORE_DUMP_SERVER_H

