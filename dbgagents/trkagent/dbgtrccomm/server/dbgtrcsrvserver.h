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

#ifndef __DbgTrcSrvServer_H__
#define __DbgTrcSrvServer_H__

// System includes
#include <e32base.h>
#include "shutdowntimer.h"
// User includes

// Forward declarations
class COstBaseRouter;


//
// CDbgTrcSrvServer (header)
//
//
// The DbgTrc Server
//
#ifdef EKA2
class CDbgTrcSrvServer : public CPolicyServer
#else
class CDbgTrcSrvServer : public CServer2
#endif
{
	public:
		static CDbgTrcSrvServer* NewLC();
		~CDbgTrcSrvServer();

	private:
		CDbgTrcSrvServer();
		void ConstructL();

	public:
		CSession2* NewSessionL(const TVersion& aVersion,const RMessage2& aMessage) const;
		// called by the session objects
		void SessionOpened();
		void SessionClosed();

	private:
		TInt RunError(TInt aError);

	private:
	#ifdef EKA2	
		TCustomResult CustomSecurityCheckL(const RMessage2& aMsg, TInt& aAction, TSecurityInfo& aMissing);
	#endif
	
	private:
		COstBaseRouter* iOstRouter;
		TInt iSessionCount;
		CShutdownTimer iShutdown;
};

#endif //__DbgTrcSrvServer_H__
