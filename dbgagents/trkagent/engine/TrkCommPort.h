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


#ifndef __TRKCOMMPORT_H__
#define __TRKCOMMPORT_H__

#include <e32base.h>
#include "TrkConnData.h"


//
// Macros
//
// these must be defined before including msgcmd.h
#define DS_PROTOCOL	DS_PROTOCOL_RTOS
#define DEBUG_MSGCMD 0

#include "msgcmd.h"

#define SafeDelete(x) { if (x) delete x; x = NULL; }


//
// class CInactivityTimerDisabler
//
// Active object used to avoid auto-switch off
//
class CInactivityTimerDisabler : public CActive
{
public:

	CInactivityTimerDisabler();
	void ConstructL();
	~CInactivityTimerDisabler();
	void Activate();

protected:

	void RunL();
	void DoCancel();

private:

	enum { KPeriod = 5 * 1000000 };

	RTimer iTimer;
};

//
//
// CInactivityTimerDisabler implementation
//
//

inline CInactivityTimerDisabler::CInactivityTimerDisabler()
	: CActive(EPriorityLow)
{
	CActiveScheduler::Add(this);
}

inline void CInactivityTimerDisabler::ConstructL()
{
	User::LeaveIfError(iTimer.CreateLocal());
}

inline CInactivityTimerDisabler::~CInactivityTimerDisabler()
{
	Cancel();
	iTimer.Close();
	Deque();
}

inline void CInactivityTimerDisabler::Activate()
{
	iTimer.After(iStatus, KPeriod);
	SetActive();
}

inline void CInactivityTimerDisabler::RunL()
{
	User::ResetInactivityTime();
	Activate();
}

inline void CInactivityTimerDisabler::DoCancel()
{
	iTimer.Cancel();
}


//
// Forward declarations
class CTrkFramingLayer;


//
// class CTrkCommPort
//
// Abstract base class for communications port
//
class CTrkCommPort : public CActive
{
public:
	
	virtual ~CTrkCommPort();

	virtual void OpenPortL() = 0;
	virtual void ClosePort() = 0;
	virtual void SendDataL(const TDesC8& aData) = 0;
	virtual void Listen(CTrkFramingLayer *aFramingLayer) = 0;
	virtual void StopListening() = 0;
	virtual TBool IsConnectionEstablished() { return iConnectionStatus == ETrkConnected; };
	virtual void GetConnectionInfo(TDes& aMessage) { aMessage = iConnectionMessage; };
	virtual void GetErrorInfo(TDes& aMessage) { aMessage = iErrorMessage; };
	virtual TTrkConnStatus GetConnectionStatus() { return iConnectionStatus; };

protected:

	CTrkCommPort(TInt aPriority, TTrkConnStatus aConnectionStatus);

	TBuf<KMaxPath> iConnectionMessage;
	TBuf<KMaxPath> iErrorMessage;
	TTrkConnStatus iConnectionStatus;
};

inline CTrkCommPort::CTrkCommPort(TInt aPriority,TTrkConnStatus aConnectionStatus=ETrkConnected)
	: CActive(aPriority),
	iConnectionStatus(aConnectionStatus)
{
}

inline CTrkCommPort::~CTrkCommPort()
{
}

#endif // __TRKCOMMPORT_H__
