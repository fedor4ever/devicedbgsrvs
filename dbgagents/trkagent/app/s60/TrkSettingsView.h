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

#ifndef TRKSETTINGSVIEW_H
#define TRKSETTINGSVIEW_H

// INCLUDES
#include <aknview.h>
   

// CONSTANTS

// FORWARD DECLARATIONS
class CTrkSettingsContainer;
class CTrkSettingsData;

// CLASS DECLARATION

class CTrkSettingsView : public CAknView
{
    public: // Constructors and destructor
        /**
        * Symbian two-phased constructor.
        */
        static CTrkSettingsView* NewL(CTrkSettingsData* aSettingsData);

        /**
        * Destructor.
        */
        virtual ~CTrkSettingsView();

        /** 
        * Handle screen size change.
        * @since 2.8
        */
        void HandleSizeChange();

    private: // Functions from base classes
        
        /**
        * From CAknView returns Uid of View
        * @return TUid uid of the view
        */
        TUid Id() const;

        /**
        * From CAknView, handle commands
        * @param aCommand command to be handled
        */
        void HandleCommandL( TInt aCommand );

        /**
        * From CAknView Activate this view
        * @param aPrevViewId 
        * @param aCustomMessageId 
        * @param aCustomMessage 
        */
        void DoActivateL( const TVwsViewId& aPrevViewId,
                          TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );

        /**
        * From CAknView Deactivate this view
        */
        void DoDeactivate();

   
        /**
        * From MEikMenuObserver Dynamically customize menu items
        * @param aResourceId Menu pane resource ID
        * @param aMenuPane Menu pane pointer
        */
        void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );
        
        void HandleStatusPaneSizeChange();

    private:
        /**
        * C++ constructor.
        */
        CTrkSettingsView(CTrkSettingsData* aSettingsData);

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();

    private: // Data
        CTrkSettingsContainer* iContainer;
        CTrkSettingsData* iSettingsData;
        TBool iViewActivated;
};

#endif //TRKSETTINGSVIEW_H

// End of File
