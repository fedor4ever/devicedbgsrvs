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

#ifndef __TrkTcbSrvSession_h__
#define __TrkTcbSrvSession_h__

// System includes
#include <e32base.h>

// User includes
// Constants

// Classes referenced
class CTrkTcbSrvSessionEngine;
// Enumerations


//
// CTrkTcbSrvSession (header)
//
/**
 * This class acts as a simple wrapper around the IPC client-server framework,
 * and drives a real underlying engine, which is actually responsible for
 * fulfulling all the client requests.
 */
 
class CTrkTcbSrvSession : public CSession2
{
	public:
		static 	CTrkTcbSrvSession* NewL();
				~CTrkTcbSrvSession();

	private:
				CTrkTcbSrvSession();
		void 	ConstructL();

	public:
		void 	HandleServerDestruction();

	private:



	private:
	//FROM CSession2
		void 	ServiceL(const RMessage2& aMessage);

	private:
		TBool 	DoServiceL();
		
		inline const RMessage2&	Message() const { return *iMessage; }

	private:
	// CMD - TRKTCBSERVER SPECIFIC FUNCTIONALITY
		TBool	CmdOpenFileL();
		TBool	CmdReadFileL();
		TBool	CmdWriteFileL();
		TBool	CmdCloseFileL();
		TBool	CmdPositionFileL();
		TBool 	CmdShutDownServer();


private:
	/*
	 * Used to transfer collections of objects between server and client.
	 */
	CBufBase*					iTransferBuffer;

	CTrkTcbSrvSessionEngine*	iSessionEngine;

	const RMessage2*			iMessage;
};

#endif
