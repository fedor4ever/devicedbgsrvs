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

#ifndef TRKSETTINGSCONTAINER_H
#define TRKSETTINGSCONTAINER_H

// INCLUDES
#include <coecntrl.h>

class CTrkSettingsData;
class CTrkConnSettingsList;


class CTrkSettingsContainer : public CCoeControl
{
    public: // Constructors and destructor
        /**
        * C++ constructor.
        */
        CTrkSettingsContainer(CTrkSettingsData* aSettingsData);

        /**
        * Symbian OS default constructor.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Destructor.
        */
        virtual ~CTrkSettingsContainer();

    public: // Functions from base classes
        /**
        * From CCoeControl set the size and position of its component controls.
        */
        void SizeChanged();
    
    public: // New functions
        /**
        * Open setting page for currently selected setting item.
        */
        void EditCurrentL();

        /**
        * Save all settings.
        */
        void SaveSettingsL();

    protected: // Functions from base classes

        /**
        * From CCoeControl return the number of controls owned
        * @return TInt number of controls
        */
        TInt CountComponentControls() const;

        /**
        * From CCoeControl returns a control
        * @param aIndex index of a control
        * @return CCoeControl* pointer on a control
        */
        CCoeControl* ComponentControl(TInt aIndex) const;

        /**
        * From CCoeControl event handling section
        * @param aKeyEvent the key event
        * @param aType the type of the event
        * @return TKeyResponse key event was used by this control or not
        */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                     TEventCode aType );


    private: //data
        CTrkConnSettingsList* iSettingsList;
        CTrkSettingsData* iSettingsData;
};

#endif  //TRKSETTINGSCONTAINER_H

// End of File
