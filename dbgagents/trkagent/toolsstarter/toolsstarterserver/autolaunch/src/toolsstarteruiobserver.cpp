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

// INCLUDE FILES
#include <e32debug.h>
#include <ActiveIdle2DomainPSKeys.h>
#include <e32property.h>

#include "logging.h"
#include "toolsstarteruiobserver.h"

// CONSTANTS


// ============================ MEMBER FUNCTIONS ===============================

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
//
CToolsStarterIdleObserver::CToolsStarterIdleObserver(MToolsStarterIdleObserver& aObserver) :
    CActive(EPriorityStandard), // Standard priority
    iObserver(aObserver)
    {
    }

// ----------------------------------------------------------------------------
// Two-phased constructor
// ----------------------------------------------------------------------------
//
CToolsStarterIdleObserver* CToolsStarterIdleObserver::NewL(MToolsStarterIdleObserver& aObserver)
    {
    CToolsStarterIdleObserver* self = new(ELeave) CToolsStarterIdleObserver(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ----------------------------------------------------------------------------
// 2nd phase construction
// ----------------------------------------------------------------------------
//
void CToolsStarterIdleObserver::ConstructL()
    {
    CActiveScheduler::Add(this);
    
    // attach to the property which listens the idle state
    User::LeaveIfError( iProperty.Attach( KPSUidAiInformation, KActiveIdleState ) );
    
    LOG_MSG("CToolsStarterIdleObserver::ConstructL succesfully attached to property");
    }

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
//
CToolsStarterIdleObserver::~CToolsStarterIdleObserver()
    {
    Cancel();
    iProperty.Close();
    }

// ----------------------------------------------------------------------------
// Starts listening for events
// ----------------------------------------------------------------------------
//
void CToolsStarterIdleObserver::StartL()
    {
    Cancel();
    
    // subscribe to the property
    iProperty.Subscribe(iStatus);
    SetActive();
    }

// ----------------------------------------------------------------------------
// Implements cancellation of an outstanding request.
// ----------------------------------------------------------------------------
//
void CToolsStarterIdleObserver::DoCancel()
    {
    iProperty.Cancel();
    }

// ----------------------------------------------------------------------------
// Handles an active object's request completion event.
// ----------------------------------------------------------------------------
//
void CToolsStarterIdleObserver::RunL()
    {
    // check the status
    TInt idleStatus(KErrNotFound);
    TInt err( iStatus.Int() );
    
    if (err == KErrNone)
        {
        // resubscribe before processing new value to prevent missing updates
        StartL();
        err = iProperty.Get( KPSUidAiInformation, KActiveIdleState, idleStatus );
        
        if (err == KErrNone && idleStatus == EPSAiForeground) 
            {
            LOG_MSG("CToolsStarterIdleObserver::RunL idle foreground state detected");

            // idle state is active so notify the observer and no need to wait anymore
            iObserver.IdleStateActivated();
            Cancel();
            }
        else
            {
            LOG_MSG2("CToolsStarterIdleObserver::RunL unknown state %d", idleStatus);
            }
        }

    else if (err != KErrCancel)
        {
        // try again..
        StartL();
        } 
    }

// ----------------------------------------------------------------------------

// End of File
