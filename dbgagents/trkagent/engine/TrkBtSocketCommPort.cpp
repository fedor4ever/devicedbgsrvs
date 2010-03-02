/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
// Notifier to select a device
#include <btextnotifiers.h>

#ifdef __UIQ_BUILD__
#include <QBTselectdlg.h>
#include <btsdp.h>          // CSdpAgent
#endif

#include "TrkBtSocketCommPort.h"
#include "TrkFramingLayer.h"
#include "TrkConnData.h"


//
// Static helper functions
//

//
//
// CTrkBtSocketCommPort implementation
//
//

//
// CTrkBtSocketCommPort constructor
//
CTrkBtSocketCommPort::CTrkBtSocketCommPort()
	: CTrkCommPort(CActive::EPriorityStandard, ETrkNotConnected),
	  iServerStarted(EFalse),
	  iConnected(EFalse),
	  iListening(EFalse),
	  iCurrUUID(0),
	  iPortValid(EFalse),
	  iDiscovering(EFalse),
	  iSearchPattern(NULL),
	  iAgent(NULL),
	  iConnectionListener(NULL)
{
   iAddr.Reset();
}

//
// CTrkBtSocketCommPort::ConstructL
//
void CTrkBtSocketCommPort::ConstructL(TTrkConnData aTrkConnData, TDes& /*aErrorMessage*/, MTrkConnectionListener* aCallback)
{
	iUnitNumber = aTrkConnData.iPortNumber;
	iUpdatedPort = iUnitNumber;

	CActiveScheduler::Add(this);		
	
	iConnectionListener = aCallback;
	
}

//
// CTrkBtSocketCommPort destructor
//
CTrkBtSocketCommPort::~CTrkBtSocketCommPort()
{
	// make sure we cancel the request for data before shutting down
	Cancel();
	StopDiscovery();
	
	if (iConnected)
	{
		iSock.Close();
		iConnected = EFalse;
	}
	
	if (iServerStarted)
	{
		iSocketServ.Close();
		iServerStarted = EFalse;
	}	
	

}

//
// CTrkBtSocketCommPort::NewL
//
CTrkBtSocketCommPort* CTrkBtSocketCommPort::NewL(TTrkConnData aTrkConnData, TDes& aErrorMessage, MTrkConnectionListener* aCallback)
{
	CTrkBtSocketCommPort* self = new(ELeave) CTrkBtSocketCommPort;
	CleanupStack::PushL(self);
	self->ConstructL(aTrkConnData, aErrorMessage, aCallback);
	CleanupStack::Pop(self);
	return self;
}

