/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef _DSTYPES_H_
#define _DSTYPES_H_

#ifndef ui8
typedef unsigned char	ui8;
#endif

#ifndef ui16
typedef unsigned short	ui16;
#endif

#ifndef ui32
typedef unsigned long	ui32;
#endif

typedef struct {
	ui16	hword_0;
	ui16	hword_1;
	ui16	hword_2;
	ui16	hword_3;
	ui16	hword_4;
} ui80;

typedef struct {
	ui32	word_0;
	ui32	word_1;
	ui32	word_2;
	ui32	word_3;
} ui128;

#if __MWERKS__
  #if __option( longlong )
	typedef unsigned long long ui64;
  #else /* NOT __option( longlong ) */
    typedef struct {
		ui32	word_0;
		ui32	word_1;
  	} ui64;
  #endif /* if/else __option( longlong ) */
#else /* NOT __MWERKS__ */
  #if __GNUC__
	typedef unsigned long long ui64;
  #else /* NOT __GNUC__ */
    typedef struct { ui32 word_0, word_1; } ui64;
  #endif /* if/else __GNUC__ */
#endif /* if/else __MWERKS__ */



#if defined( __CWDS__ ) || defined( __NEED_BOOL__ ) /* see __config.h */
	typedef int bool;
	#define false (0)
	#define true (1)
#endif

#endif /* _DSTYPES_H_ */
