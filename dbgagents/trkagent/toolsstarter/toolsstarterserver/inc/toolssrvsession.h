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

#ifndef __toolssrvsession_h__
#define __toolssrvsession_h__

// System includes
#include <e32base.h>
#include "sessionmessgelistener.h"
#include "launchmanagerinterface.h"

//
// CToolsSrvSession (header)
//
// This class acts as a simple wrapper around the IPC client-server framework,
// and drives a real underlying engine, which is actually responsible for
// fulfulling all the client requests.

 
class CToolsSrvSession : public CSession2 ,public MConnStateListener
{
	public:
		static 	CToolsSrvSession* NewL();
				~CToolsSrvSession();

	private:
				CToolsSrvSession();
		void 	ConstructL();

	public:
		void 	HandleServerDestruction();
		void    CreateL();

	private:



	private:
	//FROM CSession2
		void 	ServiceL(const RMessage2& aMessage);

	private:
	// CMD - ToolsSERVER SPECIFIC FUNCTIONALITY
		
		void   CmdGetUsbConnStatus(const RMessage2& aMessage);
		void   CmdConnNofify(const RMessage2& aMessage);
		void   CmdConnNotifyCancel(const RMessage2& aMessage);
		void   CmdShutDownServer();


private:
	/*
	 * Used to transfer collections of objects between server and client.
	 */
	CBufBase*	 iTransferBuffer;
	RMessagePtr2 iBlockedRead;
	TBool     iPendingRead;
	ILaunchManagerInterface* iLaunchManager;
	
	public:
	    
	void ConnStatusChanged(TConnectionStatus );
	
};

#endif
