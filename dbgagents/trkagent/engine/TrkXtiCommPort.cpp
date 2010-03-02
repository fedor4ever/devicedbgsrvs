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
#include "TrkXtiCommPort.h"
#include "TrkFramingLayer.h"

_LIT(KSendErrMsg, "Failed to write to the XTI channel");

//
// Default comm values
//


//
// Static helper functions
//

//
//
// CTrkXtiCommPort implementation
//
//

//
// CTrkXtiCommPort constructor
//
CTrkXtiCommPort::CTrkXtiCommPort()
	: CTrkCommPort(CActive::EPriorityStandard),
	  iConnected(EFalse),
	  iListening(EFalse),
	  iLineNumber(0),
	  iLddLoaded(EFalse)
{
}

//
// CTrkXtiCommPort::ConstructL
//
void CTrkXtiCommPort::ConstructL(TTrkConnData aTrkConnData, TDes& aErrorMessage)
{

	iLDD = aTrkConnData.iLDD;
	// We don't really need this
	//iUnitNumber = aTrkConnData.iPortNumber;

	if (!iLDD.Size())
	{
		aErrorMessage.Format(_L("LDD not specified in init file\r\n"));		
		User::Leave(KErrCorrupt);
	}
	
}

//
// CTrkXtiCommPort destructor
//
CTrkXtiCommPort::~CTrkXtiCommPort()
{
	if (iLddLoaded)	
	{				
		//cancel the read request
		iXtiDriver.ReadCancel();		
		iXtiDriver.Close();

		TInt err = User::FreeLogicalDevice(iLDD);
		if (KErrNone != err)
			User::Panic(_L("FreeLogicalDevice failed"), err);
			
		iLddLoaded = EFalse;
	}
	
}

//
// CTrkXtiCommPort::NewL
//
CTrkXtiCommPort* CTrkXtiCommPort::NewL(TTrkConnData aTrkConnData, TDes& aErrorMessage)
{
	CTrkXtiCommPort* self = new(ELeave) CTrkXtiCommPort;
	CleanupStack::PushL(self);
	self->ConstructL(aTrkConnData, aErrorMessage);
	CleanupStack::Pop(self);
	return self;
}

//
// CTrkXtiCommPort::OpenPortL
//
// Open the serial type communications port
//
void CTrkXtiCommPort::OpenPortL()
{
	TInt error = KErrNone;
	
	// make sure the LDD is not already loaded
	User::FreeLogicalDevice(iLDD);

	error = User::LoadLogicalDevice(iLDD);
	if ((KErrAlreadyExists != error) && (KErrNone != error))
		ReportAndLeaveIfErrorL(error, _L("Failed to load xti logical device."));	

	error = iXtiDriver.Open();
	if (KErrNone != error)
		ReportAndLeaveIfErrorL(error, _L("Failed to open xti logical device."));
		
	iLddLoaded = ETrue;
	
	
	_LIT(KXTIMSG, "Connection: XTI\r\nLDD: TrkXtiDriver\r\n");
	
	iConnectionMessage = KXTIMSG;
}

//
// CTrkXtiCommPort::ClosePort
//
// Close the communications port
//
void CTrkXtiCommPort::ClosePort()
{
	Cancel();
	
	if (iLddLoaded)
	{
		//cancel the read request
		iXtiDriver.ReadCancel();		
		iXtiDriver.Close();
		
		TInt err = User::FreeLogicalDevice(iLDD);
		if (KErrNone != err)
			User::Panic(_L("FreeLogicalDevice failed"), err);

		iLddLoaded = EFalse;			
	}

}

//
// CTrkXtiCommPort::SendDataL
//
// Write data to the serial type port
//
void CTrkXtiCommPort::SendDataL(const TDesC8& aBuffer)
{
	TInt err = iXtiDriver.Write(aBuffer);
	
	ReportAndLeaveIfErrorL(err, KSendErrMsg);
}

//
// CTrkXtiCommPort::Listen
//
// Start listening for data coming into the serial type communications port
//
void CTrkXtiCommPort::Listen(CTrkFramingLayer *aFramingLayer)
{
	iFramingLayer = aFramingLayer;
	CActiveScheduler::Add(this);
	IssueReadRequest();
	iListening = ETrue;
}

//
// CTrkXtiCommPort::StopListening
//
// Stop listening for data coming into the serial type communications port
//
void CTrkXtiCommPort::StopListening()
{
	if (iListening)
	{
		Cancel();
		Deque();
	}
	
	iListening = EFalse;
}

//
// CTrkXtiCommPort::ReportAndLeaveIfErrorL
//
// If an error occurred, print the error information to the screen and bail out
//
void CTrkXtiCommPort::ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc)
{
	if (KErrNone != aError)
	{
		iErrorMessage.Format(_L("%S\r\nError Code: %d\r\n"), &aDesc, aError);
		User::Leave(aError);
	}
}


//
// CTrkXtiCommPort::IssueReadRequest
//
// Wait for data to come into the communications port
//
void CTrkXtiCommPort::IssueReadRequest()
{
	iNextReadChar = 0;
	iXtiDriver.ReadOneOrMore(iStatus, iReceivedChars);
	SetActive();
}

//
// CTrkXtiCommPort::DoCancel
//
// Cancel the request for data from the communications port
//
void CTrkXtiCommPort::DoCancel()
{
	iXtiDriver.ReadCancel();
}

//
// CTrkXtiCommPort::RunL
//
// Called when data comes into the communications port
//
void CTrkXtiCommPort::RunL()
{
	// pass the data onto the framing layer
	if (iStatus.Int() == KErrNone)
	{
		while (iNextReadChar < iReceivedChars.Length())
			iFramingLayer->HandleByte(iReceivedChars[iNextReadChar++]);
		//iXtiDriver.Write(iReceivedChars);
	}
	// continue waiting for data
	IssueReadRequest();
}

