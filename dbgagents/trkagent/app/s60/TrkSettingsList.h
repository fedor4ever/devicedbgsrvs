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

#ifndef __TrkSettingsList_h__
#define __TrkSettingsList_h__


#include <aknsettingitemlist.h>

class CTrkSettingsList : public CAknSettingItemList
{
public:
	CTrkSettingsList(CTrkSettingsData* aSettingsData);

	virtual CAknSettingItem* CreateSettingItemL(TInt aIdentifier) = 0;
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	
	void ChangeSelectedItemL(); 

private:
	void SizeChanged();
	
protected:
	CTrkSettingsData* iSettingsData;

};

class CTrkConnSettingsList : public CTrkSettingsList
{
public:	
	CTrkConnSettingsList(CTrkSettingsData* aSettingsData);
	CAknSettingItem* CreateSettingItemL(TInt aIdentifier);
	virtual void EditItemL(TInt aIndex, TBool aCalledFromMenu);
	void ControlStateChangedAfterL(TInt aIndex);
	void ControlStateChangedBeforeL(TInt aIndex);

private:

};

class CTrkConnIndexPopupSettingItem : public CAknEnumeratedTextPopupSettingItem
{
public:
	CTrkConnIndexPopupSettingItem(TInt aResourceId, TInt& aValue);
	TInt CurrentIndex() { return InternalValue();};
};


#endif //__TrkSettingsList_h__
