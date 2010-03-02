// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __CoreDumpUI_DataStructs_H
#define __CoreDumpUI_DataStructs_H

/**
@file
@internalTechnology
@released
*/

#include <e32std.h> //descriptors, TUidType etc

struct TListData
/** Stores library, server, device and logical channel data */
{
	TFullName iName;
};

struct TUIProcessData
/** Stores process details */
	{
	TBuf<8> iIdString;
	TFileName iProcessName; // this is a TBuf (big)

	TBuf<3> iObserved;
	};

struct TUIThreadData
/** Stores thread details */
	{
	TBuf<8> iIdString;
	TFileName iThreadName; // this is a TBuf (big)
	TBuf<10> iPriorityString;
	TBuf<8> iOwnerIdString;
	TBuf<3>	 iObserved;
	};

struct TUIPluginData
/** Stores plugin details */
	{
	TFullName iName; // this is a TBuf (big)
	TBuf<128> iDescription;
	TBuf<10> iUid;	 // UID of the plugin
	TBuf<50> iType;   
	TBuf<8> iInstances;
	};
	
struct TUIExecutableData
/** Stores the executable details */
	{
	TFileName iExeName;
	TBuf<3> iObserved;
	};

struct TUIPluginInstanceData
/** Stores plugin instance data */
	{
	TBuf<8> iIdString;
	TFullName iName; // this is a TBuf (big)
	TBuf<128> iDescription;
	TBuf<10> iUid;	 // UID of the plugin
	TBuf<50> iType;  
	TBuf<10> iBoundTo;
	};

#endif
