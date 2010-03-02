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

#ifndef __ToolsSrvActiveScheduler_H__
#define __ToolsSrvActiveScheduler_H__

// System includes
#include <e32base.h>



//
// CToolsSrvScheduler (header)
//
//
// The Active Scheduler for the Trk TCB Server thread
//
class CToolsSrvScheduler : public CActiveScheduler
{
	public:										
		static CToolsSrvScheduler* NewLC();

	private:
		CToolsSrvScheduler();
};

#endif //__ToolsSrvActiveScheduler_H__
