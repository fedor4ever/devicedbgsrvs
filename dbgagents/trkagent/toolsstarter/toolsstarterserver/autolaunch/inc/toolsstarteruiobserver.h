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

#ifndef __TOOLSSTARTERIDLEOBSERVER_H__
#define __TOOLSSTARTERIDLEOBSERVER_H__


//  INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <e32property.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

class MToolsStarterIdleObserver
    {
public:
    virtual void IdleStateActivated() = 0;
    };

// CLASS
class CToolsStarterIdleObserver : public CActive
    {
public:
    ~CToolsStarterIdleObserver();
    static CToolsStarterIdleObserver* NewL(MToolsStarterIdleObserver* aObserver);

public:
    void StartL();

private:
    CToolsStarterIdleObserver(MToolsStarterIdleObserver* aObserver);
    void ConstructL();

private:
    void RunL();
    void DoCancel();
    
private:
    MToolsStarterIdleObserver*     iObserver;
    RProperty                   iProperty;
    };

#endif // __TOOLSSTARTERIDLEOBSERVER_H__

// End of File

