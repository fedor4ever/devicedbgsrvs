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

#include "TrkTcbClientDefs.h"
#include "TrkTcbCmdCodes.h"

// System includes

// User includes

// Type definitions

// Constants

// Enumerations


//
// TrkTcbClientDefs (source)
//

//
// TrkTcbClientDefs::ServerAndThreadName()
//
// Returns the TrkTcb server's unique name.
// @return Unique name of the TrkTcb server.
//
const TDesC& TrkTcbClientDefs::ServerAndThreadName()
{
	return KTrkTcbServerName;
}

//
// TrkTcbClientDefs::ServerImageName()
//
// Return the name of the server image (Exe/Dll)
//
const TDesC& TrkTcbClientDefs::ServerImageName()
{
	return KTrkTcbServerImageName;
}

//
// TrkTcbClientDefs::ServerUidType()
//
// Return the Uid associated with the server image
//
TUidType TrkTcbClientDefs::ServerUidType()
{
	return TUidType(KNullUid, KNullUid, KTrkTcbServerUid);
}


//
// TrkTcbClientDefs::Version()
//
// Return the version number of the server/client
//
TVersion TrkTcbClientDefs::Version()
{
	return TVersion(KTrkTcbServerMajorVN, KTrkTcbServerMinorVN, KTrkTcbServerBuildVN);
}


