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

#include "trksrvclientdefs.h"
#include "trksrvcmdcodes.h"

// System includes

// User includes

// Type definitions

// Constants

// Enumerations


//
// TrkSrvClientDefs (source)
//

//
// TrkSrvClientDefs::ServerAndThreadName()
//
// Returns the Trk server's unique name.
// @return Unique name of the Trk server.
//
const TDesC& TrkSrvClientDefs::ServerAndThreadName()
{
	return KTrkServerName;
}

//
// TrkSrvClientDefs::ServerImageName()
//
// Return the name of the server image (Exe/Dll)
//
const TDesC& TrkSrvClientDefs::ServerImageName()
{
	return KTrkServerImageName;
}

//
// TrkSrvClientDefs::ServerUidType()
//
// Return the Uid associated with the server image
//
TUidType TrkSrvClientDefs::ServerUidType()
{
	return TUidType(KNullUid, KNullUid, KTrkServerUid);
}


//
// TrkSrvClientDefs::Version()
//
// Return the version number of the server/client
//
TVersion TrkSrvClientDefs::Version()
{
	return TVersion(KTrkServerMajorVN, KTrkServerMinorVN, KTrkServerBuildVN);
}


