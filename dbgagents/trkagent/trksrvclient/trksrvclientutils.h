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

#ifndef __TrkSrvClientUtils_H__
#define __TrkSrvClientUtils_H__

// System includes
#include <e32base.h>


//
//  TrkSrvClientUtils (header)
//
class TrkSrvClientUtils
{
public:										// ENUMERATIONS
	/** Panic numbers associated with the TRKSRVCLI panic category. */
	enum TTrkSrvClientPanic
	{
		ETrkSrvClientPanicArgument			= 0,
		ETrkSrvClientPanicCancel			= 1,
		ETrkSrvClientPanicLast
	};
	/** Panic numbers associated with the TRKSRVCLIFAULT panic category. */
	enum TTrkSrvClientFault
	{
		ETrkSrvClientFaultLogic					= 0,
		ETrkSrvClientFaultData					= 1,
		ETrkSrvClientFaultCouldNotLoadServer	= 2,
		ETrkSrvClientFaultDebugFuncError		= 3,
		ETrkSrvClientFaultLast
	};

public:

	static void	Panic(TTrkSrvClientPanic aPanic);

	static void	Fault(TTrkSrvClientFault aFault);

	static TInt	StartTrkServer();
};

#endif //__TrkTcbClientUtils_H__

