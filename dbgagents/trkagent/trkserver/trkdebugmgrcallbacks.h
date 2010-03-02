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


#ifndef __TRKDEBUGMGRCALLBACKS_H__
#define __TRKDEBUGMGRCALLBACKS_H__
#include <e32std.h>
#include "TrkConnData.h"

/**
*  Abstract callback interface for TRK engine to call into the app for certain situations.
*  All callbacks into the app from TRK engine can be added here.
*/
class MTrkDebugMgrCallbacks
{
    public:

        /**
        * Called by TRK engine when a connection status is changed
        */
        virtual void DebugConnStatusChanged(TTrkConnStatus& aConnStatus) = 0;

        /**
        * Called by TRK engine when debugging is started or stopped.
        */
        virtual void DebuggingStatusChanged(TBool& aDebugging) = 0;
        
      
};

#endif //__TRKENGINECALLBACK_H__
