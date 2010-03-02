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


#ifndef __TRKFRAMINGLAYER_H__
#define __TRKFRAMINGLAYER_H__

#include "TrkCommPort.h"

//
// Forward declarations
//
class CTrkDispatchLayer;


//
// class CTrkFramingLayer
//
// Frames outgoing messages to the host and unframes incoming messages
// from the host
//
class CTrkFramingLayer : public CBase
{
public:

	enum TFramingState
	{
		eWaiting,
		eFound,
		eInFrame,
		eFrameOverflow
	};

	CTrkFramingLayer(CTrkCommPort* aPort);
	~CTrkFramingLayer();

	void Listen(CTrkDispatchLayer* aDispatchLayer);
	void StopListening();

	void HandleByte(TUint8 aByte);

	void RespondOkL(const TDesC8& aMsg);
	void RespondErr(TUint8 aType, TUint8 aErr);
	void InformEventL(const TDesC8& aMsg);
	void SendRawMsgL(const TDesC8& aMsg);
	
	TBool IsBigEndian() { return iIsBigEndian; };
	void ResetSequenceIDs() { iSequenceId = 1; iOutSequenceId = 1; };

private:

	void SendMsgL(const TDesC8& aMsg);

	void DiscardFrame();
	TBool ProcessFrame();

	void IncrementSequenceId() { if (iSequenceId == 0xFF) iSequenceId = 1; else iSequenceId++; };
	void IncrementOutSequenceId() { if (iOutSequenceId == 0xFF) iOutSequenceId = 1; else iOutSequenceId++; };

private:

	CTrkCommPort* iPort;
	CTrkDispatchLayer* iDispatchLayer;

	TFramingState iFramingState;
	TBool iEscape;
	TUint8 iFCS;

	TBuf8<MAXMESSAGESIZE> iBuffer;	// contains the incoming messages
	TUint8 iSequenceId;
	TUint8 iOutSequenceId;
		
	TBool iIsBigEndian;
	
	HBufC8* iLastReply;
};

#endif // __TRKFRAMINGLAYER_H__
