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

#ifndef __toolssrvserver_H__
#define __toolssrvserver_H__

// System includes
#include <e32base.h>
#include "toolslaunchmgr.h"
//
// CToolsSrvServer (header)
// The Tools Server
//
#ifdef EKA2
class CToolsSrvServer : public CPolicyServer
#else
class CToolsSrvServer : public CServer2
#endif
{
	public:
		static CToolsSrvServer* NewLC();
		~CToolsSrvServer();

	private:
		CToolsSrvServer();
		void ConstructL();

	public:
		CSession2* NewSessionL(const TVersion& aVersion,const RMessage2& aMessage) const;
        // called by the session objects
		void SessionOpened();
		void SessionClosed();
	
	public:
	    TConnectionStatus GetUsbConnStatus();
	    ILaunchManagerInterface* GetLaunchInterface();

	private:
		TInt RunError(TInt aError);

	private:
	    TInt iSessionCount;
	    CToolsLaunchMgr* iLaunchManager;
	#ifdef EKA2	
		TCustomResult CustomSecurityCheckL(const RMessage2& aMsg, TInt& aAction, TSecurityInfo& aMissing);
	#endif
};

//
//                  inline implementations
//
/*
 * To get the Launchmanger instance to be used for sessions
 * 
 */
inline ILaunchManagerInterface* CToolsSrvServer::GetLaunchInterface()
{
    if(iLaunchManager)
        return iLaunchManager;
    return NULL;    
    
}

#endif
