/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __TRKSOCKETCOMMPORT_H__
#define __TRKSOCKETCOMMPORT_H__

#include <in_sock.h>
#include "TrkCommPort.h"


_LIT8(KSocketComm, "[SOCKETCOMM]");

//
// class CTrkSocketCommPort
//
// Defines an interface to a socket type communications port
//

class CTrkSocketCommPort : public CTrkCommPort
{
public:
		
	static CTrkSocketCommPort* NewL(const TDesC& aInitFilePath, TDes& aErrorMessage);
	~CTrkSocketCommPort();

	void OpenPortL();
	void ClosePort();

	void SendDataL(const TDesC8& aBuffer);
	void Listen(CTrkFramingLayer *aFramingLayer);
	void StopListening();

protected:

	void DoCancel();
	void RunL();

private:

	CTrkSocketCommPort();
	void ConstructL(const TDesC &aInitFilePath, TDes& aErrorMessage);
	void ConnectL();
	void IssueReadRequest();

private:

	RSocket iSocket;
	RSocketServ iSocketServ;

	TBool iConnected;
	TBool iListening;

	TBuf8<MAXMESSAGESIZE> iReceivedChars;
	TInt iNextReadChar;

	CTrkFramingLayer* iFramingLayer;
	
	TInt iLineNumber;
};

#endif // __TRKSOCKETCOMMPORT_H__
