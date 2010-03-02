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

#include "dbgtrcclidefs.h"
#include "dbgtrccmdcodes.h"


//
// DbgTrcCliDefs (source)
//

//
// DbgTrcCliDefs::ServerAndThreadName()
//
// Returns the DbgTrc server's unique name.
// @return Unique name of the DbgTrc server.
//
const TDesC& DbgTrcCliDefs::ServerAndThreadName()
{
	return KDbgTrcServerName;
}

//
// DbgTrcCliDefs::ServerImageName()
//
// Return the name of the server image (Exe/Dll)
//
const TDesC& DbgTrcCliDefs::ServerImageName()
{
	return KDbgTrcServerImageName;
}

//
// DbgTrcCliDefs::ServerUidType()
//
// Return the Uid associated with the server image
//
TUidType DbgTrcCliDefs::ServerUid()
{
	return TUidType(KNullUid, KNullUid, KDbgTrcServerUid);
}


//
// DbgTrcCliDefs::Version()
//
// Return the version number of the server/client
//
TVersion DbgTrcCliDefs::Version()
{
	return TVersion(KDbgTrcServerMajorVN, KDbgTrcServerMinorVN, KDbgTrcServerBuildVN);
}


