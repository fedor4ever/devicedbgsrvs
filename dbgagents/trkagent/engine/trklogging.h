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

#if defined(__TRKLOGGING__)

    #include <e32debug.h>

    #define LOG_MSG( a )              RDebug::Printf( a )
    #define LOG_MSG2( a, b )          RDebug::Printf( a, b )
    #define LOG_MSG3( a, b, c )       RDebug::Printf( a, b, c )
    #define LOG_MSG4( a, b, c, d )    RDebug::Printf( a, b, c, d )
    #define LOG_MSG5( a, b, c, d, e )    RDebug::Printf( a, b, c, d, e )
	#define LOG_MSG6( a, b, c, d, e, f ) RDebug::Printf( a, b, c, d, e, f )

#else

  #define LOG_MSG( a )
  #define LOG_MSG2( a, b )
  #define LOG_MSG3( a, b, c )
  #define LOG_MSG4( a, b, c, d )
  #define LOG_MSG5( a, b, c, d, e )
  #define LOG_MSG6( a, b, c, d, e, f )

#endif
