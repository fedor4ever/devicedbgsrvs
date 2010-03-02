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

#include <e32base.h>

// User includes
#include "dbgtrcsrvscheduler.h"


//
// CDbgTrcSrvScheduler (source)
//

//
// CDbgTrcSrvScheduler::CDbgTrcSrvScheduler()
//
// Constructor
//
CDbgTrcSrvScheduler::CDbgTrcSrvScheduler()
{
}


//
// CDbgTrcSrvScheduler::NewLC()
//
// Creates an instance of CDbgTrcSrvScheduler
//
CDbgTrcSrvScheduler* CDbgTrcSrvScheduler::NewLC()
{
	CDbgTrcSrvScheduler* self = new(ELeave) CDbgTrcSrvScheduler();
	CleanupStack::PushL(self);
	return self;
}
