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

#include <e32base.h>
#include "coredumpserver.h"
#include <debuglogging.h>

_LIT(KWildCard, "*");

/**
Checks whether another instance of the CDS exists in the system. It is checked
that:
    * the secure id of another process is KSecurityServerSecureID??
    * the PID is different to this process
    * the FileName of the other process contains KCoreDumpServerName

The final check is only made for the benefit of hardware boards, in a 
production device, the first two checks would be sufficient.

@return ETrue if the CDS is judged to be already running, EFalse otherwise
*/
TBool CDSAlreadyExists()
	{
	//find PID of this process
	RProcess process;
	TProcessId thisProcessId = process.Id();

	TFindProcess find(KWildCard);
	TFullName name;

	//try and find another CDS based on SecureID value??, which has a different PID
	while(find.Next(name)==KErrNone)
		{
		if (process.Open(find) == KErrNone)
			{
			//if(process.SecureId() == KSecurityServerSecureID) //does CDS have something like that
				{
				if(process.Id() != thisProcessId)
					{
					if(process.FileName().Find(KCoreDumpServerName) != KErrNotFound)
						{
	                    LOG_MSG( "core_dump_main.cpp::CDSAlreadyExists() returning: true" );
						process.Close();
						return ETrue;
						}
					}
				}
			}
		process.Close();
		}

	LOG_MSG( "core_dump_main.cpp::CDSAlreadyExists() returning: false\n" );
	return EFalse;
	}


/**
Perform all server initialisation, in particular creation of the
scheduler and server and then run the scheduler
*/

void CCoreDumpServer::RunServerL()
	{

	LOG_MSG( "CCoreDumpServer::RunServerL() -> new (ELeave) CActiveScheduler\n" );
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler;

	LOG_MSG( "CCoreDumpServer::RunServerL() -> PushL(scheduler)\n" );
	CleanupStack::PushL(scheduler);

	LOG_MSG( "CCoreDumpServer::RunServerL() -> Install(scheduler)\n" );
	CActiveScheduler::Install(scheduler);
    
	LOG_MSG( "CCoreDumpServer::RunServerL() -> CCoreDumpServer::NewLC()\n" );
	CCoreDumpServer::NewLC();
	
	LOG_MSG( "CCoreDumpServer::RunServerL() -> Rendezvous(KErrNone)\n" );
	RProcess::Rendezvous(KErrNone);

	LOG_MSG( "CCoreDumpServer::RunServerL() -> CActiveScheduler::Start()\n" );
	CActiveScheduler::Start();
	
	// Free the server and active scheduler.
    CleanupStack::PopAndDestroy(2, scheduler);
	}

/**
   Entry point for Core Dump Server
*/
TInt E32Main()
	{
	__UHEAP_MARK;

	CTrapCleanup* cleanup = CTrapCleanup::New();
	
	TInt err = KErrNoMemory;
	if	(cleanup)
		{
        if(!CDSAlreadyExists())
            {
		    TRAP(err, CCoreDumpServer::RunServerL());
            }
        else
            err = KErrAlreadyExists; 
		delete cleanup;
		}

	LOG_MSG2( "<- core_dump_main.cpp::E32Main() returning: %d\n", err );
	__UHEAP_MARKEND;
	return KErrNone;
	}

