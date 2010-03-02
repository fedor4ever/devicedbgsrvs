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
#include <AknGlobalNote.h>
#include <trkapp_200170bb.rsg>

#include "TrkAppUi.h"
#include "TrkSettingsData.h"
#include "TrkSettingsList.h"
#include "TrkSettingsContainer.h"


// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------------------------
// CTrkSettingsContainer::CTrkSettingsContainer
// C++ constructor
// ---------------------------------------------------------------------------
//
CTrkSettingsContainer::CTrkSettingsContainer(CTrkSettingsData* aSettingsData)
: iSettingsData(aSettingsData)
{
}


// ---------------------------------------------------------------------------
// CTrkSettingsContainer::ConstructL
// Symbian OS 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CTrkSettingsContainer::ConstructL(const TRect& aRect)
{
	SetRect(aRect);

    iSettingsList = new ( ELeave ) CTrkConnSettingsList(iSettingsData);
    iSettingsList->SetContainerWindowL(*this);
    iSettingsList->ConstructFromResourceL(R_TRK_CONNECTION_SETTINGS_LIST);
    iSettingsList->SetRect(aRect);
    iSettingsList->ControlStateChangedAfterL(0);
}


// Destructor
CTrkSettingsContainer::~CTrkSettingsContainer()
{
    delete iSettingsList;
}


// ---------------------------------------------------------------------------
// CTrkSettingsContainer::CountComponentControls
// From CCoeControl return the number of controls owned
// ---------------------------------------------------------------------------
//
TInt CTrkSettingsContainer::CountComponentControls() const
{
    return 1;
}


// ---------------------------------------------------------------------------
// CTrkSettingsContainer::ComponentControl
// From CCoeControl returns a control
// ---------------------------------------------------------------------------
//
CCoeControl* CTrkSettingsContainer::ComponentControl(TInt /* aIndex */ ) const
{
    return iSettingsList;
}


// ---------------------------------------------------------------------------
// CTrkSettingsContainer::EditCurrentL
// Open setting page for currently selected setting item.
// ---------------------------------------------------------------------------
//
void CTrkSettingsContainer::EditCurrentL()
{
    TInt index = iSettingsList->ListBox()->CurrentItemIndex();

    iSettingsList->EditItemL(index, ETrue);
}


// ---------------------------------------------------------------------------
// CTrkSettingsContainer::SaveSettingsL
// Save all settings.
// ---------------------------------------------------------------------------
//
void CTrkSettingsContainer::SaveSettingsL()
{
    TRAPD(err, iSettingsList->StoreSettingsL());
    if (!err)
    {
    	err = iSettingsData->SaveSettings();
    }
    
    if (err!= KErrNone)
    {
		_LIT(KUnableToSave, "Unable to save the specified settings");
		CAknGlobalNote* globalNote = CAknGlobalNote::NewLC();
		globalNote->ShowNoteL(EAknGlobalErrorNote, KUnableToSave);
		CleanupStack::PopAndDestroy(globalNote);
    }
}


// ---------------------------------------------------------------------------
// CTrkSettingsContainer::OfferKeyEventL
// Key event handling
// ---------------------------------------------------------------------------
//
TKeyResponse CTrkSettingsContainer::OfferKeyEventL(
    const TKeyEvent& aKeyEvent,
    TEventCode aType )
{
    if ( aKeyEvent.iCode == EKeyLeftArrow ||
         aKeyEvent.iCode == EKeyRightArrow )
	{
        // Listbox takes all events even if it doesn't use them
        return EKeyWasNotConsumed; 
	}

    return iSettingsList->OfferKeyEventL( aKeyEvent, aType );
}
    
// ---------------------------------------------------------------------------
// CTrkSettingsContainer::SizeChanged
// Set the size and position of component controls.
// ---------------------------------------------------------------------------
//
void CTrkSettingsContainer::SizeChanged()
{
    if (iSettingsList)
	{
        iSettingsList->SetRect(Rect());

	}
}
// End of File
