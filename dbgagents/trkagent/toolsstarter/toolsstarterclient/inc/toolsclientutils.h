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

#ifndef __toolsclientutils_h__
#define __toolsclientutils_h__

// System includes
#include <e32base.h>


//
//  ToolsClientUtils (header)
//
class ToolsClientUtils
{
public:										// ENUMERATIONS
	/** Panic numbers associated with the TRKTCBCLI panic category. */
	enum TToolsClientPanic
	{
		EToolsClientPanicArgument			= 0,
		EToolsClientPanicCancel			= 1,
		EToolsClientPanicLast
	};
	/** Panic numbers associated with the TRKTCBCLIFAULT panic category. */
	enum TToolsClientFault
	{
		EToolsClientFaultLogic					= 0,
		EToolsClientFaultData					= 1,
		EToolsClientFaultCouldNotLoadServer	= 2,
		EToolsClientFaultDebugFuncError		= 3,
		EToolsClientFaultLast
	};

public:

	static void	Panic(TToolsClientPanic aPanic);

	static void	Fault(TToolsClientFault aFault);

	static TInt	StartToolsServer();
};

#endif //__toolsclientutils_h__

