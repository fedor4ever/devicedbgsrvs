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

#ifndef __TOOLSSTARTERAUTOLAUNCH_H
#define __TOOLSSTARTERAUTOLAUNCH_H


//  INCLUDES
#include <apmrec.h>
#include <e32std.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Implements CToolsStarterAutoLaunch
*/
class CToolsStarterAutoLaunch : public CApaDataRecognizerType
    {
public: // new functions
    static CApaDataRecognizerType* CreateRecognizerL();
    CToolsStarterAutoLaunch();
    static void CreateAutoStartThreadL();
    static TInt StartAppThreadFunction( TAny* aParam );

public: // from CApaDataRecognizerType
    TUint PreferredBufSize();
    TDataType SupportedDataTypeL( TInt aIndex ) const;

private: // from CApaDataRecognizerType
    void DoRecognizeL( const TDesC& aName, const TDesC8& aBuffer );
    };


class CToolsStarterLauncher : public CBase
    {
public:
    static CToolsStarterLauncher* NewL();
    ~CToolsStarterLauncher();

private:
    CToolsStarterLauncher();
    void ConstructL();

public:
    void StartServer();
    void WaitForUiServices();
        
    };

#endif // __TOOLSSTARTERAUTOLAUNCH_H

// End of File
