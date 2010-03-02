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


#ifndef __TRKDBGTRCCOMMPORT_H__
#define __TRKDBGTRCCOMMPORT_H__

#include "TrkCommPort.h"
#include "dbgtrccomm.h"

//
// class CTrkDbgTrcCommPort
//
// Defines an interface to a serial type communications port
//
class CTrkDbgTrcCommPort : public CTrkCommPort
{
public:

	static CTrkDbgTrcCommPort* NewL(TTrkConnData aTrkConnData, TDes& aErrorMessage);
	~CTrkDbgTrcCommPort();

	void OpenPortL();
	void ClosePort();

	void SendDataL(const TDesC8& aData);
	void Listen(CTrkFramingLayer *aFramingLayer);
	void StopListening();

protected:

	void DoCancel();
	void RunL();

private:

	CTrkDbgTrcCommPort();
	void ConstructL(TTrkConnData aTrkConnData, TDes& aErrorMessage);
	void IssueReadRequest();
	void ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc);
	
private:
	TUint  iUnitNumber;

	TBool iConnected;
	TBool iListening;

	TBuf8<MAXMESSAGESIZE> iReceivedChars;
	TInt iNextReadChar;

	CTrkFramingLayer* iFramingLayer;
		
	TInt iLineNumber;
	
	RDbgTrcComm iDbgTrcPort;
};

#endif // __TRKDBGTRCCOMMPORT_H__