//
// CTrkBtSocketCommPort::OpenPortL
//
// Open the bluetooth selection dialog and connect to the selected device.
//
void CTrkBtSocketCommPort::OpenPortL()
{
	if (!iServerStarted)
	{
		StartC32();
		ReportAndLeaveIfErrorL(iSocketServ.Connect(), _L("Failed to connect to socketserver."));
		iServerStarted = ETrue;
	}

#ifdef __UIQ_BUILD__
	CBTDeviceArray* aSelectedDeviceArray = new (ELeave) CArrayPtrFlat<CBTDevice>(4); 
	CleanupStack::PushL(aSelectedDeviceArray); 
	CQBTUISelectDialog* dialog = CQBTUISelectDialog::NewL(aSelectedDeviceArray); 
	TInt err = dialog->RunDlgLD(KQBTUISelectDlgFlagNone);
	if (err == KErrNone)
	{ 
		CBTDevice* ptrBTDev = aSelectedDeviceArray->operator[](0); 

	  	TBTDeviceResponseParamsPckg devRespParamsPckg;
		devRespParamsPckg().SetDeviceAddress(ptrBTDev->BDAddr()); 
		devRespParamsPckg().SetDeviceName(ptrBTDev->FriendlyName()); 
		devRespParamsPckg().SetDeviceClass(ptrBTDev->DeviceClass()); 

		TBuf8<100> tmpName = ptrBTDev->DeviceName();
		iName.Copy(tmpName);
		iAddr = devRespParamsPckg().BDAddr();
	}
	else
	{ 
		ReportAndLeaveIfErrorL(err, _L("Failed to get device list."));
	}
	CleanupStack::Pop(aSelectedDeviceArray); 
#else
	// Try to open bluetooth selection dialog to get the name and address of the device
	TBTDeviceResponseParamsPckg aResponse;
    
    RNotifier notifier;
    User::LeaveIfError(notifier.Connect());
  
    TBTDeviceSelectionParamsPckg selectionFilter;

    TRequestStatus status;
    notifier.StartNotifierAndGetResponse(
        status,
        KDeviceSelectionNotifierUid,
        selectionFilter,
        aResponse
    );

    User::WaitForRequest(status);

    if (status.Int() != KErrNone)
    {
        notifier.CancelNotifier(KDeviceSelectionNotifierUid);
	    notifier.Close();
	
     	ReportAndLeaveIfErrorL(status.Int(), _L("Failed to get device list."));
    }

    notifier.CancelNotifier(KDeviceSelectionNotifierUid);
    notifier.Close();
    
    iName = aResponse().DeviceName();
    iAddr = aResponse().BDAddr();
#endif

	// load protocol, RFCOMM
	TProtocolDesc pdesc;
	ReportAndLeaveIfErrorL(iSocketServ.FindProtocol(_L("RFCOMM"), pdesc),_L("Unable to find protocol."));

	// open socket
	ReportAndLeaveIfErrorL(iSock.Open(iSocketServ, _L("RFCOMM")),_L("Unable to open socket."));
	// set address and port
    iState = EDiscovering;
    DiscoverPortL();
    iConnectionStatus = ETrkConnecting;
}

/*
 * To connect to BT socket after getting appropriate port
 * 
 */
void CTrkBtSocketCommPort::ConnectToPortL()
{
	TBTSockAddr addr;
	addr.SetBTAddr(iAddr);
    addr.SetPort(iUpdatedPort);

	TRequestStatus cstatus;
	iSock.Connect(addr, cstatus);
	User::WaitForRequest(cstatus);

    TInt err = cstatus.Int();    
    if (err != KErrNone)
    {
        iErrorMessage.Format(_L("Error connecting socket\r\nError Code: %d\r\n"),err);
        iConnectionStatus = ETrkConnectionError;
        iConnectionListener->AsyncConnectionFailed();
        return;
	}
		
	iConnected = ETrue;
    iConnectionMessage.Format(_L("BT Dev Name: %S\r\nBT Port number: %d\r\n"), &iName, iUpdatedPort);
		
    iConnectionStatus = ETrkConnected;
    iConnectionListener->AsyncConnectionSuccessfulL();    
}

//
// CTrkBtSocketCommPort::ClosePort
//
// Close the communications port
//
void CTrkBtSocketCommPort::ClosePort()
{
	Cancel();
	
	if (iConnected)
	{
		iSock.Close();
		iConnected = EFalse;
	}

	if (iServerStarted)
	{
		iSocketServ.Close();
		iServerStarted = EFalse;
	}
}

//
// CTrkBtSocketCommPort::SendDataL
//
// Write data to the serial type port
//
void CTrkBtSocketCommPort::SendDataL(const TDesC8& aBuffer)
{
	iSock.CancelRead();
	iState = ESending;

	iSock.Write(aBuffer, iStatus);
	User::WaitForRequest(iStatus);
	
	if (iStatus!=KErrNone)
	{
		ReportAndLeaveIfErrorL(iStatus.Int(), _L("Error sending data."));
	}	
}

//
// CTrkBtSocketCommPort::Listen
//
// Start listening for data coming into the serial type communications port
//
void CTrkBtSocketCommPort::Listen(CTrkFramingLayer *aFramingLayer)
{
	iFramingLayer = aFramingLayer;
	iListening = ETrue;
}

//
// CTrkBtSocketCommPort::StopListening
//
// Stop listening for data coming into the serial type communications port
//
void CTrkBtSocketCommPort::StopListening()
{
	if (iListening)
	{
		iSock.CancelRead();
		Cancel();
		Deque();
	}
	
	iListening = EFalse;
}

