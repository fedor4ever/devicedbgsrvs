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


#include <e32cons.h>
#include <f32file.h>

#include "TrkEngineModel.h"
#include "TrkSerialCommPort.h"
#include "TrkFramingLayer.h"


//
// Static helper functions
//
static void BuildPortName(const TDesC& aModuleName, TUint aUnit, TDes& aCompleteName)
{
	aCompleteName.Zero();
	aCompleteName.Append(aModuleName);
	aCompleteName.AppendFill(':', 2);
	aCompleteName.AppendNum(aUnit);
}

static TUint RateCapacityBitMaskFromRate(TUint aRate)
{
	switch (aRate)
	{
		case 50:      return KCapsBps50;
		case 75:      return KCapsBps75;
		case 110:     return KCapsBps110;
		case 134:     return KCapsBps134;
		case 150:     return KCapsBps150;
		case 300:     return KCapsBps300;
		case 600:     return KCapsBps600;
		case 1200:    return KCapsBps1200;
		case 1800:    return KCapsBps1800;
		case 2000:    return KCapsBps2000;
		case 2400:    return KCapsBps2400;
		case 3600:    return KCapsBps3600;
		case 4800:    return KCapsBps4800;
		case 7200:    return KCapsBps7200;
		case 9600:    return KCapsBps9600;
		case 19200:   return KCapsBps19200;
		case 38400:   return KCapsBps38400;
		case 57600:   return KCapsBps57600;
		case 115200:  return KCapsBps115200;
		case 230400:  return KCapsBps230400;
		case 460800:  return KCapsBps460800;
		case 576000:  return KCapsBps576000;
		case 1152000: return KCapsBps1152000;
		case 4000000: return KCapsBps4000000;
		default:      return 0;
	}
}

static TBps TBpsRateFromRate(TUint aRate)
{
	switch (aRate)
	{
		case 50:      return EBps50;
		case 75:      return EBps75;
		case 110:     return EBps110;
		case 134:     return EBps134;
		case 150:     return EBps150;
		case 300:     return EBps300;
		case 600:     return EBps600;
		case 1200:    return EBps1200;
		case 1800:    return EBps1800;
		case 2000:    return EBps2000;
		case 2400:    return EBps2400;
		case 3600:    return EBps3600;
		case 4800:    return EBps4800;
		case 7200:    return EBps7200;
		case 9600:    return EBps9600;
		case 19200:   return EBps19200;
		case 38400:   return EBps38400;
		case 57600:   return EBps57600;
		case 115200:  return EBps115200;
		case 230400:  return EBps230400;
		case 460800:  return EBps460800;
		case 576000:  return EBps576000;
		case 1152000: return EBps1152000;
		case 4000000: return EBps4000000;
		default:      return EBpsAutobaud;
	}
}


//
//
// CTrkSerialCommPort implementation
//
//

//
// CTrkSerialCommPort constructor
//
CTrkSerialCommPort::CTrkSerialCommPort()
	: CTrkCommPort(CActive::EPriorityStandard),
	  iPDD(NULL),
	  iPDD2(NULL),
	  iLDD(NULL),
	  iCSY(NULL),
	  iServerStarted(EFalse),
	  iConnected(EFalse),
	  iListening(EFalse),
	  iLineNumber(0)
{
}

//
// CTrkSerialCommPort::ConstructL
//
void CTrkSerialCommPort::ConstructL(TTrkConnData& aTrkConnData , TDes& aErrorMessage)
{
	iPDD = aTrkConnData.iPDD;
	iLDD = aTrkConnData.iLDD;
	iCSY = aTrkConnData.iCSY;
	iUnitNumber = aTrkConnData.iPortNumber;
	iRate = aTrkConnData.iRate;
	
	
	TBuf<KMaxPath> initFile(KInitFilePath);
	initFile.Append(KInitFileName);
	
	if (!iPDD.Size())
	{
		aErrorMessage.Format(_L("PDD not specified in %S.\r\n"), &initFile);
			User::Leave(KErrCorrupt);
		}

	if (!iLDD.Size())
	{
		aErrorMessage.Format(_L("LDD not specified in %S.\r\n"), &initFile);		
			User::Leave(KErrCorrupt);
		}

	if (!iCSY.Size())
	{
		aErrorMessage.Format(_L("CSY not specified in %S.\r\n"), &initFile);		
			User::Leave(KErrCorrupt);
		}

		if (!iRate)
		{
		aErrorMessage.Format(_L("RATE not specified in %S.\r\n"), &initFile);		
			User::Leave(KErrCorrupt);
		}
	
	iPDD2.Format(iPDD, iUnitNumber+1);
		
	iConnectionMessage.Format(_L("PDD: %S\r\nLDD: %S\r\nCSY: %S\r\nPort number: %d\r\nBaud rate: %d\r\n"), &iPDD2, &iLDD, &iCSY, iUnitNumber, iRate);

	if (aTrkConnData.iDefault)
	{
		iConnectionMessage.Insert(0, _L("No init file found, using default values.\r\n"));
	}
}

