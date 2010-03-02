/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
#include <coecntrl.h>
#include <eikenv.h>
#include <eikmenup.h>
#include <akntitle.h>
#include <AknUtils.h>
#include <aknmessagequerydialog.h>
#include <AknGlobalNote.h>

#include <avkon.rsg>
#include <trkapp_200170bb.rsg>

#include "trkservereventlistener.h"
#include "trkdebugstatelistener.h"
#include "trkconnstatelistener.h"
#include "TrkUids.h"
#include "TrkSettingsData.h"
#include "TrkAppContainer.h"
#include "TrkAppUi.h"
#include "TrkAppView.h"
#include "TrkApp.hrh"

#ifdef __OEM_TRK__
_LIT(KAboutText, "System TRK \nVersion %d.%d.%d \nTRK API Version %d.%d\nCopyright (c) 2006-2008, Nokia\n All rights reserved");
#else
_LIT(KAboutText, "Application TRK \nVersion %d.%d.%d \nTRK API Version %d.%d\nCopyright (c) 2006-2008, Nokia\n All rights reserved");
#endif

#define DISABLE_CRASH_LOGGER
#define KMaxCmdLineLength 20

// ---------------------------------------------------------
// CTrkAppView::CTrkAppView
// Constructor
// ---------------------------------------------------------
//
CTrkAppView::CTrkAppView(CTrkSettingsData* aSettingsData, RTrkSrvCliSession& aTrkSession)
	: //iVwsSession(NULL),
	  iSettingsData(aSettingsData),
	  iContainer(NULL),
	  iTrkSession(aTrkSession),
	  iConnStatus(ETrkNotConnected),
	  iConnMsg(KNullDesC),
	  iDebugging(EFalse),
	  iTrkDebugStateListener(NULL),
	  iTrkConnStateListener(NULL)
{
}

// ---------------------------------------------------------
// CTrkAppView::NewL
// 
// ---------------------------------------------------------
//
CTrkAppView* CTrkAppView::NewL(CTrkSettingsData* aSettingsData, RTrkSrvCliSession& aTrkSession)
{
	CTrkAppView* self = new(ELeave) CTrkAppView(aSettingsData, aTrkSession);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
}
// ---------------------------------------------------------
// CTrkAppView::~CTrkAppView
// Desctructor
// ---------------------------------------------------------
//
CTrkAppView::~CTrkAppView()
{
	//if (iVwsSession)
	//{
	//	iVwsSession->EnableServerEventTimeOut(ETrue);
	//	delete iVwsSession;
	//}
	
    if (iContainer)
    {
	    AppUi()->RemoveFromStack(iContainer);
	    delete iContainer;
	    iContainer = NULL;
    }

    if (iTrkDebugStateListener)
    {
        delete iTrkDebugStateListener;
        iTrkDebugStateListener = NULL;
    }
    if (iTrkConnStateListener)
    {
        delete iTrkConnStateListener;
        iTrkConnStateListener = NULL;
    }
}

// ---------------------------------------------------------
// CTrkAppView::ConstructL
// 
// ---------------------------------------------------------
//

void CTrkAppView::ConstructL()
{
    BaseConstructL(R_TRKAPP_VIEW);

	RProcess().SetPriority(EPriorityHigh);
	RThread().SetPriority(EPriorityAbsoluteHigh);

#ifdef DISABLE_CRASH_LOGGER	
	//Close the Mobile Crash Logger.
	//CloseCrashLogger();
#endif
	
    iTrkSession.GetTrkVersion(iMajorVersion, iMinorVersion, iMajorAPIVersion, iMinorAPIVersion, iBuildNumber);    
    iTrkSession.GetDebugConnStatus(iConnStatus, iConnMsg);
    iTrkSession.GetDebuggingStatus(iDebugging);
    
    iTrkDebugStateListener = new CTrkDebugStateListener(iTrkSession, this);
    iTrkDebugStateListener->ListenL();
    
    iTrkConnStateListener = new CTrkConnStateListener(iTrkSession, this);
    iTrkConnStateListener->ListenL();
	}
	
