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
#include "TrkDccCommPort.h"
#include "TrkFramingLayer.h"

_LIT(KSendErrMsg, "Failed to write to the DCC channel");

//
// Default comm values
//


//
// Static helper functions
//

//
//
// CTrkDccCommPort implementation
//
//

//
// CTrkDccCommPort constructor
//
CTrkDccCommPort::CTrkDccCommPort()
	: CTrkCommPort(CActive::EPriorityStandard),
	  iConnected(EFalse),
	  iListening(EFalse),
	  iLineNumber(0),
	  iLddLoaded(EFalse)
{
}

//
// CTrkDccCommPort::ConstructL
//
void CTrkDccCommPort::ConstructL(TTrkConnData& aTrkConnData, TDes& aErrorMessage)
{

	iLDD = aTrkConnData.iLDD;
	iUnitNumber = aTrkConnData.iPortNumber;

	if (!iLDD.Size())
	{
		aErrorMessage.Format(_L("LDD not specified in init file\r\n"));		
		User::Leave(KErrCorrupt);
	}

	
}

//
// CTrkDccCommPort destructor
//
CTrkDccCommPort::~CTrkDccCommPort()
{
	if (iLddLoaded)	
	{
		//cancel the read request
		iDccDriver.ReadCancel();
		iDccDriver.Close();
		
		TInt err = User::FreeLogicalDevice(iLDD);
		if (KErrNone != err)
			User::Panic(_L("FreeLogicalDevice failed"), err);
			
		iLddLoaded = EFalse;
	}

}

//
// CTrkDccCommPort::NewL
//
CTrkDccCommPort* CTrkDccCommPort::NewL(TTrkConnData& aTrkConnData, TDes& aErrorMessage)
{
	CTrkDccCommPort* self = new(ELeave) CTrkDccCommPort;
	CleanupStack::PushL(self);
	self->ConstructL(aTrkConnData, aErrorMessage);
	CleanupStack::Pop(self);
	return self;
}

//
// CTrkDccCommPort::OpenPortL
//
// Open the serial type communications port
//
void CTrkDccCommPort::OpenPortL()
{
	TInt error = KErrNone;

	// make sure the LDD is not already loaded
	User::FreeLogicalDevice(iLDD);

	error = User::LoadLogicalDevice(iLDD);
	if ((KErrAlreadyExists != error) && (KErrNone != error))
		ReportAndLeaveIfErrorL(error, _L("Failed to load dcc logical device."));

	error = iDccDriver.Open();
	if (KErrNone != error)
		ReportAndLeaveIfErrorL(error, _L("Failed to open dcc logical device."));
	
	iLddLoaded = ETrue;
	
	
	_LIT(KDCCMSG, "LDD: TrkDccDriver\r\nPort number: 42\r\n");
	
	iConnectionMessage = KDCCMSG;
}

//
// CTrkDccCommPort::ClosePort
//
// Close the communications port
//
void CTrkDccCommPort::ClosePort()
{
	Cancel();
	
	if (iLddLoaded)
	{
		//cancel the read request
		iDccDriver.ReadCancel();		
		iDccDriver.Close();
		
		TInt err = User::FreeLogicalDevice(iLDD);
		if (KErrNone != err)
			User::Panic(_L("FreeLogicalDevice failed"), err);

		iLddLoaded = EFalse;			
	}

}

//
// CTrkDccCommPort::SendDataL
//
// Write data to the serial type port
//
void CTrkDccCommPort::SendDataL(const TDesC8& aBuffer)
{
	TInt err = iDccDriver.Write(aBuffer);
	
	ReportAndLeaveIfErrorL(err, KSendErrMsg);
}

//
// CTrkDccCommPort::Listen
//
// Start listening for data coming into the serial type communications port
//
void CTrkDccCommPort::Listen(CTrkFramingLayer *aFramingLayer)
{
	iFramingLayer = aFramingLayer;
	CActiveScheduler::Add(this);
	IssueReadRequest();
	iListening = ETrue;
}

//
// CTrkDccCommPort::StopListening
//
// Stop listening for data coming into the serial type communications port
//
void CTrkDccCommPort::StopListening()
{
	if (iListening)
	{
		Cancel();
		Deque();
	}
	
	iListening = EFalse;
}

//
// CTrkDccCommPort::ReportAndLeaveIfErrorL
//
// If an error occurred, print the error information to the screen and bail out
//
void CTrkDccCommPort::ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc)
{
	if (KErrNone != aError)
	{
		iErrorMessage.Format(_L("%S\r\nError Code: %d\r\n"), &aDesc, aError);
		User::Leave(aError);
	}
}


//
// CTrkDccCommPort::IssueReadRequest
//
// Wait for data to come into the communications port
//
void CTrkDccCommPort::IssueReadRequest()
{
	iNextReadChar = 0;
	iDccDriver.ReadOneOrMore(iStatus, iReceivedChars);
	SetActive();
}

//
// CTrkDccCommPort::DoCancel
//
// Cancel the request for data from the communications port
//
void CTrkDccCommPort::DoCancel()
{
	iDccDriver.ReadCancel();
}

//
// CTrkDccCommPort::RunL
//
// Called when data comes into the communications port
//
void CTrkDccCommPort::RunL()
{
	// pass the data onto the framing layer
	if (iStatus.Int() == KErrNone)
	{
		while (iNextReadChar < iReceivedChars.Length())
			iFramingLayer->HandleByte(iReceivedChars[iNextReadChar++]);
		//iDccDriver.Write(iReceivedChars);
	}
	// continue waiting for data
	IssueReadRequest();
}

