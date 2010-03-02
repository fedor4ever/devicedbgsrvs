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


#ifndef __TRKCONNECTIONLISTENER_H__
#define __TRKCONNECTIONLISTENER_H__

/**
*  Abstract callback interface for port listeners to call into the engine for certain situations.
*/
class MTrkConnectionListener
{
    public:
        /*
         * This is called when the BT socket has find the appropriate port/ 
         * 
        */
        virtual void AsyncConnectionSuccessfulL() = 0;
        
        /*
         * This is called to update the UI when the PC relinquishes the connection
         * 
        */
        virtual void AsyncConnectionFailed() = 0;
      
};

#endif //__TRKCONNECTIONLISTENER_H__