//
// CTrkSerialCommPort destructor
//
CTrkSerialCommPort::~CTrkSerialCommPort()
{
	// make sure we cancel the request for data before shutting down
	Cancel();

	iPort.Close();
	iConnected = EFalse;
	iServer.Close();
	iServerStarted = EFalse;	
}

//
// CTrkSerialCommPort::NewL
//
CTrkSerialCommPort* CTrkSerialCommPort::NewL(TTrkConnData& aTrkConnData, TDes& aErrorMessage)
{
	CTrkSerialCommPort* self = new(ELeave) CTrkSerialCommPort;
	CleanupStack::PushL(self);
	self->ConstructL(aTrkConnData, aErrorMessage);
	CleanupStack::Pop(self);
	return self;
}

//
// CTrkSerialCommPort::OpenPortL
//
// Open the serial type communications port
//
void CTrkSerialCommPort::OpenPortL()
{
	TInt error = KErrNone;

	if (!iServerStarted)
	{
		StartC32();
		ReportAndLeaveIfErrorL(iServer.Connect(),
							  _L("Failed to connect to RCommServ."));
		iServerStarted = ETrue;
	}

	// load the csy module
	ReportAndLeaveIfErrorL(iServer.LoadCommModule(iCSY), _L("Failed to load CSY module."));

	TInt ports = 0;
	ReportAndLeaveIfErrorL(iServer.NumPorts(ports),
						   _L("Unable to obtain information about port."));

	// make sure the unit number is in range
	TSerialInfo serialInfo;

	error = KErrNotFound;
	
	TBuf<32> csyModule;
	csyModule = iCSY;
	csyModule.LowerCase();
	for (TInt i=0; i<ports && error; i++)
	{
		TBuf<32> module;
		ReportAndLeaveIfErrorL(iServer.GetPortInfo(i, module, serialInfo),
							   _L("Unable to obtain information about port."));		
		module.LowerCase();
		if (module == csyModule)
			error = KErrNone;
	}

	ReportAndLeaveIfErrorL(error, _L("Unable to obtain port information for comm module."));
	
	//If pdd is "NONE", don't try to load it.
	//This is true incase of USB, where the USB pdd is a kernel extension on most of the phones
	//and so can't be loaded.
	TBuf<10> defaultUsbPdd;
	defaultUsbPdd.Copy(KDefaultUsbPDD);	
	if (iPDD2.Compare(defaultUsbPdd))
	{
		error = User::LoadPhysicalDevice(iPDD2);
		if (error != KErrNone && error != KErrAlreadyExists)
			ReportAndLeaveIfErrorL(error, _L("Failed to load physical device."));
	}

	error = User::LoadLogicalDevice(iLDD);
	if (error != KErrNone && error != KErrAlreadyExists)
		ReportAndLeaveIfErrorL(error, _L("Failed to load logical device."));

	// Open port (try first in DTE mode and then if unsuccessful in
	// DCE mode - necessary for cogent).

	TBuf<KMaxPortName+4> portName;
	BuildPortName(serialInfo.iName, iUnitNumber, portName);
	error = iPort.Open(iServer, portName, ECommExclusive, ECommRoleDTE);
	if (error != KErrNone)
	{
		ReportAndLeaveIfErrorL(iPort.Open(iServer, portName, ECommExclusive, ECommRoleDCE),
							   _L("Failed to open port."));
	}
	iConnected = ETrue;

	// Check Speed
	TCommCaps caps;
	iPort.Caps(caps);

	TUint rateBitMask = RateCapacityBitMaskFromRate(iRate);
	if (rateBitMask == 0 || (rateBitMask & caps().iRate) == 0)
		ReportAndLeaveIfErrorL(KErrGeneral, _L("Unsupported baud rate."));

	// Configure physical and logical characteristics
	TCommConfig config;
	iPort.Config(config);
	config().iRate = TBpsRateFromRate(iRate);
	config().iParity = EParityNone;
	config().iDataBits = EData8;
	config().iStopBits = EStop1;
	config().iFifo = EFifoEnable;

	config().iHandshake = 0;	// no flow control at all

	ReportAndLeaveIfErrorL(iPort.SetConfig(config), _L("Failed to set port configuration."));

	iPort.SetReceiveBufferLength(MAXMESSAGESIZE);

	//reset the rx and tx buffers just in case 
	//if there is any stale data lying around from the previous debug sessions
	iPort.ResetBuffers(); 
}

