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

#ifndef __Toolsclient_pan__
#define __Toolsclient_pan__

//  Data Types

enum TToolsClientPanic
    {
    EToolsClientNullPointer
    };

//  Function Prototypes

GLREF_C void Panic(TToolsClientPanic aPanic);

#endif  // __toolsclient_pan__

