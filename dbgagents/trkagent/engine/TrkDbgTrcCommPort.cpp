/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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


#include <e32cons.h>
#include <f32file.h>

#include "TrkConnData.h"
#include "TrkDbgTrcCommPort.h"
#include "TrkFramingLayer.h"

_LIT(KSendErrMsg, "Failed to write to the DbgTrc channel");

//
// Default comm values
//


//
// Static helper functions
//

//
//
// CTrkDbgTrcCommPort implementation
//
//

//
// CTrkDbgTrcCommPort constructor
//
CTrkDbgTrcCommPort::CTrkDbgTrcCommPort()
	: CTrkCommPort(CActive::EPriorityStandard),
	  iConnected(EFalse),
	  iListening(EFalse),
	  iLineNumber(0)
{
}

//
// CTrkDbgTrcCommPort::ConstructL
//
void CTrkDbgTrcCommPort::ConstructL(TTrkConnData aTrkConnData, TDes& aErrorMessage)
{
	iUnitNumber = aTrkConnData.iPortNumber;
}

//
// CTrkDbgTrcCommPort destructor
//
CTrkDbgTrcCommPort::~CTrkDbgTrcCommPort()
{
	
}

//
// CTrkDbgTrcCommPort::NewL
//
CTrkDbgTrcCommPort* CTrkDbgTrcCommPort::NewL(TTrkConnData aTrkConnData, TDes& aErrorMessage)
{
	CTrkDbgTrcCommPort* self = new(ELeave) CTrkDbgTrcCommPort;
	CleanupStack::PushL(self);
	self->ConstructL(aTrkConnData, aErrorMessage);
	CleanupStack::Pop(self);
	return self;
}

//
// CTrkDbgTrcCommPort::OpenPortL
//
// Open the serial type communications port
//
void CTrkDbgTrcCommPort::OpenPortL()
{
	TInt error = KErrNone;

	error = iDbgTrcPort.Connect();
	if ((KErrAlreadyExists != error) && (KErrNone != error))
		ReportAndLeaveIfErrorL(error, _L("Failed to connect to the debug trace comms server."));

	TAcmConfig config;	
	error = iDbgTrcPort.GetAcmConfig(config);

	if (KErrNone != error)
		ReportAndLeaveIfErrorL(error, _L("Failed to get the port configuration."));
		
	TAcmConfigV01 config1(*(TAcmConfigV01*)config.Ptr());
	config1.iPortNumber = iUnitNumber;
	
	error = iDbgTrcPort.SetAcmConfig(config);
	if (KErrNone != error && KErrInUse != error)
		ReportAndLeaveIfErrorL(error, _L("Failed to set the port configuration."));

	error = iDbgTrcPort.Open();
	
	if (KErrNone != error)
		ReportAndLeaveIfErrorL(error, _L("Failed to open debug trace port."));
	
	iConnected = ETrue;
	
	// register the protocol id here
	error = iDbgTrcPort.RegisterProtocolID(EOstProtTrk, EFalse);
	if (KErrNone != error)
		ReportAndLeaveIfErrorL(error, _L("Failed to register the protocol id."));

		
	iConnectionMessage.Format(_L("Connection: USB \r\nPort: %d\r\n"), iUnitNumber);
}

//
// CTrkDbgTrcCommPort::ClosePort
//
// Close the communications port
//
void CTrkDbgTrcCommPort::ClosePort()
{
	Cancel();	
	
	if (iConnected)
	{
		iDbgTrcPort.UnRegisterProtocolID(EOstProtTrk);		
		//iDbgTrcPort.Disconnect();
		iDbgTrcPort.Close();
		iConnected = EFalse;			
	}	
}

//
// CTrkDbgTrcCommPort::SendDataL
//
// Write data to the serial type port
//
void CTrkDbgTrcCommPort::SendDataL(const TDesC8& aBuffer)
{
	TRequestStatus status;
	iDbgTrcPort.WriteMessage(status, aBuffer, EFalse);
	User::WaitForRequest(status);
	
	TInt err = status.Int();	
	ReportAndLeaveIfErrorL(err, KSendErrMsg);
}

//
// CTrkDbgTrcCommPort::Listen
//
// Start listening for data coming into the serial type communications port
//
void CTrkDbgTrcCommPort::Listen(CTrkFramingLayer *aFramingLayer)
{	
	iFramingLayer = aFramingLayer;
	CActiveScheduler::Add(this);
	IssueReadRequest();
	iListening = ETrue;
}

//
// CTrkDbgTrcCommPort::StopListening
//
// Stop listening for data coming into the serial type communications port
//
void CTrkDbgTrcCommPort::StopListening()
{
	if (iListening)
	{
		Cancel();
		Deque();
	}
	
	iListening = EFalse;
}

//
// CTrkDbgTrcCommPort::ReportAndLeaveIfErrorL
//
// If an error occurred, print the error information to the screen and bail out
//
void CTrkDbgTrcCommPort::ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc)
{
	if (KErrNone != aError)
	{
		iErrorMessage.Format(_L("%S\r\nError Code: %d\r\n"), &aDesc, aError);
		User::Leave(aError);
	}
}


//
// CTrkDbgTrcCommPort::IssueReadRequest
//
// Wait for data to come into the communications port
//
void CTrkDbgTrcCommPort::IssueReadRequest()
{
	iNextReadChar = 0;
	iDbgTrcPort.ReadMessage(iStatus, iReceivedChars);
	SetActive();
}

//
// CTrkDbgTrcCommPort::DoCancel
//
// Cancel the request for data from the communications port
//
void CTrkDbgTrcCommPort::DoCancel()
{
	iDbgTrcPort.ReadCancel();
}

//
// CTrkDbgTrcCommPort::RunL
//
// Called when data comes into the communications port
//
void CTrkDbgTrcCommPort::RunL()
{
	// pass the data onto the framing layer
	if (iStatus.Int() == KErrNone)
	{
		while (iNextReadChar < iReceivedChars.Length())
			iFramingLayer->HandleByte(iReceivedChars[iNextReadChar++]);
	}
	// continue waiting for data
	IssueReadRequest();
}

