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

#ifndef __LOGGING_H__
#define __LOGGING_H__


#define _LOGGING_


#ifdef _LOGGING_
#define LOG_MSG(x) RDebug::Printf(x)
#define LOG_MSG2(x, y) RDebug::Printf(x, y)
#define LOG_MSG3(x, y, z) RDebug::Printf(x, y, z)
#else
#define LOG_MSG(x)
#define LOG_MSG2(x, y)
#define LOG_MSG3(x, y, z)
#endif


#endif // __LOGGING_H__

// End of File

