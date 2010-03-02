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
#include <AknUtils.h>
#include "TrkAppUi.h"

#include <trkapp_200170bb.rsg>

#include "TrkApp.hrh"
#include "TrkUids.h"
#include "TrkSettingsContainer.h"
#include "TrkSettingsData.h"
#include "TrkSettingsView.h"


// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------------------------
// CTrkSettingsView::CTrkSettingsView
// C++ constructor
// ---------------------------------------------------------------------------
//
CTrkSettingsView::CTrkSettingsView(CTrkSettingsData* aSettingsData)
: iSettingsData(aSettingsData)
{
}


// ---------------------------------------------------------------------------
// CTrkSettingsView.::NewLC
// Symbian OS two-phased constructor.
// ---------------------------------------------------------------------------
//
CTrkSettingsView* CTrkSettingsView::NewL(CTrkSettingsData* aSettingsData)
{
    CTrkSettingsView* self = new (ELeave) CTrkSettingsView(aSettingsData);
    self->ConstructL();

    return self;
}


// ---------------------------------------------------------------------------
// CTrkSettingsView::ConstructL
// Symbian OS 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CTrkSettingsView::ConstructL()
{
    BaseConstructL(R_TRKSETTINGS_VIEW);
}


// Destructor
CTrkSettingsView::~CTrkSettingsView()
{
    if (iContainer)
	{
        AppUi()->RemoveFromStack(iContainer);
        delete iContainer;
        iContainer = NULL;
	}
}


// ---------------------------------------------------------------------------
// CTrkSettingsView::Id
// From CAknView, returns Uid of View
// ---------------------------------------------------------------------------
//
TUid CTrkSettingsView::Id() const
{
    return KUidTrkConnSettingsView;
}


// ---------------------------------------------------------------------------
// CTrkSettingsView::HandleCommandL
// From MEikMenuObserver delegate commands from the menu
// ---------------------------------------------------------------------------
//
void CTrkSettingsView::HandleCommandL(TInt aCommand)
{
    switch (aCommand)
	{
        case ETrkCmdChange:
		{
            iContainer->EditCurrentL();
            break;
		}
        case EAknSoftkeyBack:
		{
			if (iContainer)
			{
				iContainer->SaveSettingsL();
			}
            AppUi()->ActivateLocalViewL(KUidTrkAppView);
            break;
		}

        default:
		{
            AppUi()->HandleCommandL( aCommand );
            break;
		}
 	}
}


// ---------------------------------------------------------------------------
// CTrkSettingsView::DoActivateL
// Activate this view
// ---------------------------------------------------------------------------
//
void CTrkSettingsView::DoActivateL( const TVwsViewId& /* aPrevViewId */,
                                          TUid /*aCustomMessageId*/,
                                          const TDesC8& /*aCustomMessage*/ )
{

    if (!iContainer)
	{
        iContainer = new (ELeave) CTrkSettingsContainer(iSettingsData);
        iContainer->SetMopParent( this );
        iContainer->ConstructL(ClientRect());
        AppUi()->AddToStackL( *this, iContainer );
        iContainer->ActivateL();
	}
	
}


// ---------------------------------------------------------------------------
// CTrkSettingsView::DoDeactivate
// Deactivate this view
// ---------------------------------------------------------------------------
//
void CTrkSettingsView::DoDeactivate()
{
    if (iContainer)
	{   
        AppUi()->RemoveFromStack(iContainer);
        delete iContainer;
        iContainer = NULL;
	}
}


// ---------------------------------------------------------------------------
// CTrkSettingsView::DynInitMenuPaneL
// Dynamically customize menu items
// ---------------------------------------------------------------------------
//
void CTrkSettingsView::DynInitMenuPaneL( TInt /*aResourceId*/, CEikMenuPane* /*aMenuPane*/)
{
}



// -----------------------------------------------------------------------------
// CTrkSettingsView::HandleSizeChange
// Handle screen size change.
// -----------------------------------------------------------------------------
//
void CTrkSettingsView::HandleSizeChange()
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
void CTrkSettingsView::HandleStatusPaneSizeChange()
{
	CAknView::HandleStatusPaneSizeChange();
}


// End of File
