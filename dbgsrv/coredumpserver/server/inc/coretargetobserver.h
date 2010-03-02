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
 @internalTechnology
 @released
*/


#ifndef CORE_TARGET_OBSERVER_H
#define CORE_TARGET_OBSERVER_H

#include <e32base.h>
#include <e32debug.h>
#include <rm_debug_api.h> //RSecuritySvrSession
#include "corecrashhandler.h"

using namespace Debug;

/** 
Class that tracks the observation of targets, bot processes and threads within
@internalComponent
*/
class CTargetObserver : public CActive
{
public:
    static CTargetObserver* NewL(RSecuritySvrSession &aSession, CCrashHandler &aHandler, const TDesC& aProcessName);
    static CTargetObserver* NewLC(RSecuritySvrSession &aSession, CCrashHandler &aHandler, const TDesC& aProcessName);
    ~CTargetObserver();

	void Observe();
    void SetCrashEventL(TEventType aType, TKernelEventAction aAction);

    const TDesC& TargetName() const;

    TInt ThreadCount() const;
    void AddThreadL(const TDesC &aThreadName);
    void DelThreadL(const TDesC &aThreadName);
    void ClearThreads();
    TBool HasThread(const TDesC &aThreadName) const;
    const TDesC& Thread(TInt aIndex) const;

protected:
   	CTargetObserver(RSecuritySvrSession &aSession, CCrashHandler &iHandler);
	void ConstructL(const TDesC& aProcessName);

	virtual void RunL();
	virtual void DoCancel();
	virtual TInt RunError(TInt aError);
 
private:
    RSecuritySvrSession &iSecSess; 
    CCrashHandler &iHandler; 
    RBuf iTargetName; //process name
    RPointerArray<HBufC> iThreadList; //threads name array 
    TCrashEventInfo iCrashEventInfo; //crash event info struct
    TTime iEventTime; //crash event time
    TPtr8 iEventInfoPtr; //descriptor wrapper for crash event info
};
#endif //CORE_TARGET_OBSERVER_H
