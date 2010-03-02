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

#ifndef __TrkSrvSession_h__
#define __TrkSrvSession_h__

// System includes
#include <e32base.h>

// User includes

#include "trkdebugmanager.h"
#include "trkdebugmgrcallbacks.h"

// Constants

// Classes referenced
class CTrkSrvSessionEngine;
// Enumerations


//
// CTrkSrvSession (header)
//
/**
 * This class acts as a simple wrapper around the IPC client-server framework,
 * and drives a real underlying engine, which is actually responsible for
 * fulfulling all the client requests.
 */
 
class CTrkSrvSession : public CSession2, public MTrkDebugMgrCallbacks
{
	public:
		static 	CTrkSrvSession* NewL(CTrkDebugManager* aTrkDebugMgr);
				~CTrkSrvSession();
    
	public:
        void    HandleServerDestruction();

	public:
	//FROM CSession2
	    void DebugConnStatusChanged(TTrkConnStatus& aConnStatus);
	    void DebuggingStatusChanged(TBool& aDebugging);
	    
	private:
    //FROM CSession2
        void    ServiceL(const RMessage2& aMessage);
    
	
	private:
                CTrkSrvSession(CTrkDebugManager* aTrkDebugMgr);
        void    ConstructL();

	private:	
	// CMD - TRKSERVER SPECIFIC FUNCTIONALITY
		void	CmdGetVersionL(const RMessage2& aMessage);
		void	CmdConnectL(const RMessage2& aMessage);
		void	CmdDisConnectL(const RMessage2& aMessage);
		void    CmdGetCurrentConnDataL(const RMessage2& aMessage);
		void    CmdSetCurrentConnDataL(const RMessage2& aMessage);
		void	CmdGetDebugConnStatusL(const RMessage2& aMessage);
		void	ConnStatusNotifyL(const RMessage2& aMessage);
		void 	CmdGetDebugStatusL(const RMessage2& aMessage);
		void 	DebugStatusNotifyL(const RMessage2& aMessage);
		void    CmdShutDownServer(const RMessage2& aMessage);
		void    ConnStatusNotifyCancel(const RMessage2& aMessage);
		void    DebugStatusNotifyCancel(const RMessage2& aMessage);
		void    GetPlugAndPlayTypeL(const RMessage2& aMessage);
		void    SetPlugAndPlayTypeL(const RMessage2& aMessage);

private:
	/*
	 * Used to transfer collections of objects between server and client.
	 */
	CBufBase*				iTransferBuffer;

	CTrkDebugManager* 		iTrkDebugMgr;
	
    RMessagePtr2            iBlockedDebugState;   
    RMessagePtr2            iBlockedConnState;
    TBool                   iPendingDebugState;
    TBool                   iPendingConnState;
};

#endif
