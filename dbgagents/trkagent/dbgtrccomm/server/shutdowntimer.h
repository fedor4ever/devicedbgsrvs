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


#ifndef _SHUTDOWNTIMER_H
#define _SHUTDOWNTIMER_H

/**
@file
@internalTechnology
@prototype
*/

#include <e32base.h>

const TInt KShutdownDelay = 1000000; // approx 1 second
const TInt KActivePriorityShutdown = -1; // priority for shutdown AO

/**
Timer class used to manage shutdown of the server
*/
class CShutdownTimer : public CTimer
{
public:
	CShutdownTimer();
	void ConstructL();
	void Start();
private:
	void RunL();
};

#endif // _SHUTDOWNTIMER_H

