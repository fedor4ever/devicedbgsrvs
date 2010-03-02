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

#include "toolsclientdefs.h"
#include "ToolsCmdCodes.h"

// System includes

// User includes

// Type definitions

// Constants

// Enumerations


//
// ToolsClientDefs (source)
//

//
// ToolsClientDefs::ServerAndThreadName()
//
// Returns the Tools server's unique name.
// @return Unique name of the Tools server.
//
const TDesC& ToolsClientDefs::ServerAndThreadName()
{
	return KToolsServerName;
}

//
// ToolsClientDefs::ServerImageName()
//
// Return the name of the server image (Exe/Dll)
//
const TDesC& ToolsClientDefs::ServerImageName()
{
	return KToolsServerImageName;
}

//
// ToolsClientDefs::ServerUidType()
//
// Return the Uid associated with the server image
//
TUidType ToolsClientDefs::ServerUidType()
{
	return TUidType(KNullUid, KNullUid, KToolsServerUid);
}


//
// ToolsClientDefs::Version()
//
// Return the version number of the server/client
//
TVersion ToolsClientDefs::Version()
{
	return TVersion(KToolsServerMajorVN, KToolsServerMinorVN, KToolsServerBuildVN);
}


