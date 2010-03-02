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

#ifndef __TrkSrvServer_H__
#define __TrkSrvServer_H__

// System includes
#include <e32base.h>

// User includes
#include "trkdebugmanager.h"

// Type definitions

// Constants

// Enumerations


//
// CTrkSrvServer (header)
// The Trk Server
//
#ifdef EKA2
class CTrkSrvServer : public CPolicyServer
#else
class CTrkSrvServer : public CServer2
#endif
{
	public:
		static CTrkSrvServer* NewLC(CTrkDebugManager* aTrkDebugMgr);
		~CTrkSrvServer();

	private:
		CTrkSrvServer(CTrkDebugManager* aTrkDebugMgr);
		void ConstructL();

	public:
		CSession2* NewSessionL(const TVersion& aVersion,const RMessage2& aMessage) const;

	private:
		TInt RunError(TInt aError);

	private:
	#ifdef EKA2	
		TCustomResult CustomSecurityCheckL(const RMessage2& aMsg, TInt& aAction, TSecurityInfo& aMissing);
	#endif
	
	private:
		CTrkDebugManager* iTrkDebugMgr;
};

#endif
