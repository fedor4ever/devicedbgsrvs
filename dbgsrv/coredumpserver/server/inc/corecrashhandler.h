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

#ifndef CORE_CRASH_HANDLER_H
#define CORE_CRASH_HANDLER_H

#include <e32base.h>
#include <e32debug.h>
#include <formatterapi.h>
#include <debuglogging.h>

class CCoreDumpSession;

const TUid KUidSELFFormatterV1 = TUid::Uid(0x10282fe3);

//Identifier for SELF v1
_LIT(KSelfV1, "Symbian Elf Core Dump formatter");
_LIT(KDexcV1, "D_EXC formatter plugin description");

typedef struct GCCCrashEventInfo : TEventInfo
{
    TTime iEventTime; 
} TCrashEventInfo;

_LIT(KCrashProcessingThread, "core_dump_processor"); //crash processing thread name

/**
Active object class that creates the thread that handles the crash generation
@see CCoreDumpSession
@see CTargetObserver
@see CCoreDumpFormatter
*/
class CCrashHandler : public CActive
{
    static TInt Processing(TAny* aParam); //crash processing thread entry point

    struct TCrashThreadParams //crash processing thread params
	    {
	    CCoreDumpFormatter *iFormatter;
	    TCrashInfo iCrashInfo;
	    };

    enum {EEventsQueueDefaultLength = 1};    

public:
	
	/**
	 * Defines the type of crash we are processing and so what DS to use
	 */
	enum TCrashType
		{
		ELiveCrash,  //!< Live Application Crash
		ESystemCrash,//!< System Crash (ie. Flash)
		ELast        //!< ELast
		};
	
    static CCrashHandler* NewL(CCoreDumpSession &aCoreSess);
    static CCrashHandler* NewLC(CCoreDumpSession &aCoreSess);
    ~CCrashHandler();

    TBool CrashInProgress() const {return iThreadsRun > 0 ? ETrue : EFalse;};
    void HandleCrashEventL(const TCrashEventInfo &aEventInfo);
    void HandleCrashFromFlashL(const TCrashInfo& aCrashInf);
    void HandleCrashFromFlashL(const TCrashInfo& aCrashInf, const RMessage2& aMessage);
    void CancelHandleCrashFromFlash(const TCrashInfo& aCrashInf);

protected:
   	CCrashHandler(CCoreDumpSession &aCoreSess);
	void ConstructL();

	virtual void RunL();
	virtual void DoCancel();
	virtual TInt RunError(TInt aError);

private:
	void ProcessCrashL(const TCrashInfo::TCrashSource& aType);
	void ProcessLiveCrashL();
	void ProcessSystemCrashL();
	
    void StartThreadL();
    void ResetProperties(const TDesC &aProgress);
    void PostProcessL();
 
private:
    TUint iThreadsRun; // number of running threads
    RThread iThread; //thread processing the crash
    RArray<TCrashEventInfo> iRemainingEvents; //copy of structures provided by target observer
    RArray<TCrashInfo> iRemainingFlashCrashEvents;
    RArray<TUint64> iTids;
    TCrashThreadParams iParams;
    CCoreDumpSession &iCoreSess;
	TUint iCrashCount;
	RMessagePtr2 iMessage;
};

#endif //CORE_CRASH_HANDLER_H
