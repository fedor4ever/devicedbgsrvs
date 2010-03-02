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
//



/**
 @file
 @internalComponent
*/

#include "coretargetobserver.h"

CTargetObserver::CTargetObserver(RSecuritySvrSession &aSecSession, CCrashHandler &aHandler)
    : CActive(CActive::EPriorityStandard),
    iSecSess(aSecSession),
    iHandler(aHandler),
    iEventInfoPtr( (TUint8*)&iCrashEventInfo, 0, sizeof(TEventInfo) ) 
	{
	CActiveScheduler::Add(this);
	}

CTargetObserver* CTargetObserver::NewL(RSecuritySvrSession &aSecSession, CCrashHandler &aHandler, const TDesC &aProcessName)
{
    LOG_MSG("->CTargetObserver::NewL()\n");
	CTargetObserver* self = CTargetObserver::NewLC(aSecSession, aHandler, aProcessName);
	CleanupStack::Pop(self);
    return self;
}

CTargetObserver* CTargetObserver::NewLC(RSecuritySvrSession &aSession, CCrashHandler &aHandler, const TDesC& aProcessName)
	{
    LOG_MSG("->CTargetObserver::NewLC()\n");
	CTargetObserver *self = new (ELeave) CTargetObserver(aSession, aHandler);
	CleanupStack::PushL(self);
	self->ConstructL(aProcessName);
	return self;
	}

void CTargetObserver::ConstructL(const TDesC& aTargetName)
	{
    LOG_MSG("->CTargetObserver::ConstructL()\n");

	iTargetName.CreateL(aTargetName.Length());
	iTargetName.Copy(aTargetName);
    
	LOG_MSG("CTargetObserver::Observe -> AttachExecutable\n");
    User::LeaveIfError(iSecSess.AttachExecutable( iTargetName, EFalse));
	}

void CTargetObserver::SetCrashEventL(TEventType aType, TKernelEventAction aAction)
{
	LOG_MSG3("->CTargetObserver::SetCrashEventL(type=%d, action=%d)\n", aType, aAction );
    User::LeaveIfError( iSecSess.SetEventAction( iTargetName, aType, aAction ));
}

CTargetObserver::~CTargetObserver()
	{
    LOG_MSG("->CTargetObserver::~CTargetObserver()\n");
	Cancel();
	iTargetName.Close();
    iThreadList.ResetAndDestroy();
	}

void CTargetObserver::DoCancel()
	{
	LOG_MSG( "CTargetObserver::DoCancel -> CancelGetEvent\n");
    TInt err = iSecSess.CancelGetEvent( iTargetName );
    if(err != KErrNone)
        {
        LOG_MSG2( "CTargetObserver::DoCancel - iSecSess.DetachExecutable returned:%d!", err);
        //panic client?? close DSS session?? 
        }

	LOG_MSG("CTargetObserver::DoCancel -> iSecSess.DetachExecutable\n");
    err = iSecSess.DetachExecutable( iTargetName );
    if(err != KErrNone)
        {
        LOG_MSG2( "CTargetObserver::DoCancel iSecSess.DetachExecutable returned:%d!\n", err);
        //panic client?? close DSS session?? 
        }
	}

const TDesC& CTargetObserver::TargetName() const
{
    return iTargetName;
}

TInt CTargetObserver::ThreadCount() const
{
    return iThreadList.Count();
}

const TDesC& CTargetObserver::Thread(TInt aIndex) const
{
    LOG_MSG2("CTargetObserver::Thread(%d)\n", aIndex);
    return *iThreadList[aIndex];
}

void CTargetObserver::AddThreadL(const TDesC &aThreadName)
{
    LOG_MSG("->CTargetObserver::AddThreadL()\n");

    if(HasThread(aThreadName))
    {
        LOG_MSG("->CTargetObserver::AddThreadL() - already exists!\n");
        User::Leave(KErrAlreadyExists);
    }
    
    HBufC *thread;
    thread = HBufC::NewL(aThreadName.Length());
    *thread = aThreadName;
    iThreadList.AppendL(thread);
    LOG_MSG("CTargetObserver::AddThreadL - thread added\n");
}

TBool CTargetObserver::HasThread(const TDesC &aThreadName) const
{
    LOG_MSG("->CTargetObserver::HasThread()");
    TInt count = iThreadList.Count();
    for(TInt i = 0; i < count; ++i)
    {
        if(iThreadList[i]->Des() == aThreadName)
        {
            LOG_MSG("CTargetObserver::HasThreadL - thread found");
            return ETrue;
        }
    }
    LOG_MSG("CTargetObserver::HasThreadL - thread not found");
    return EFalse;
}

void CTargetObserver::DelThreadL(const TDesC &aThreadName)
{
    LOG_MSG("->CTargetObserver::DelThreadL()");

    TInt count = iThreadList.Count();
    for(TInt i = 0; i < count; ++i)
    {
        if(iThreadList[i]->Des() == aThreadName)
        {
            delete iThreadList[i];
            iThreadList.Remove(i);
            LOG_MSG("CTargetObserver::DelThreadL - thread deleted");
            return;
        }
    }

    LOG_MSG("CTargetObserver::DelThreadL - thread not found!");
    User::Leave(KErrNotFound);
}

void CTargetObserver::ClearThreads()
{
    LOG_MSG("->CTargetObserver::ClearThreadsL()\n");
    iThreadList.ResetAndDestroy();
}

// RunL() completes a previously issued Observe call 
void CTargetObserver::RunL()
	{
    LOG_MSG2("->CTargetObserver::RunL(status:%d)", iStatus.Int());
	User::LeaveIfError(iStatus.Int()); //something bad happened

    iCrashEventInfo.iEventTime.UniversalTime(); //not 100% exact time of the crash, but as soon as we are notified about it

    Observe(); 

    RThread thread;
    LOG_MSG2("CTargetObserver::RunL() - opening handle to crashed thread:%Lu\n", iCrashEventInfo.iThreadId);
    TInt err = thread.Open(iCrashEventInfo.iThreadId);
    if(err != KErrNone)
        {
        LOG_MSG2("CTargetObserver::RunL - unable to open thread handle! err:%d\n", err); 
        User::Leave(err);
        }
    CleanupClosePushL(thread); 
    
    if( (iThreadList.Count() == 0) || (HasThread(thread.FullName())) )
        {
        //crash event of the whole process or thread that we observe
        LOG_MSG("CTargetObserver::RunL() -> HandleCrashEventL()");
        iHandler.HandleCrashEventL(iCrashEventInfo);
        }
    else //crash event of thread that we don't care about
        {
        LOG_MSG("CTargetObserver::RunL() - resuming crashed thread");
        iSecSess.ResumeThread(iCrashEventInfo.iThreadId);
        }
    CleanupStack::PopAndDestroy(); //thread

	}


// Report any leave to the client if possible.
TInt CTargetObserver::RunError(TInt aError)
	{
	return KErrNone;
	}

void CTargetObserver::Observe()
	{
	LOG_MSG("->CTargetObserver::Observe()\n");
	iSecSess.GetEvent( iTargetName, iStatus, iEventInfoPtr );
    SetActive(); //wait for crash event
	}
