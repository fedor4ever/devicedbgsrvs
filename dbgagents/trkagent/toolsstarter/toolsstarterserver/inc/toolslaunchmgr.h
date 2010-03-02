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


#ifndef     _TOOLSLAUNHCMGR_H_
#define     _TOOLSLAUNHCMGR_H_

//  INCLUDES
#include <e32base.h>
#include <e32std.h>
#include "launchmanagerinterface.h"
#include "toolsprocess.h"
#include "toolsconnectionlistener.h"
#include "toolsusbportlistener.h"
#include "sessionmessgelistener.h"


// DATA TYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION


class CToolsLaunchMgr: public CBase, public ILaunchManagerInterface, public MToolsConnectionListener
{
    public:  
        static CToolsLaunchMgr* NewL();
        static CToolsLaunchMgr* NewLC();
        ~CToolsLaunchMgr();

	private:
	    
		CToolsLaunchMgr();
		void ConstructL();
		void LaunchTools(TCmdLineConnType aConnType);
		void StopTools();
				        
	public:        
	    // from MConnectionListener
	    void ConnectionAvailable();
	    void ConnectionUnAvailable();

         //From ILaunchManagerInterface 
	    void RegisterListener(MConnStateListener* aListener);           
	    void UnregisterListener(MConnStateListener* aListener);
	    TConnectionStatus GetUsbConnStatus();
	    void Launch();
	    
	public:
	    
	    TInt CloseCrashLogger();    
	    TInt TerminateProcess(const TDesC& aProcessName);
        
    private:
    	CToolsUsbPortListener* iUsbPortListener;    	
    	RPointerArray<CToolsProcess> iToolsProcessList;
    	RPointerArray<MConnStateListener> iToolListenerList;
    
};



#endif      //  _TOOLSLAUNHCMGR_H_


//  End of File
