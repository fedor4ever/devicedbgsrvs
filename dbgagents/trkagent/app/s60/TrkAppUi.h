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

#include <eikappui.h>
#include <aknViewAppUi.h>
#include <AknGlobalNote.h>

#include "trksrvclisession.h"
#include "TrkAppView.h"

class CTrkSettingsView;
class CTrkSettingsData;

//
//
// CTrkAppUi
//
//
class CTrkAppUi : public CAknViewAppUi
{
public:

    void ConstructL();
	~CTrkAppUi();

public: // New functions
    void SetTitleL(TInt aResourceId);
    void SetTitleL(const TDesC& aText);

private:

    void HandleCommandL(TInt aCommand);

   /**
    * From CEikAppUi
    * Handle skin change event.
    */
    void HandleResourceChangeL( TInt aType );
	    
private:
	CTrkSettingsData* iSettingsData;
	CTrkAppView* iAppView;
	CTrkSettingsView* iSettingsView;
    RTrkSrvCliSession iTrkSession;
};

