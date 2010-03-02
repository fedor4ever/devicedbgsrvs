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
#include <apmrec.h>
#include <ecom/implementationproxy.h>
#include <e32debug.h>
#include <startupdomainpskeys.h>

#include "toolsstarterautolaunch.h"
#include "logging.h"

#define KPSDelay 500000


// CONSTANTS
const static TUint KToolsStarterAutoLaunchImplUid = 0x200170B9;
const static TUid KToolsStarterAutoLaunchUid = {0x200170B8};
_LIT(KToolsStarterServerExe, "toolsstarterserver.exe");
const TUid KToolsStarterServerExeUid = {0x200170B5};
_LIT( KToolsStarterMatchPattern, "toolsstarterserver**" );

const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( KToolsStarterAutoLaunchImplUid,
    		CToolsStarterAutoLaunch::CreateRecognizerL )
    };


// ============================ MEMBER FUNCTIONS ===============================

EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }


CApaDataRecognizerType* CToolsStarterAutoLaunch::CreateRecognizerL()
    {
	LOG_MSG("CToolsStarterAutoLaunch::CreateRecognizerL");

	CApaDataRecognizerType* recognizer = new (ELeave) CToolsStarterAutoLaunch();
	CToolsStarterAutoLaunch::CreateAutoStartThreadL();
    return recognizer;
    }


CToolsStarterAutoLaunch::CToolsStarterAutoLaunch() :
    CApaDataRecognizerType( KToolsStarterAutoLaunchUid, CApaDataRecognizerType::ENormal )
    {
    iCountDataTypes = 1;
    }



TUint CToolsStarterAutoLaunch::PreferredBufSize()
    {
    return 0;
    }


TDataType CToolsStarterAutoLaunch::SupportedDataTypeL( TInt /*aIndex*/ ) const
    {
    return TDataType();
    }


void CToolsStarterAutoLaunch::DoRecognizeL( const TDesC& /*aName*/, const TDesC8& /*aBuffer*/ )
    {
    }


void CToolsStarterAutoLaunch::CreateAutoStartThreadL()
    {
    LOG_MSG("CToolsStarterAutoLaunch::CreateAutoStartThread");
    
    //create a new thread for starting our application
    RThread startAppThread;

    User::LeaveIfError( startAppThread.Create(
        _L( "ToolsStarterAutoLaunch" ),
        CToolsStarterAutoLaunch::StartAppThreadFunction,
        KDefaultStackSize,
        KMinHeapSize,
        KMinHeapSize,
        NULL,
        EOwnerThread ) );

    startAppThread.SetPriority( EPriorityNormal );
    startAppThread.Resume();
    startAppThread.Close();
    }


TInt CToolsStarterAutoLaunch::StartAppThreadFunction( TAny* /*aParam*/ )
    {
    LOG_MSG("CToolsStarterAutoLaunch::StartAppThreadFunction");
    
     // create a trap cleanup and active scheduler
    CTrapCleanup* pC = CTrapCleanup::New();
    
    //CActiveScheduler* pS = new CActiveScheduler;
    //CActiveScheduler::Install(pS);
    
    if (pC==NULL)// || pS==NULL)
        LOG_MSG("Failed to create the trapcleanup and active scheduler");
        
            // create a server starter instance
    CToolsStarterLauncher* serverStarter = NULL;
    TRAPD(err, serverStarter = CToolsStarterLauncher::NewL());
    
    if (err == KErrNone)
    	{
	    // wait for UI to be up
	   // serverStarter->WaitForUiServices();
		// now start the toolsstarter server
		serverStarter->StartServer();
		delete serverStarter;	    
	   	}    

  	delete pC;
  	//delete pS;
  	
    return KErrNone;
    }


CToolsStarterLauncher* CToolsStarterLauncher::NewL()
    {
    CToolsStarterLauncher* self = new(ELeave) CToolsStarterLauncher();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }



CToolsStarterLauncher::CToolsStarterLauncher()
    {
    }


    
CToolsStarterLauncher::~CToolsStarterLauncher()
    {

    }


 
void CToolsStarterLauncher::ConstructL()
    {
    }

void CToolsStarterLauncher::StartServer()
    {
    LOG_MSG("CToolsStarterLauncher::StartServer");
    
    // check if toolsstarter is already running
    TFullName processName;
    TFindProcess finder( KToolsStarterMatchPattern );
    TInt err = finder.Next( processName );
    if ( err == KErrNone )
        {
        LOG_MSG("KToolsStarterMatchPattern server already running");
        return;
        }

    // otherwise create a new process
    RProcess pr;
    err = pr.Create(KToolsStarterServerExe, TPtr(NULL, 0), TUidType(KNullUid, KNullUid, KToolsStarterServerExeUid));
    if ( err == KErrNone )
        {
        LOG_MSG("CToolsStarterLauncher::StartServer process created");
        pr.Resume();
        pr.Close();
        }
    else
        {
        LOG_MSG2("CToolsStarterLauncher::StartServer Could not create process %d", err);
        }
    }


void CToolsStarterLauncher::WaitForUiServices()
    {
    LOG_MSG("ToolsStarter Launcher: WaitForUiServices");
    RProperty property;
    TInt err = KErrNotReady;
    while ( err != KErrNone )
        {
        User::After(KPSDelay); // Wait for the startup to define the key
        err = property.Attach(KPSUidStartup, KPSGlobalSystemState);
        LOG_MSG2("ToolsStarter Launcher: WaitForUiServices attach err %d \n", err);
        }
    // Wait for the correct startup state
    TRequestStatus status;
    property.Subscribe(status);
    TInt state = 0;
    err = property.Get(state);
    LOG_MSG3("ToolsStarter Launcher: WaitForUiServices state %d err %d \n", state, err);
    while ( err == KErrNone &&
            !( state == ESwStateCriticalPhaseOK ||
               state == ESwStateNormalRfOn ||
               state == ESwStateNormalRfOff ||
               state == ESwStateNormalBTSap ) )
        {
        state = 0; // Reset previous value
        User::WaitForRequest(status);
        err = status.Int();
        if ( err == KErrNone )
            {
            property.Subscribe( status );
            err = property.Get( state );
            }
        LOG_MSG3("ToolsStarter Launcher: WaitForUiServices state changed %d err %d", state, err);
        }
    property.Cancel();
    property.Close();
    LOG_MSG("ToolsStarter Launcher: WaitForUiServices end");
    }


