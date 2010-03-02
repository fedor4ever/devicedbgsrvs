/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __TRKXTICOMMPORT_H__
#define __TRKXTICOMMPORT_H__

#include "TrkCommPort.h"
#include "TrkXtiComm.h"

//
// class CTrkXtiCommPort
//
// Defines an interface to Trace Core 
//
class CTrkXtiCommPort : public CTrkCommPort
{
public:

	static CTrkXtiCommPort* NewL(TTrkConnData aTrkConnData, TDes& aErrorMessage);
	~CTrkXtiCommPort();

	void OpenPortL();
	void ClosePort();

	void SendDataL(const TDesC8& aData);
	void Listen(CTrkFramingLayer *aFramingLayer);
	void StopListening();

protected:

	void DoCancel();
	void RunL();

private:

	CTrkXtiCommPort();
	void ConstructL(TTrkConnData aTrkConnData, TDes& aErrorMessage);
	void IssueReadRequest();
	void ReportAndLeaveIfErrorL(TInt aError, const TDesC& aDesc);
	
private:

	TBool iConnected;
	TBool iListening;

	TBuf8<MAXMESSAGESIZE> iReceivedChars;
	TInt iNextReadChar;

	CTrkFramingLayer* iFramingLayer;
	
	RTrkXtiDriver iXtiDriver;
	
	TBuf<32> iLDD;
//	TUint iUnitNumber;
	
	TInt iLineNumber;
	
	TBool iLddLoaded;
};

#endif // __TRKXTICOMMPORT_H__
