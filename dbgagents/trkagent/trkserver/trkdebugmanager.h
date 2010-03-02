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

#ifndef __TrkDebugManager_h__
#define __TrkDebugManager_h__

// System includes
#include <e32base.h>
#include <f32file.h>

// User includes
#include "TrkEngine.h"
#include "trksrvcmdcodes.h"
#ifndef __TEXT_SHELL__
#include "toolsconnectionlistener.h"
#endif
// Classes referenced
class MTrkDebugMgrCallbacks;

//
// CTrkDebugManager (header)
//
// The underlying engine used by each session. Manipulates the server-side objects.
//
class CTrkDebugManager : public CBase, public MTrkEngineCallback , public MTrkUsbConnectionListener
{
	//Static construct and destruct
	public:
		static 	CTrkDebugManager*	NewLC();
		static 	CTrkDebugManager*	NewL();
				~CTrkDebugManager();
	

	//methods implementing the server cmds functionality
	public:	
		void 	StartDebuggingL();
		void    StartDebuggingL(TTrkConnType aConnType);
		void 	StopDebugging();
		void	GetVersion(TTRKVersion& aVersion);
		void    GetConnectionMsg(TDes& aConnStatusMsg);
		void	ConnectToDebuggerL();
		void	DisConnect();
		void    GetCurrentConnData(TTrkConnData& aConnData);
		void    SetCurrentConnData(TTrkConnData& aConnData);
	    TBool   IsConnected() { return (iConnStatus == ETrkConnected); }
		TTrkConnStatus GetConnectionStatus(){return iConnStatus;};
	    TBool   IsDebugging();
	    TBool   GetPlugAndPlayOption();
	    void    SetPlugAndPlayOption(TBool aPlugAndPlay);
	    
	    void    SetDebugMgrCallback(MTrkDebugMgrCallbacks* aDebugMgrCallback) { iDebugMgrCallback = aDebugMgrCallback; }
	    void    NotifyTheUserL();
	    void    StopListening();
	
	public: // methods from MTrkEngineCallback
	    void    OnConnection(); 
	    void    OnCloseConnection();
	    void    DebuggingStarted();
	    void    DebuggingEnded();
	    void    OnAsyncConnectionFailed();
	    
	public: //Methods from  public MTrkUsbConnectionListener
	    void   ConnectionAvailable();
	    void   ConnectionUnAvailable();
	    	    
    //private construct
    private:
                CTrkDebugManager();
        void    ConstructL();

	//date members
	private:
		CTrkEngine* iTrkEngine;
		TTrkConnStatus iConnStatus;
		TBool iDebugging;
		TBuf<KMaxPath> iConnMsg;
		TBool iPlugAndPlay;
        MTrkDebugMgrCallbacks* iDebugMgrCallback;
#ifndef __TEXT_SHELL__
        CToolsConnectionListener* iToolsConnectionListener;
#endif

};

#endif // __TrkDebugManager_h__
