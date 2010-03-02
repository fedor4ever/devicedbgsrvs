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


#ifndef __TRKBTSOCKETCOMMPORT_H__
#define __TRKBTSOCKETCOMMPORT_H__

#include <e32base.h>
#include <c32comm.h>
#ifdef __UIQ_BUILD__
#include <BTExtNotifiers.h>
#include <BtSdp.h>
#endif
// Extra stuff for RFComm
#include <es_sock.h>
#include <btdevice.h>
#include <bt_sock.h>

#include <flogger.h>
#include "TrkCommPort.h"
#include "TrkConnectionListener.h"
#include <btsdp.h> 

#include <btextnotifiers.h>


//
// class CTrkBtSocketCommPort
//
// Defines an interface to a serial type communications port
//
class TTrkConnData;

class MSdpAttributeValueVisitor;
class MSdpAgentNotifier;
class CTrkBtSocketCommPort : public CTrkCommPort,   public MSdpAgentNotifier, public MSdpAttributeValueVisitor
{
public:

	static CTrkBtSocketCommPort* NewL(TTrkConnData aTrkConnData, TDes& aErrorMessage, MTrkConnectionListener* aCallback);
	~CTrkBtSocketCommPort();

	void OpenPortL();
	void ClosePort();

	void SendDataL(const TDesC8& aData);
	void Listen(CTrkFramingLayer *aFramingLayer);
	void StopListening();

	void DiscoverPortL();
	// stopes the discovery (aborts it and delete variables used for it) 
	void StopDiscovery();
protected:

	void DoCancel();
	void RunL();

private:

	CTrkBtSocketCommPort();
	void ConstructL(TTrkConnData aTrkConnData, TDes& aErrorMessage,MTrkConnectionListener* aCallback);
	void IssueReadRequest();
	void ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc);
	// from MSdpAgentNotifier:
	void NextRecordRequestComplete(TInt aError, TSdpServRecordHandle aHandle, TInt aTotalRecordsCount);
	void AttributeRequestResult(TSdpServRecordHandle aHandle, TSdpAttributeID aAttrID, CSdpAttrValue* aAttrValue);
	void AttributeRequestComplete(TSdpServRecordHandle aHandle, TInt aError);

	// from MSdpAttributeValueVisitor:
	void VisitAttributeValueL(CSdpAttrValue &aValue, TSdpElementType aType); // Called for processing of each service attribute.
	void StartListL(CSdpAttrValueList&);
	void EndListL();
	void ConnectToPortL();
	
private:

    TUint  iUpdatedPort;
	TUint  iUnitNumber;
	TUint  iRate;

	RCommServ iServer;
	//RComm  iPort;

	TBool iServerStarted;
	TBool iConnected;
	TBool iListening;

	TBuf8<MAXMESSAGESIZE> iReceivedChars;
	TInt iNextReadChar;

	CTrkFramingLayer* iFramingLayer;
 
 	// name of the device connector connects
	THostName iName;
	// address of the device connector connects
	TBTDevAddr iAddr;
	TUUID iCurrUUID; // currently discovered attribute uuid in the service attributes
	TBool iPortValid; // true if the Port was retrieved
	TBool iDiscovering; // true if currently discovering the device (if interested on results of listener functions)
	CSdpSearchPattern* iSearchPattern; // Service discovery search pattern
	CSdpAgent* iAgent; // Service discovery agent      	
	// connecting socket
	RSocket iSock;

	// socket server
    RSocketServ iSocketServ;
    // length of received data
    TSockXfrLength iLen;
	
    MTrkConnectionListener* iConnectionListener;
    
    enum TState
    {
        ENone = 1,
        EConnecting,
		EWaiting,
		ESending,
		EDiscovering,
    };
    TState iState;
};

#endif // __TRKBTSOCKETCOMMPORT_H__
