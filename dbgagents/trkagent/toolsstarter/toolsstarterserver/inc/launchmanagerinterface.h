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


#ifndef LAUNCHMANAGERINTERFACE_H_
#define LAUNCHMANAGERINTERFACE_H_

#include "sessionmessgelistener.h"

class ILaunchManagerInterface 
{
  public:

    virtual void RegisterListener(MConnStateListener* aListener)=0;
    virtual void UnregisterListener(MConnStateListener* aListener)=0;
    virtual TConnectionStatus GetUsbConnStatus()=0;
    virtual void Launch()=0;
    
     
};

#endif /* LAUNCHMANAGERINTERFACE_H_ */
