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


#ifndef __TOOLSCOMMPORTLISTENER_H__
#define __TOOLSCOMMPORTLISTENER_H__

#include <e32base.h>

#include "toolsconnectionlistener.h"


//
// class CToolsCommPortListener
//
// Abstract base class for communications port listener
//
class CToolsCommPortListener : public CActive
{
public:
	
	virtual ~CToolsCommPortListener();
	
	virtual void ListenL() = 0;
	virtual void StopListening() = 0;

protected:
	
	CToolsCommPortListener(MToolsConnectionListener *aConnectionListener, TInt aPriority);
	virtual TInt RunError( TInt aError );

protected:
	
	MToolsConnectionListener* iConnectionListener;
};

inline CToolsCommPortListener::CToolsCommPortListener(MToolsConnectionListener *aConnectionListener, TInt aPriority)
	: CActive(aPriority),
	iConnectionListener(aConnectionListener)
{
}

inline CToolsCommPortListener::~CToolsCommPortListener()
{
}

inline TInt CToolsCommPortListener::RunError(TInt aError)
{
	User::Panic( _L("TOOLSSTARTER"), aError );	
	return aError;
}

#endif // __TOOLSCOMMPORTLISTENER_H__
