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

#include <AknGlobalNote.h>

#include "TrkSettingsData.h"
#include "TrkSettingsList.h"
#include "TrkUids.h"
#include "TrkApp.hrh"

CTrkSettingsList::CTrkSettingsList(CTrkSettingsData* aSettingsData)	
:iSettingsData(aSettingsData)
{
}

TKeyResponse CTrkSettingsList::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
	if (aType != EEventKey) // Is not key event?
	{
		return EKeyWasNotConsumed;
	}

	switch (aKeyEvent.iCode) // The code of key event is...
	{
		default:
			CAknSettingItemList::OfferKeyEventL(aKeyEvent, aType);
			break;
	}

	return EKeyWasConsumed;
}

void CTrkSettingsList::SizeChanged()
{
	if (ListBox())
		ListBox()->SetRect(Rect());
}

void CTrkSettingsList::ChangeSelectedItemL()
{
	if (ListBox())
		EditItemL(ListBox()->CurrentItemIndex(), ETrue);
}




CTrkConnSettingsList::CTrkConnSettingsList(CTrkSettingsData* aSettingsData)
:CTrkSettingsList(aSettingsData)
{
}

CAknSettingItem* CTrkConnSettingsList::CreateSettingItemL(TInt aIdentifier)
{
	CAknSettingItem* item = NULL;
	
	switch (aIdentifier)
	{
		case ETrkConnectionType:
		{
			item = new (ELeave) CTrkConnIndexPopupSettingItem(aIdentifier, iSettingsData->iConnTypeIndex);
			break;
		}
		case ETrkConnectionPort:
		{
			item = new (ELeave) CAknIntegerEdwinSettingItem(aIdentifier, iSettingsData->iPortNumber);			
			break;
		}
		case ETrkConnectionBaudRate:
		{		
			item = new (ELeave) CTrkConnIndexPopupSettingItem(aIdentifier, iSettingsData->iBaudRateIndex);
			break;
		}
		case ETrkPlugAndPlayOption:
	    {
            item = new (ELeave) CTrkConnIndexPopupSettingItem(aIdentifier, iSettingsData->iPlugAndPlayType);
			break;
		}
	}
	
	return item;
}

void CTrkConnSettingsList::ControlStateChangedAfterL(TInt aIndex)
{
	CAknSettingItemArray* array = SettingItemArray();
	
	CTrkConnIndexPopupSettingItem* connTypePopUp = (CTrkConnIndexPopupSettingItem *)array->At(0);
	TInt index = connTypePopUp->CurrentIndex();
	
	CAknSettingItem* portEditor = array->At(1);
	CAknSettingItem* baudRatePopUp = array->At(2);

	switch(aIndex)
	{
		case 0:
		{
			switch (index)
			{
				// Now port and baud rate are hidden for 
				case 0: // USB (Dbg&Trc)
				case 1: // BT
				case 2: // XTI
				case 3: // DCC
				{
					portEditor->SetHidden(ETrue);
					baudRatePopUp->SetHidden(ETrue);
					HandleChangeInItemArrayOrVisibilityL();				
						break;			
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
}

void CTrkConnSettingsList::ControlStateChangedBeforeL(TInt aIndex)
{
	CAknSettingItemArray* array = SettingItemArray();
	
	CTrkConnIndexPopupSettingItem* connTypePopUp = (CTrkConnIndexPopupSettingItem *)array->At(0);
	TInt index = connTypePopUp->CurrentIndex();
	
	CAknSettingItem* portEditor = array->At(1);
	CAknSettingItem* baudRatePopUp = array->At(2);

	switch(aIndex)
	{
		case 1:
		{
			switch(index)
			{
				case 2:
				{
					_LIT(KPortNumberInfo, "Recommended port for IR connection is 0.");
					CAknGlobalNote* globalNote = CAknGlobalNote::NewLC();
					globalNote->ShowNoteL(EAknGlobalInformationNote, KPortNumberInfo);
					CleanupStack::PopAndDestroy(globalNote);
					break;	
				}
				default:
					break;
			}
		}
		default:
			break;
	}
}

void CTrkConnSettingsList::EditItemL(TInt aIndex, TBool aCalledFromMenu)
{
	CAknSettingItemList::EditItemL(aIndex, aCalledFromMenu);
	ControlStateChangedAfterL(aIndex);
}



CTrkConnIndexPopupSettingItem::CTrkConnIndexPopupSettingItem(TInt aResourceId, TInt& aValue)
	: CAknEnumeratedTextPopupSettingItem(aResourceId, aValue)
{
}
