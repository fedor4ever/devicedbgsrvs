/*
* Copyright (c) 2004-2008 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __TRKCOMMPORTLISTENER_H__
#define __TRKCOMMPORTLISTENER_H__

#include <e32base.h>

#include "TrkUsbConnectionListener.h"


//
// class CTrkCommPortListener
//
// Abstract base class for communications port listener
//
class CTrkCommPortListener : public CActive
{
public:
	
	virtual ~CTrkCommPortListener();
	
	virtual void ListenL() = 0;
	virtual void StopListening() = 0;

protected:
	
	CTrkCommPortListener(MTrkUsbConnectionListener *aConnectionListener, TInt aPriority);
	virtual TInt RunError( TInt aError );

protected:
	
	MTrkUsbConnectionListener* iConnectionListener;
};

inline CTrkCommPortListener::CTrkCommPortListener(MTrkUsbConnectionListener *aConnectionListener, TInt aPriority)
	: CActive(aPriority),
	iConnectionListener(aConnectionListener)
{
}

inline CTrkCommPortListener::~CTrkCommPortListener()
{
}

inline TInt CTrkCommPortListener::RunError(TInt aError)
{
	User::Panic( _L("TRK"), aError );	
	return aError;
}

#endif // __TRKCOMMPORTLISTENER_H__