//
// CTrkBtSocketCommPort::ReportAndLeaveIfErrorL
//
// If an error occurred, print the error information to the screen and bail out
//
void CTrkBtSocketCommPort::ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc)
{
	if (KErrNone != aError)
	{
		iErrorMessage.Format(_L("%S\r\nError Code: %d\r\n"), &aDesc, aError);
		User::Leave(aError);
	}
}


//
// CTrkBtSocketCommPort::IssueReadRequest
//
// Wait for data to come into the communications port
//
void CTrkBtSocketCommPort::IssueReadRequest()
{
	iSock.CancelRead();
	iNextReadChar = 0;
	iState = EWaiting;
	iReceivedChars.Zero();
	iSock.RecvOneOrMore(iReceivedChars, 0, iStatus, iLen);
	SetActive();
}

//
// CTrkBtSocketCommPort::DoCancel
//
// Cancel the request for data from the communications port
//
void CTrkBtSocketCommPort::DoCancel()
{
	//iSock.CancelRead();
	//iPort.ReadCancel();
}

//
// CTrkBtSocketCommPort::RunL
//
// Called when data comes into the communications port
//
void CTrkBtSocketCommPort::RunL()
{
	if (iStatus != KErrNone)
	{
		switch(iState)
		{
		   case EDiscovering:
		   {
		       iConnectionListener->AsyncConnectionFailed();
		       break;	       
		   }	       
		   default:
		 	  break;
		}
	}
	else
	{
		switch (iState)
		{
	
			case EWaiting:
			{
				if (iStatus.Int() == KErrNone)
				{
					while (iNextReadChar < iReceivedChars.Length())
						iFramingLayer->HandleByte(iReceivedChars[iNextReadChar++]);
				}
				break;
			}
			case ESending:
			{
				break;
			}
			case EDiscovering:
			{
				ConnectToPortL();
				break;
			}
			case EConnecting:
			{
				break;
			}
			default:
				break;
		}
		IssueReadRequest();
	}

}

//
// CTrkBtSocketCommPort::DiscoverPortL
//
// Called to figure out the port number for serial profile
//
void CTrkBtSocketCommPort::DiscoverPortL()
{
    StopDiscovery(); // to avoid conflicts if called several times before the old discovery is finished
   
    const TInt BT_UUID_SPP_SP = 0x1101; // SPP (Serial Port) UUID
    
    // Init new service discovery agent
    iAgent = CSdpAgent::NewL( *this, iAddr);
    
    // Set search properties for agent (use SPP service-UUID to filter the services discovered)
    iSearchPattern = CSdpSearchPattern::NewL();
    
    iSearchPattern->AddL(BT_UUID_SPP_SP);
    iAgent->SetRecordFilterL(*iSearchPattern);
    iDiscovering = ETrue;
   
    // Initiate search, result will be received with call of NextRecordRequestComplete()
    iAgent->NextRecordRequestL();
    iStatus = KRequestPending;
    SetActive();       
}

//
//CTrkBtSocketCommPort::StopDiscovery()
//
//Called to stop the discovery services
//
void CTrkBtSocketCommPort::StopDiscovery()
{
    iDiscovering = EFalse;
    
    if (iAgent)
    {
        iAgent->Cancel();
        delete iAgent;
        iAgent = NULL;
    }
    
    if (iSearchPattern)
    {
        iSearchPattern->Reset();
        delete iSearchPattern;
        iSearchPattern = NULL;
    }
}


