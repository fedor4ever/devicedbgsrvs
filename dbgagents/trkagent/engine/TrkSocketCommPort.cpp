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



#include "TrkSocketCommPort.h"
#include "TrkFramingLayer.h"
#include "TrkEngine.h"
#include <e32cons.h>


CTrkSocketCommPort::CTrkSocketCommPort()
	: CTrkCommPort(CActive::EPriorityStandard),
	  iConnected(EFalse)
{
}

void CTrkSocketCommPort::ConstructL(const TDesC &/*aInitFilePath*/, TDes& /*aErrorMessage*/)
{
	CActiveScheduler::Add(this); 
}

void CTrkSocketCommPort::IssueReadRequest()
{
	iNextReadChar = 0;
	TSockXfrLength len;
	iSocket.RecvOneOrMore(iReceivedChars, 0, iStatus, len);

	SetActive();
}

void CTrkSocketCommPort::DoCancel()
{
	iSocket.CancelRead();
}

void CTrkSocketCommPort::RunL()
{
	if (iStatus.Int() == KErrNone)
		while (iNextReadChar < iReceivedChars.Length())
			iFramingLayer->HandleByte(iReceivedChars[iNextReadChar++]);

	IssueReadRequest();
}


CTrkSocketCommPort::~CTrkSocketCommPort()
{
	Cancel();
//	iSocket.Close();
	iConnected = EFalse;
	iSocketServ.Close();
}


CTrkSocketCommPort* CTrkSocketCommPort::NewL(const TDesC& aInitFilePath, TDes& aErrorMessage)
{
	CTrkSocketCommPort* self = new(ELeave) CTrkSocketCommPort;
	CleanupStack::PushL(self);
	self->ConstructL(aInitFilePath, aErrorMessage);
	CleanupStack::Pop(self);
	return self;
}


void CTrkSocketCommPort::OpenPortL()
{
	TInt error = KErrNone;
	
	error = iSocketServ.Connect();
	if (error != KErrNone)
		User::Leave(100);

	error = iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp);
	if (error != KErrNone)
		User::Leave(101);

	TUint protocols = 0;
	
	error = iSocketServ.NumProtocols(protocols);
	
	TProtocolDesc desc;
	
	for (TUint i=0; i<protocols; i++)
	{
		error = iSocketServ.GetProtocolInfo(i, desc);
	}
	
}

void CTrkSocketCommPort::ConnectL()
{
	TInetAddr address;
	TInt err = KErrNone;
	
//	address.SetPort(10000);
//	err = address.Input(_L("10.86.2.93"));
//
//	TRequestStatus stat;
//	iSocket.Connect(address, stat);
//	User::WaitForRequest(stat);

//	iConnected = ETrue;

//	TSockAddr test;
//	iSocket.LocalName(test);
//	TUint lp = iSocket.LocalPort();

	
	address.SetPort(6110);
	err = address.Input(_L("127.0.0.1"));
//	err = address.Input(_L("0.0.0.0"));

	err = iSocket.Bind(address);
	if (err != KErrNone)
	{
		User::Leave(err);
	}

	err = iSocket.Listen(1);
	if (err != KErrNone)
	{
		User::Leave(err);
	}

	RSocket blankSocket;
	err = blankSocket.Open(iSocketServ);

	TRequestStatus stat;
	iSocket.Accept(blankSocket, stat);
//	SetActive();
	User::WaitForRequest(stat);
	User::Leave(5000);

//	iSocket.Connect(address, iStatus);

	iConnected = ETrue;
}

void CTrkSocketCommPort::ClosePort()
{
	Cancel();
	
	if (iConnected)
	{
		iSocket.Close();
		iConnected = EFalse;
	}

	iSocketServ.Close();
}

void CTrkSocketCommPort::SendDataL(const TDesC8& aBuffer)
{
	TRequestStatus status;
	iSocket.Write(aBuffer, status);
	User::WaitForRequest(status);

	User::LeaveIfError(status.Int());
}

void CTrkSocketCommPort::Listen(CTrkFramingLayer */*aFramingLayer*/)
{
//#warning - can leave
//	if (!iConnected)
//		ConnectL();
//	iFramingLayer = aFramingLayer;
//	CActiveScheduler::Add(this);
//	IssueReadRequest();
}

void CTrkSocketCommPort::StopListening()
{
	Cancel();
	Deque();
}
