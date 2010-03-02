/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __TRKENGINECALLBACK_H__
#define __TRKENGINECALLBACK_H__

/**
*  Abstract callback interface for TRK engine to call into the app for certain situations.
*  All callbacks into the app from TRK engine can be added here.
*/
class MTrkEngineCallback
{
    public:

        /**
        * Called by TRK engine when a connection is open
        */
        virtual void OnConnection() = 0;

        /**
        * This is called when a connection is closed, this can happen for different reasons.
        * One of the reasons could be that the user has unplugged 
        * the USB cable when using the USB connection
        */
        virtual void OnCloseConnection() = 0; 
        
        /**
        * Called by TRK engine when a debug session is started
        */
        virtual void DebuggingStarted() = 0;

        /**
        * Called by TRK engine when a debug session ends
        */
        virtual void DebuggingEnded() = 0;
        
        /*
         * called when the PC rejects the BT connection 
         * 
         */
        virtual void OnAsyncConnectionFailed() = 0;
      
};

#endif //__TRKENGINECALLBACK_H__
