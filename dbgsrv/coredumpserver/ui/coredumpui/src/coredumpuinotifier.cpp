// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Core dump progress observer active object class implementation.
//

#include "coredumpuinotifier.h"

CCrashNotifier* CCrashNotifier::NewL(CResourceView &aResourceView)
    {
    CCrashNotifier* self = new(ELeave) CCrashNotifier(aResourceView);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
 * Adds itself into the active scheduler.
 */
CCrashNotifier::CCrashNotifier(CResourceView &aResourceView)
    :CActive(EPriorityStandard),
    iResourceView(aResourceView)
    {
    CActiveScheduler::Add(this);
    }

/**
 * Attaches to the core dump server crash progress property. And starts observation right away.
 */
void CCrashNotifier::ConstructL()
    {
    LOG_MSG3("CCrashNotifier::ConstrucL() - attach %d.%d", KCoreDumpServUid, ECrashProgress);
    User::LeaveIfError(iProperty.Attach(KCoreDumpServUid, ECrashProgress));
    RunL(); //start watching
    }

/**
 * Cancels the wait for outstanding request and closes handle to crash progress property. 
 */
CCrashNotifier::~CCrashNotifier()
    {
    Cancel();
    iProperty.Close();
    }

/**
 * Cancels subscription to the crash progress property.
 */
void CCrashNotifier::DoCancel()
    {
    iProperty.Cancel();
    }

/**
 * Handles completion of observation request, notifies the UI and reissues another observation request.
 */
void CCrashNotifier::RunL()
    {
    LOG_MSG( "CCrashNotifier::RunL()" );
    User::LeaveIfError(iProperty.Get(iResourceView.CrashProgress()));
    iResourceView.UpdateCrashProgressL(); 
    iProperty.Subscribe(iStatus); 
    SetActive();
    }
