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


#ifndef __TRKSERIALCOMMPORT_H__
#define __TRKSERIALCOMMPORT_H__

#include <c32comm.h>
#include "TrkCommPort.h"


//
// class CTrkSerialCommPort
//
// Defines an interface to a serial type communications port
//
class CTrkSerialCommPort : public CTrkCommPort
{
public:

	static CTrkSerialCommPort* NewL(TTrkConnData& aTrkConnData, TDes& aErrorMessage);
	~CTrkSerialCommPort();

	void OpenPortL();
	void ClosePort();

	void SendDataL(const TDesC8& aData);
	void Listen(CTrkFramingLayer *aFramingLayer);
	void StopListening();
	
protected:

	void DoCancel();
	void RunL();

private:

	CTrkSerialCommPort();
	void ConstructL(TTrkConnData& aTrkConnData, TDes& aErrorMessage);
	void IssueReadRequest();
	void ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc);
	void PrintMessage(const TDesC8& aPrefix, const TDesC8& aBinaryData) const;
	
private:

	TBuf<32> iPDD;
	TBuf<32> iPDD2;
	TBuf<32> iLDD;
	TBuf<32> iCSY;
	TUint iUnitNumber;
	TUint iRate;

	RCommServ iServer;
	RComm  iPort;

	TBool iServerStarted;
	TBool iConnected;
	TBool iListening;

	TBuf8<MAXMESSAGESIZE> iReceivedChars;
	TInt iNextReadChar;

	CTrkFramingLayer* iFramingLayer;
	
	TInt iLineNumber;
};

#endif // __TRKSERIALCOMMPORT_H__
