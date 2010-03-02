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

#ifndef __TRKAPPVIEW_H
#define __TRKAPPVIEW_H

#include <viewcli.h>
#include <aknview.h>
#include "TrkConnData.h"
#include "trkservereventcallback.h"
#include "trksrvclisession.h"

//
//
// CTrkAppView
//
//
class CTrkSettingsData;
class CTrkAppContainer;
class CTrkServerEventListener;

class CTrkAppView : public CAknView, public MTrkServerEventCallback
{
public:

	static CTrkAppView* NewL(CTrkSettingsData* aSettings, RTrkSrvCliSession& aTrkSession);
	~CTrkAppView();
    void ConstructL();
    
    /** 
    * Handle screen size change.
    * @since 2.8
    */
    void HandleSizeChange();
    
    TBool IsConnected() { return iConnStatus == ETrkConnected; }
    TTrkConnStatus GetConnectionStatus(){ return iConnStatus; } 
    
    void GetConnectionMsg(TDes& aMessage);
    
public:
    // From MTrkServerEventCallback
    void DebugStateChanged(TBool& aDebugging);
    void ConnectionStateChanged(TTrkConnStatus& aConnStatus); 
    
private:

    /**
    * From CAknView
    */
    TUid Id() const;
    
    /**
    * From CAknView
    */
    void HandleCommandL(TInt aCommand);
    
    /**
    * From CAknView
    */
    void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
        const TDesC8& aCustomMessage);
    
    /**
    * From CAknView
    */
    void DoDeactivate();
   
    /**
     * From MCoeView
     */
    void ViewActivatedL(const TVwsViewId& aPrevViewId, TUid aCustomMessageId, const TDesC8& aCustomMessage);
   
    /**
    * Adjusts the visible items in menu pane accordding to
    * socket engine state.
    * @aResourceId Resource ID identifying the menu pane to initialise
    * @aMenuPane The in-memory representation of the menu pane
    */
    void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

	void HandleStatusPaneSizeChange();


private:
	CTrkAppView(CTrkSettingsData* aSettings, RTrkSrvCliSession& aTrkSession);

    TInt CloseCrashLogger();
	TInt TerminateProcess(const TDesC& aProcessName);
	
	TTrkConnType GetCmdLineConnTypeL();

private:

    CTrkSettingsData* iSettingsData;
    CTrkAppContainer* iContainer;
	//CVwsSessionWrapper* iVwsSession;
    RTrkSrvCliSession& iTrkSession;
    TVwsViewId iPrevViewId;
			
	TInt iMajorVersion;
	TInt iMinorVersion;
	TInt iMajorAPIVersion;
	TInt iMinorAPIVersion;
	TInt iBuildNumber;
    TTrkConnStatus iConnStatus;
	
	TBuf<KMaxPath> iConnMsg;
	
	TBool iDebugging;
	
	CTrkServerEventListener* iTrkDebugStateListener;
	CTrkServerEventListener* iTrkConnStateListener;
};

#endif //__TRKAPPVIEW_H

