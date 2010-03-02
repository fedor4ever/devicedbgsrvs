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


#ifndef __TRKDCCCOMMPORT_H__
#define __TRKDCCCOMMPORT_H__

#include "TrkCommPort.h"
#include "TrkDccComm.h"

//
// class CTrkDccCommPort
//
// Defines an interface to a serial type communications port
//
class CTrkDccCommPort : public CTrkCommPort
{
public:

	static CTrkDccCommPort* NewL(TTrkConnData& aTrkConnData, TDes& aErrorMessage);
	~CTrkDccCommPort();

	void OpenPortL();
	void ClosePort();

	void SendDataL(const TDesC8& aData);
	void Listen(CTrkFramingLayer *aFramingLayer);
	void StopListening();

protected:

	void DoCancel();
	void RunL();

private:

	CTrkDccCommPort();
	void ConstructL(TTrkConnData& aTrkConnData, TDes& aErrorMessage);
	void IssueReadRequest();
	void ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc);
	
private:

	TBool iConnected;
	TBool iListening;

	TBuf8<MAXMESSAGESIZE> iReceivedChars;
	TInt iNextReadChar;

	CTrkFramingLayer* iFramingLayer;
	
	RTrkDccDriver iDccDriver;
	
	TBuf<32> iLDD;
	TUint iUnitNumber;
	
	TInt iLineNumber;
	
	TBool iLddLoaded;
};

#endif // __TRKDCCCOMMPORT_H__
