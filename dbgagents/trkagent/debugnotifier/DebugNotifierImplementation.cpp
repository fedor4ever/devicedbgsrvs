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

//  Include Files  

#include "DebugNotifierimplementation.h"	// CDebugNotifier

_LIT(KDeBugNotification,"Debug Services are available through USB");
//
// CDebugNotifier* FactoryFunction()
//
// Entry point for the dll
//
EXPORT_C CDebugNotifier* FactoryFunction()
{
    return new CDebugNotifier();    
}

//
// CDebugNotifier::NotifyTheUser()
//
// This is the method to notify the user about the debug services
//
void CDebugNotifier::NotifyTheUserL()
{
    CAknGlobalNote* debugNote = CAknGlobalNote::NewL();
    CleanupStack::PushL( debugNote );
    TInt noteid = debugNote->ShowNoteL(EAknGlobalConfirmationNote, KDeBugNotification);
    CleanupStack::PopAndDestroy();    
}


