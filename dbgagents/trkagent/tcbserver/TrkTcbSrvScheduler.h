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

#ifndef __TrkTcbSrvActiveScheduler_H__
#define __TrkTcbSrvActiveScheduler_H__

// System includes
#include <e32base.h>

// User includes

// Type definitions

// Constants

// Enumerations

// Classes referenced


//
// CTrkTcbSrvScheduler (header)
//
//
// The Active Scheduler for the Trk TCB Server thread
//
class CTrkTcbSrvScheduler : public CActiveScheduler
{
	public:										
		static CTrkTcbSrvScheduler* NewLC();

	private:
		CTrkTcbSrvScheduler();
};

#endif //__TrkTcbSrvActiveScheduler_H__
