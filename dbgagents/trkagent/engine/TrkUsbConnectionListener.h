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


#ifndef TRKUSBCONNECTIONLISTENER_H_
#define TRKUSBCONNECTIONLISTENER_H_

/**
*  Abstract callback interface for port listeners to call into the debug manager for certain situations.
*/
class MTrkUsbConnectionListener
{
    public:

        /**
        * Called by the port listeners when a connection is available
        */
        virtual void ConnectionAvailable() = 0;

        /**
        * This is called when a connection is closed, this can happen for different reasons.
        * One of the reasons could be that the user has unplugged 
        * the USB cable when using the USB connection
        */
        virtual void ConnectionUnAvailable() = 0; 
            
};
#endif /* TRKUSBCONNECTIONLISTENER_H_ */
