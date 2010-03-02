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

#include <avkon.hrh>
#include <eikmenub.h>
#include <akntitle.h>
#include <barsread.h>
#include <avkon.rsg>
#include <trkapp_200170bb.rsg>

#include "TrkApp.hrh"
#include "TrkSettingsData.h"
#include "TrkUids.h"
#include "TrkSettingsView.h"

#include "TrkAppUi.h"

_LIT(KErrorNotification,"Error in connecting to TRK server . Please restart the phone and try.");
//
// CTrkAppUi::ConstructL
//
void CTrkAppUi::ConstructL()
{
    BaseConstructL(EAknEnableSkin);
	
    TInt err = iTrkSession.ConnectToServer();
    if(err)
    {
        CAknGlobalNote* errNote = CAknGlobalNote::NewL();
        CleanupStack::PushL( errNote );
        TInt noteid = errNote->ShowNoteL(EAknGlobalErrorNote, KErrorNotification);
        CleanupStack::PopAndDestroy();  
        User::Leave(err);
    }
   	iSettingsData = CTrkSettingsData::NewL(iTrkSession); 

	iAppView = CTrkAppView::NewL(iSettingsData, iTrkSession);
	AddViewL(iAppView);

	iSettingsView = CTrkSettingsView::NewL(iSettingsData);
	AddViewL(iSettingsView);

	//set the main app view as the default view
	SetDefaultViewL(*iAppView);
}

//
// CTrkAppUi Desctructor
//
CTrkAppUi::~CTrkAppUi()
{	
	if (iSettingsData)
	{
		delete iSettingsData;
		iSettingsData = NULL;
	}

	iSettingsView = NULL;
	iAppView = NULL;
}

//
// CTrkAppUi::HandleCommandL
//
void CTrkAppUi::HandleCommandL(TInt aCommand)
{
	switch (aCommand)
	{
		case EEikCmdExit:
		case EAknSoftkeyExit:
		{
			//if (!iEngine->IsDebugging())
			{
            	Exit();
	        }
			break;
		}
	}
}

//
// CTrkAppUi::HandleResourceChangeL
//
void CTrkAppUi::HandleResourceChangeL( TInt aType )
{
	CAknViewAppUi::HandleResourceChangeL(aType);
	
	if (aType == KEikDynamicLayoutVariantSwitch ||
		aType == KEikMessageUnfadeWindows ||
		aType == KEikMessageWindowsFadeChange ) 
	{
		iAppView->HandleSizeChange();
		iSettingsView->HandleSizeChange();
		
	  	CEikStatusPane* pane = StatusPane();
    	if (pane)
        {
        	pane->HandleResourceChange(aType);
        }
	}
}

// ----------------------------------------------------
// CCcorAppUi::SetTitleL
// Set title pane text from a resource.
// ----------------------------------------------------
//
void CTrkAppUi::SetTitleL(TInt aResourceId)
{
    // Set title
    CAknTitlePane* title = static_cast<CAknTitlePane*>(StatusPane()->ControlL(TUid::Uid(EEikStatusPaneUidTitle)));
    TResourceReader reader;
    iCoeEnv->CreateResourceReaderLC(reader, aResourceId);
    title->SetFromResourceL(reader);
    CleanupStack::PopAndDestroy();  // resource reader
}


// ----------------------------------------------------
// CTrkAppUi::SetTitleL
// Set title pane text from a descriptor.
// ----------------------------------------------------
//
void CTrkAppUi::SetTitleL(const TDesC& aText)
{
    CAknTitlePane* title = static_cast<CAknTitlePane*>(StatusPane()->ControlL(TUid::Uid(EEikStatusPaneUidTitle)));
    title->SetTextL(aText);
}
