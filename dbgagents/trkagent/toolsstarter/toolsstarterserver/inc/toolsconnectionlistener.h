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


#ifndef __TOOLSCONNECTIONLISTENER_H__
#define __TOOLSCONNECTIONLISTENER_H__

/**
*  Abstract callback interface for port listeners 
*/
class MToolsConnectionListener
{
    public:

        /**
        * Callback into the implementor when a connection is available
        */
        virtual void ConnectionAvailable() = 0;

        /**
        * This is called when a connection is closed, this can happen for different reasons.
        * One of the reasons could be that the user has unplugged 
        * the USB cable when using the USB connection
        */
        virtual void ConnectionUnAvailable() = 0; 
      
};

#endif //__TOOLSCONNECTIONLISTENER_H__
