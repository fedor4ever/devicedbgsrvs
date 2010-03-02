/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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

// This file defines the API for ToolsClient.dll

#ifndef __ToolsCLIENT_H__
#define __ToolsCLIENT_H__

//  Include Files

#include <e32base.h>	// CBase
#include <e32std.h>	 // TBuf

//  Constants

const TInt KToolsClientBufferLength = 15;
typedef TBuf<KToolsClientBufferLength> TToolsClientExampleString;

//  Class Definitions

class CToolsClient : public CBase
    {
public:
    // new functions
    IMPORT_C static CToolsClient* NewL();
    IMPORT_C static CToolsClient* NewLC();
    IMPORT_C ~CToolsClient();

public:
    // new functions, example API
    IMPORT_C TVersion Version() const;
    IMPORT_C void ExampleFuncAddCharL(const TChar& aChar);
    IMPORT_C void ExampleFuncRemoveLast();
    IMPORT_C const TPtrC ExampleFuncString() const;

private:
    // new functions
    CToolsClient();
    void ConstructL();

private:
    // data
    TToolsClientExampleString* iString;
    };

#endif  // __ToolsCLIENT_H__

