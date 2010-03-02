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


#ifndef SESSIONMESSGELISTENER_H_
#define SESSIONMESSGELISTENER_H_

#include "connectionvariables.h"

class MConnStateListener
    {
public:
    virtual void ConnStatusChanged(TConnectionStatus)=0;   
    
    };


#endif /* SESSIONMESSGELISTENER_H_ */


