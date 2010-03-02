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

#ifndef __DbgTrcSrvSession_h__
#define __DbgTrcSrvSession_h__

// System includes
#include <e32base.h>

// User includes
#include "ostprotdefs.h"

// Constants

// Classes referenced
class COstBaseRouter;
class COstMessage;

// Enumerations


//
// CDbgTrcSrvSession (header)
//
/**
 * This class acts as a simple wrapper around the IPC client-server framework,
 * and drives a real underlying engine, which is actually responsible for
 * fulfulling all the client requests.
 */
 
class CDbgTrcSrvSession : public CSession2
{
	public:
		static 	CDbgTrcSrvSession* NewL(COstBaseRouter* aOstRouter);
				~CDbgTrcSrvSession();

	private:
				CDbgTrcSrvSession(COstBaseRouter* aOstRouter);
		void 	ConstructL();

	public:
		void 	HandleServerDestruction();
		void 	CreateL();

	public: // callbacks for the OST router
		void 	MessageAvailableL(TDesC8& aMsg);
		void 	WriteComplete(TInt aErrCode);
	
	private:
	//FROM CSession2
		void 	ServiceL(const RMessage2& aMessage);

		
	private:
	// CMD - SPECIFIC FUNCTIONALITY
		void 	CmdGetAcmConfigL(const RMessage2& aMessage);
		void 	CmdSetAcmConfigL(const RMessage2& aMessage);
		
		void 	CmdOpenCommPortL(const RMessage2& aMessage);
		void 	CmdCloseCommPort(const RMessage2& aMessage);

		void 	CmdRegisterProtocol(const RMessage2& aMessage);
		void 	CmdUnRegisterProtocol(const RMessage2& aMessage);
		
		void 	CmdRegisterProtocolIdsL(const RMessage2& aMessage);
		void 	CmdUnRegisterProtocolIdsL(const RMessage2& aMessage);
		
		void    CmdReadMsgL(const RMessage2& aMessage);
		void 	CmdReadCancel(const RMessage2& aMessage);
		
		void 	CmdWriteMsgL(const RMessage2& aMessage);
		void 	CmdWriteCancel(const RMessage2& aMessage);
		void 	CmdDisconnect(const RMessage2& aMessage);
		
		void 	DoDisconnect();


private:
	/*
	 * Used to transfer collections of objects between server and client.
	 */
	CBufBase*					iTransferBuffer;

	RMessagePtr2			iBlockedRead;	
	RMessagePtr2			iBlockedWrite;
	TBool 					iPendingRead;
	TBool 					iPendingWrite;
	
	TUint					iPendingReadBufferLength;
	
	COstBaseRouter* iOstRouter;
	
	RPointerArray<COstMessage>	iRecvMessageQueue;
	RArray<TOstProtIds> iProtocolIds;
};

#endif //__DbgTrcSrvSession_h__