// ---------------------------------------------------------
// CTrkAppView::GetCmdLineConnTypeL
// Now this function wont be used
// ---------------------------------------------------------
//
TTrkConnType CTrkAppView::GetCmdLineConnTypeL()
{
    TTrkConnType connType = ETrkConnInvalid;
    
	// Get command line argument
    if (User::CommandLineLength() > 0)
    {
        TBuf<512> commandLineArguments;
		User::CommandLine( commandLineArguments );

		// Check command line argument
	    if (0 == commandLineArguments.Compare(KCmdLineConnTypeUsb))
	    {
	        connType = ETrkUsbDbgTrc;
	    }
	    else if (0 == commandLineArguments.Compare(KCmdLineConnTypeXti))
	    {
            connType =  ETrkXti;
        }
    }
    if (connType == ETrkConnInvalid)
    	User::Leave(KErrNotFound);
    
    return connType;  
}

// ---------------------------------------------------------
// TUid CTrkAppView::Id()
// Return id of this view instance
// ---------------------------------------------------------
//
TUid CTrkAppView::Id() const
{
    return KUidTrkAppView;
}

// ---------------------------------------------------------
// CTrkAppView::HandleCommandL(TInt aCommand)
// Command handler
// ---------------------------------------------------------
//
void CTrkAppView::HandleCommandL(TInt aCommand)
{
  	switch (aCommand)
	{
  	    case EAknSoftkeyBack:
  	    {
            AppUi()->ActivateViewL(iPrevViewId);
            break;
  	    }
		case ETrkConnect:
		{
            TInt err = iTrkSession.Connect();
            if(!err)
            {
                iTrkSession.GetDebugConnStatus(iConnStatus, iConnMsg);
			}
			
			// force redraw here
			iContainer->DrawNow();
			break;
		}
		case ETrkDisconnect:
		{
            iTrkSession.DisConnect();
            iConnStatus = ETrkNotConnected;
			iConnMsg = KNullDesC;
			// force redraw here
			iContainer->DrawNow();
			break;
		}		
		case ETrkSettings:
		{
			AppUi()->ActivateLocalViewL(KUidTrkConnSettingsView);
			break;
		}
		case ETrkAbout:
		{
    		// Show the dialog.
    		TBuf<KMaxPath> versionInfo;
    		versionInfo.Format(KAboutText, iMajorVersion, iMinorVersion, iBuildNumber, iMajorAPIVersion, iMinorAPIVersion);
    		
		    CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL(versionInfo);
		    dlg->PrepareLC(R_TRK_ABOUT_DIALOG);
		    CAknPopupHeadingPane* hPane = dlg->QueryHeading();

		    if (hPane)
			{
				hPane->SetTextL(_L("TRK for Symbian OS"));
			}
		    dlg->RunLD();

    		break;
		}
		default:
		{
			AppUi()->HandleCommandL(aCommand);
			break;
		}
	}
}