//
// CTrkBtSocketCommPort::NextRecordRequestComplete
//
// called when the service discovery agent has completed discovering services on device (i.e. if next service found or not)
//
void CTrkBtSocketCommPort::NextRecordRequestComplete(TInt aError, TSdpServRecordHandle aHandle, TInt aTotalRecordsCount)
{
    if ( (KErrNone == aError) && (0 < aTotalRecordsCount) )
    {
        // We got next service record, request protocol descriptor to retrieve remote port
        // (it calls later the AttributeRequestResult() when the answer is ready)
        TRAPD(err, iAgent->AttributeRequestL(aHandle, KSdpAttrIdProtocolDescriptorList));
        if( err )
        {
	        iErrorMessage.Format(_L("Error in finding the serial port record\r\nError Code: %d\r\n"),err);
	        iConnectionStatus = ETrkConnectionError;
	        StopDiscovery();
	        TRequestStatus *status = &iStatus;
	        User::RequestComplete(status, aError);
        }
    }
    else
    {
	    // no any services found or error occures (probably the device is not present or SPP service is not present)
	    iErrorMessage.Format(_L("Error in finding the serial port record\r\nError Code: %d\r\n"),aError);
	    iConnectionStatus = ETrkConnectionError;
	    StopDiscovery();
	    TRequestStatus *status = &iStatus;
	    User::RequestComplete(status, aError);  
    }    

}


//
//CTrkBtSocketCommPort::AttributeRequestResult
//
// Called when the service attributes for the service record have been retrieved.
//
void CTrkBtSocketCommPort::AttributeRequestResult(TSdpServRecordHandle, TSdpAttributeID, CSdpAttrValue* aAttrValue)
{
    // can't ignore the call of this function because we need take care about aAttrValue
    // Parse attributes, it will return results by several calls of VisitAttributeValue()
    TRAPD(err, aAttrValue->AcceptVisitorL(*this));
    if( err )
    {   
	    iErrorMessage.Format(_L("Error in finding the serial port record \r\nError Code: %d"),err);
	    iConnectionStatus = ETrkConnectionError;
	    StopDiscovery();
	    TRequestStatus *status = &iStatus;
	    User::RequestComplete(status, err);
    }
    delete aAttrValue;
}

//
// CTrkBtSocketCommPort::AttributeRequestComplete
//
// Called when the request to resolve the service attributes for the service record completes.
//
void CTrkBtSocketCommPort::AttributeRequestComplete(TSdpServRecordHandle, TInt aError)
{
    // if KErrNone ==aError and we need not just the first SPP service but check several services,
    // can call for iAgent->NextRecordRequestL() here (and add corresponded processing in
    // NextRecordRequestComplete() )
    if (KErrNone != aError)
    {
        iErrorMessage.Format(_L("Error in finding the serial port record\r\nError Code: %d"),aError);
        iConnectionStatus = ETrkConnectionError;
        StopDiscovery();
        TRequestStatus *status = &iStatus;
        User::RequestComplete(status, aError);
    }
    else
    {
        StopDiscovery();
        TRequestStatus *status = &iStatus;
        User::RequestComplete(status, KErrNone);  
    }
}
    
//
// CTrkBtSocketCommPort::VisitAttributeValueL
//    
// Called for processing of each service attribute.
//
void CTrkBtSocketCommPort::VisitAttributeValueL(CSdpAttrValue &aValue, TSdpElementType aType)
{
    if (!iDiscovering)
    {
        return; // ignore calls to that function if not discovering currently
    }
    
    // Check for attributes of UUID type. 
    // If the UUID is RFCOMM UUID, resolve the value for this attribute,
    // which will be the channel number.
    switch (aType)
    {
        case ETypeUUID: // UUID of attribute, store it 
        {
            TPtrC8 uuid(aValue.UUID().ShortestForm());
            iCurrUUID.SetL(uuid);
            break;
        }
        case ETypeUint: // uint value, check if the current attribute type is KRFCOMM, store the port value if "yes"
        {
            if (iCurrUUID == KRFCOMM)
            {
                iUpdatedPort = aValue.Uint();
                iPortValid = TRUE;
                iDiscovering = EFalse;
            }
            break;
        }
        default:
            // other attributes are not interesting for processing...
            break;
    }   
}

/*
 * This method should be overridden
 */
void CTrkBtSocketCommPort::StartListL(CSdpAttrValueList&)
{
}

/*
 * This method should be overridden 
 * 
 */
void CTrkBtSocketCommPort::EndListL()
{
}
