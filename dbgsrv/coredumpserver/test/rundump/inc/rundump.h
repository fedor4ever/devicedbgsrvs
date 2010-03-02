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


#ifndef __RUNDUMP_H__
#define __RUNDUMP_H__


//  Include Files

#include <e32base.h>
#include "optionconfig.h"
#include <coredumpinterface.h>


//  Function Prototypes

GLDEF_C TInt E32Main();
void DoConfigureL(const TUint32& aIndex, 
	     const TUint32& aUID, 
	        const COptionConfig::TParameterSource& aSource, 
	        const COptionConfig::TOptionType& aType, 
	        const TDesC& aPrompt, 
	        const TUint32& aNumOptions,
	        const TDesC& aOptions,
	        const TInt32& aVal, 
	        const TDesC& aStrValue,
	        const TUint aInstance,
	        const RCoreDumpSession &sess);


#endif  // __RUNDUMP_H__