// ---------------------------------------------------------
// CTrkAppView::DoActivateL(...)
// Activation of this view instance.
// ---------------------------------------------------------
//
void CTrkAppView::DoActivateL(const TVwsViewId& /*aPrevViewId*/, TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
{
    if (!iContainer)
	{
        iContainer = new (ELeave) CTrkAppContainer(this);
        iContainer->SetMopParent(this);
        iContainer->ConstructL(ClientRect());
	    AppUi()->AddToStackL(*this, iContainer);
			    
	    iContainer->MakeVisible(ETrue);
	    iContainer->ActivateL();
	} 
}

// ---------------------------------------------------------
// CConnTestView::DoDeactivate()
// Deactivation of this view instance. Controls are not 
// deleted, so that old information in status and output
// windows stays visible.
// ---------------------------------------------------------
//
void CTrkAppView::DoDeactivate()
{
    if (iContainer)
    {
        AppUi()->RemoveFromStack(iContainer);
	   	iContainer->MakeVisible(EFalse);
	    delete iContainer;
	    iContainer = NULL;    
    }
 }

void CTrkAppView::ViewActivatedL(const TVwsViewId& aPrevViewId, TUid aCustomMessageId, const TDesC8& aCustomMessage)
{
    // save the previous view id
    if ((KUidTrk != aPrevViewId.iAppUid) && (KUidTrkConnSettingsView != aPrevViewId.iViewUid))
    {
        iPrevViewId = aPrevViewId;
    }
    CAknView::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
}

void CTrkAppView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
{
	if (R_TRK_MAIN_MENU == aResourceId)
	{
		switch (iConnStatus)
	    {
		    case ETrkConnected:
			{
				aMenuPane->SetItemDimmed(ETrkConnect, ETrue);
				//If there is an active debug session, don't allow the user to disconnect.
		        aMenuPane->SetItemDimmed(ETrkDisconnect, iDebugging);
		        aMenuPane->SetItemDimmed(EAknSoftkeyExit, EFalse);
		        break;
	        }
		    case ETrkNotConnected:
		    case ETrkConnectionError:
			{
				aMenuPane->SetItemDimmed(ETrkConnect, EFalse);
		        aMenuPane->SetItemDimmed(ETrkDisconnect, ETrue);            
		        aMenuPane->SetItemDimmed(EAknSoftkeyExit, EFalse);
		        break;
	        }
		    case ETrkConnecting:
	        {
		        aMenuPane->SetItemDimmed(ETrkConnect, ETrue);
				aMenuPane->SetItemDimmed(ETrkDisconnect, ETrue);			
		        aMenuPane->SetItemDimmed(EAknSoftkeyExit, ETrue);
		        break;
	        }		    	        	        
		}
	}
}

// -----------------------------------------------------------------------------
// CTrkAppView::HandleSizeChange
// Handle screen size change.
// -----------------------------------------------------------------------------
//
void CTrkAppView::HandleSizeChange()
{
    if (iContainer)
    {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        iContainer->SetRect(mainPaneRect);
    }
}

/** 
 * Handle status pane size change for this view (override)
 */
void CTrkAppView::HandleStatusPaneSizeChange()
{
	CAknView::HandleStatusPaneSizeChange();
}


// -----------------------------------------------------------------------------
// CTrkAppView::GetConnectionMsg
// -----------------------------------------------------------------------------
//
void CTrkAppView::GetConnectionMsg(TDes& aMessage)
{
	aMessage = iConnMsg;
}

void CTrkAppView::DebugStateChanged(TBool& aDebugging)
{
    iDebugging = aDebugging;
}

void CTrkAppView::ConnectionStateChanged(TTrkConnStatus& aConnStatus) 
{
    iConnStatus = aConnStatus;
    
    switch(iConnStatus)
    {
        case ETrkConnected:
        case ETrkConnecting:
        case ETrkConnectionError:
        case ETrkNotConnected:
             iTrkSession.GetDebugConnStatus(iConnStatus, iConnMsg);
             break;
                    
        default: 
            iConnMsg = KNullDesC;
            break;
	}

    // force redraw here
    iContainer->DrawDeferred();
}

// -----------------------------------------------------------------------------
// CTrkAppView::CloseCrashLogger
// -----------------------------------------------------------------------------
//
TInt CTrkAppView::CloseCrashLogger()
{
	TInt err = KErrNone;
	
	//The old mobile crash file name is "d_exc_mc.exe" and the new one is "mc_useragent.exe"	
	//This is the string that needs to be passed to the RProcess::Open call to get a handle.
	//The complete process name actually includes the UID info as well.
	//Instead of hard coding the process name, its better to just 
	//search for the process and find it that way.		
	//_LIT16(KCrashLoggerName, "mc_useragent.exe[1020e519]0001");
	_LIT16(KOldCrashLoggerName, "d_exc_mc*");
	_LIT16(KCrashLoggerName, "mc_useragent*");
	
	err = TerminateProcess(KOldCrashLoggerName);
	err = TerminateProcess(KCrashLoggerName);

	return err;
}

// -----------------------------------------------------------------------------
// CTrkAppView::TerminateProcess()
// -----------------------------------------------------------------------------
//
TInt CTrkAppView::TerminateProcess(const TDesC& aProcessName)
{
	_LIT16(KCrashLoggerClMsg, "Closing crash logger");
	
	TFindProcess find(aProcessName);
	TFullName name;	
		
	TInt err = find.Next(name);
	if (KErrNone == err)
	{	
		RProcess process;
		err = process.Open(find);
	
		if (KErrNone == err)
		{
			User::InfoPrint(KCrashLoggerClMsg);
			process.Kill(KErrNone);
		}
	}
	return err;
}

