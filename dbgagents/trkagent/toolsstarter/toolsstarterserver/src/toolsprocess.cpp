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
#include <e32std.h>                      

#include "toolsprocess.h" 

#define SafeDelete(x) { if (x) delete x; x = NULL; }
                   
// LOCAL FUNCTION PROTOTYPES

// LOCAL CONSTANTS


//
// CProcessTracker constructor
//
// Accepts CTrkDispatchLayer interface and the process ID of the
// process to watch
//
CProcessTracker::CProcessTracker(CToolsProcess *aToolsProcess, TUint32 aProcessId)
    : CActive(EPriorityStandard),
      iProcessId(aProcessId),
      iToolsProcess(aToolsProcess)
{
    CActiveScheduler::Add(this);

    TInt error = iProcess.Open(iProcessId);
    if (KErrNone != error)
        User::Panic(_L("Unable to open process"), __LINE__);
}

//
// CProcessTracker destructor
//
CProcessTracker::~CProcessTracker()
{
    Cancel();
    Deque();
    iProcess.Close();
}

//
// CProcessTracker::Watch
//
// Watch for the process to exit
//
void CProcessTracker::Watch()
{
    iProcess.Logon(iStatus);
    SetActive();
}

//
// CProcessTracker::RunL
//
// Called when the process exits
//
void CProcessTracker::RunL()
{
    // sometimes this is run when the process has yet to exit
    if (iProcess.ExitType() == EExitPending)
    {
        iProcess.LogonCancel(iStatus);
        Watch();
    }
    else
    {
        iToolsProcess->ProcessDied(iProcess.ExitReason());
    }
}

//
// CProcessTracker::DoCancel
//
// Stop waiting for this process to exit
//
void CProcessTracker::DoCancel()
{
    iProcess.LogonCancel(iStatus);
}

//
// CToolsProcess implementation
//

TBool CToolsProcess::iFirstTimeAfterBoot = ETrue;
//
// CToolsProcess::NewL
//
CToolsProcess* CToolsProcess::NewL(const TDesC& aPath, const TDesC& aArgsUsb, const TDesC& aArgsXti)
{
    CToolsProcess* self = new(ELeave) CToolsProcess;
    self->ConstructL(aPath, aArgsUsb, aArgsXti);
    return self;
}

//
// CTrkEngine::NewLC
//
CToolsProcess* CToolsProcess::NewLC(const TDesC& aPath, const TDesC& aArgsUsb, const TDesC& aArgsXti)
{
    CToolsProcess* self = new(ELeave) CToolsProcess;
    CleanupStack::PushL(self);
    
    self->ConstructL(aPath, aArgsUsb, aArgsXti);
    
    return self;
}

//
// CToolsProcess::ConstructL
// Constructor
//
void CToolsProcess::ConstructL(const TDesC& aPath, const TDesC& aArgsUsb, const TDesC& aArgsXti) 
{
    iPath.Copy(aPath);
    iArgsUsb.Copy(aArgsUsb);
    iArgsXti.Copy(aArgsXti);
}
//
// CToolsProcess::CToolsProcess
// Constructor
//
CToolsProcess::CToolsProcess() 
            : iRunning(EFalse),
              iProcessTracker(NULL)
{
        
}

//
// CToolsProcess::~CToolsProcess
// Destructor
//
CToolsProcess::~CToolsProcess()
{
    SafeDelete(iProcessTracker);
    iProcess.Close();
}

//
// CToolsProcess::Start
//
TInt CToolsProcess::Start(TCmdLineConnType aConnType) 
{
    TBuf<KMaxCmdLineConnArgsLen> cmdConnArgs = _L("");
    switch (aConnType)
    {
        case EUsb:
            cmdConnArgs = iArgsUsb;
            break;
        case EXti:
            cmdConnArgs = iArgsXti;
            break;
        default:
            return KErrNotFound;
    }
  
    TInt err = KErrNone;
    // check to see if we have already launched or not.
    // We only launch the first time and we track the process that we launched
    // Also check to see if the process is already running or not.
    // User could have manually launched the application in which case the user needs to 
    // connect from the tool manually.
    if (!iRunning)
    {       
        err = iProcess.Create(iPath, cmdConnArgs);  
        if (!err)
        {
            iProcessId = iProcess.Id();     
            iProcess.Resume();        
          
        }
        if (!err)
        {
            iRunning = ETrue; //now the process is running..
        }
        
        
    }
    
    return err;
}

//
// CToolsProcess::Stop
//
void CToolsProcess::Stop() 
{
    if (iRunning)
    {
        iRunning = EFalse;
        iProcess.Kill(-1);
        iProcess.Close();
        SafeDelete(iProcessTracker);
    }
}

//
// CToolsProcess::Stop
//
void CToolsProcess::ProcessDied(TInt aExitReason) 
{
    iRunning = EFalse;
    iProcess.Close();
    SafeDelete(iProcessTracker);        
}