//
// CTrkSerialCommPort::ClosePort
//
// Close the communications port
//
void CTrkSerialCommPort::ClosePort()
{
	Cancel();
	
	if (iConnected)
	{
		iPort.Close();
		iConnected = EFalse;
	}

	if (iServerStarted)
	{
		iServer.Close();
		iServerStarted = EFalse;
	}
}

//
// CTrkSerialCommPort::SendDataL
//
// Write data to the serial type port
//
void CTrkSerialCommPort::SendDataL(const TDesC8& aBuffer)
{
	//_LIT8(KPrefix, "TrkToCarbide: ");
	//PrintMessage(KPrefix(), aBuffer);
	
	TRequestStatus status;
	iPort.Write(status, aBuffer);
	User::WaitForRequest(status);
	
	User::LeaveIfError(status.Int());
}

//
// CTrkSerialCommPort::Listen
//
// Start listening for data coming into the serial type communications port
//
void CTrkSerialCommPort::Listen(CTrkFramingLayer *aFramingLayer)
{
	iFramingLayer = aFramingLayer;
	CActiveScheduler::Add(this);
	IssueReadRequest();
	iListening = ETrue;
}

//
// CTrkSerialCommPort::StopListening
//
// Stop listening for data coming into the serial type communications port
//
void CTrkSerialCommPort::StopListening()
{
	if (iListening)
	{
		Cancel();
		Deque();
	}
	
	iListening = EFalse;
}

//
// CTrkSerialCommPort::ReportAndLeaveIfErrorL
//
// If an error occurred, print the error information to the screen and bail out
//
void CTrkSerialCommPort::ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc)
{
	if (KErrNone != aError)
	{
		iErrorMessage.Format(_L("%S\r\nError Code: %d\r\n"), &aDesc, aError);
		User::Leave(aError);
	}
}

//
// CTrkSerialCommPort::IssueReadRequest
//
// Wait for data to come into the communications port
//
void CTrkSerialCommPort::IssueReadRequest()
{
	iNextReadChar = 0;
	iReceivedChars.Zero();
	iPort.ReadOneOrMore(iStatus, iReceivedChars);

	SetActive();
}

//
// CTrkSerialCommPort::DoCancel
//
// Cancel the request for data from the communications port
//
void CTrkSerialCommPort::DoCancel()
{
	iPort.ReadCancel();
}

//
// CTrkSerialCommPort::RunL
//
// Called when data comes into the communications port
//
void CTrkSerialCommPort::RunL()
{
	//_LIT8(KPrefix, "CarbideToTrk: ");
	//PrintMessage(KPrefix(), iReceivedChars);

	// pass the data onto the framing layer
	if (iStatus.Int() == KErrNone)
		while (iNextReadChar < iReceivedChars.Length())
			iFramingLayer->HandleByte(iReceivedChars[iNextReadChar++]);

	// continue waiting for data
	IssueReadRequest();
}

void CTrkSerialCommPort::PrintMessage(const TDesC8& aPrefix, const TDesC8& aBinaryData) const
	{
	TInt msgLength = aBinaryData.Length();
	//RDebug::Printf("%S - start", &aPrefix);
	for(TInt i=0; i<msgLength; i+=8)
		{
		TBuf8<30> tempBuf;
		tempBuf.SetLength(0);
		for(TInt j=i; (j<i+8) && (j<msgLength); j++)
			{
			TUint byte = (TUint)aBinaryData[j];
			tempBuf.AppendNumFixedWidth(byte, EHex, 2);
			tempBuf.Append(' ');
			}
		//RDebug::Printf("%S: [%S]", &aPrefix, &tempBuf);
		}
	//RDebug::Printf("%S - end", &aPrefix);

	}
