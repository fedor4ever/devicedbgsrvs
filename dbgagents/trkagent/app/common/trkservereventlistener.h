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


#ifndef __TRKSERVEREVENTLISTENER_H__
#define __TRKSERVEREVENTLISTENER_H__

#include <e32base.h>

#include "trkservereventcallback.h"
//
// class CTrkServerEventListener
//
// Abstract base class for trk server event listener
//
class RTrkSrvCliSession;

class CTrkServerEventListener : public CActive
{
public:
	
	virtual ~CTrkServerEventListener();
	
	virtual void ListenL() = 0;
	virtual void StopListening() = 0;

protected:
	
	CTrkServerEventListener(RTrkSrvCliSession& aTrkSrvSession, MTrkServerEventCallback *aTrkServerEventCallback, TInt aPriority);
	virtual TInt RunError( TInt aError );

protected:
    RTrkSrvCliSession& iTrkSrvSession;
	MTrkServerEventCallback* iTrkServerEventCallback;
};

inline CTrkServerEventListener::CTrkServerEventListener(RTrkSrvCliSession& aTrkSrvSession, MTrkServerEventCallback *aTrkServerEventCallback, TInt aPriority)
	: CActive(aPriority),
	iTrkSrvSession(aTrkSrvSession),
	iTrkServerEventCallback(aTrkServerEventCallback)
{
}

inline CTrkServerEventListener::~CTrkServerEventListener()
{
}

inline TInt CTrkServerEventListener::RunError(TInt aError)
{
	User::Panic( _L("TRK"), aError );	
	return aError;
}

#endif // __TRKSERVEREVENTLISTENER_H__


//  End of File
