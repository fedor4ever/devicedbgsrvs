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


#ifndef __TRKXTIDRIVER_H__
#define __TRKXTIDRIVER_H__

#include "TrkXtiSubscriber.h"

// Debug messages
#ifdef _DEBUG
#define LOG_MSG(x) Kern::Printf(x)
#define LOG_MSG2(x, y) Kern::Printf(x, y)
#else
#define LOG_MSG(x)
#define LOG_MSG2(x, y)
#endif

//class DTrkXtiSubscriber;
//
// Macros
//
#define MAXMESSAGESIZE	0x0900
#define KTRKXTIDRIVERREQ 0x44;
const TInt kIntTrkMessageId = 0x44;

//
// class DTrkXtiDriverFactory
//
class DTrkXtiDriverFactory : public DLogicalDevice
{
public:

	DTrkXtiDriverFactory();
	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DLogicalChannelBase*& aChannel);
};

//
// DTrkXtiChannel
//
class DTrkXtiChannel : public DLogicalChannel					  								
{
public:

	DTrkXtiChannel(DLogicalDevice* aLogicalDevice);
	~DTrkXtiChannel();

	virtual TInt DoCreate(TInt aUnit, const TDesC* anInfo, const TVersion& aVer);	
	virtual void HandleMsg(TMessageBase* aMsg);
    
    void DoCompleteRx(const TDesC8& aMessage);	
	
	
protected: // from DLogicalChannel
	virtual void DoCancel(TInt aReqNo);
	virtual void DoRequest(TInt aReqNo, TRequestStatus* aStatus, TAny* a1, TAny* a2);
	virtual TInt DoControl(TInt aFunction, TAny *a1, TAny *a2);
	
	
private:
	TInt DoWrite(TUint32 aLength, TAny* a2);
	TInt DoRead();

	TInt WriteXtiChannel(TDesC8& aDes);
	void ReadXtiChannel();

		
private:
		
	DThread* iClientThread;
	
	TRequestStatus* iRequestGetEventStatus;
	
	TAny* iRxClientBuffer;
	TUint32 iRxClientBufferLength;

	TBuf8<MAXMESSAGESIZE> iTxBuffer;
	TBuf8<MAXMESSAGESIZE> iRxPendingMsgBuffer;
	TBool iRxPendingMsg;
	
	TRequestStatus* iRxRequestStatus;
	
	TInt iInterruptId;
	
	DSemaphore* iLock;
	
	TDfcQue* iDFCQue;
	
	DTrkXtiSubscriber *iTrkXtiSubscriber;
};

#endif //__TRKXTIDRIVER_H__
