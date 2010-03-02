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


#ifndef __TRKSERVEREVENTCALLBACK_H__
#define __TRKSERVEREVENTCALLBACK_H__
#include "TrkConnData.h" 
/**
*  Abstract callback interface for TRK event listeners to call into the app for certain situations.
*  All callbacks into the app from TRK event listeners can be added here.
*/
class MTrkServerEventCallback
{
    public:

        /**
        * Called by TRK debug state listener when debugging state changes
        */
        virtual void DebugStateChanged(TBool& aDebugging) = 0;

        /**
        * Called when a connection state changes               
        */
        virtual void ConnectionStateChanged(TTrkConnStatus& aConnected) = 0; 
      
};

#endif //__TRKSERVEREVENTCALLBACK_H__

