// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//



/**
 @file tcds_unitBlockController.cpp
 @internalTechnology
*/

#include "tcds_unitBlockController.h"

_LIT(KCFlashDataSourceWrapper, "CFlashDataSourceWrapper");
_LIT(KServerDataSourceWrapper, "CServerDataSourceWrapper");


Ctcds_unitBlockController* Ctcds_unitBlockController::NewL()
/**
 * @return - Instance of the test block controller
 */
    {
    RDebug::Printf("Ctcds_unitBlockController::NewL()");
    Ctcds_unitBlockController* block = new (ELeave) Ctcds_unitBlockController();
    return block;
    }


CDataWrapper* Ctcds_unitBlockController::CreateDataL(const TDesC& aData)
    {
    
    RDebug::Printf("Ctcds_unitBlockController::CreateDataL()");
    CDataWrapper* wrapper = NULL;
	if (KCFlashDataSourceWrapper() == aData)
		{
		wrapper = CFlashDataSourceWrapper::NewL();
		}
	if (KServerDataSourceWrapper() == aData)
		{
		wrapper = CServerDataSourceWrapper::NewL();
		}	

    return wrapper;
    }
