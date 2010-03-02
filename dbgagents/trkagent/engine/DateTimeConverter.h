/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __DATETIMECONVERTER_H__
#define __DATETIMECONVERTER_H__

#include <e32std.h>

//
// class TDateTimeConverter
//
// Converts between Win32 time and Symbian OS time
//
class TDateTimeConverter
{
public:

	TDateTimeConverter(TUint32 aWinTimeDate);
	TDateTimeConverter(TDateTime aEpocTimeDate);
	
	TUint32 GetWinTimeDate();
	TDateTime GetEpocTimeDate();

private:

	union TWinTimeDate
	{
		struct
		{
			unsigned day:5;
			unsigned month:4;
			unsigned year:7;
			unsigned second:5;
			unsigned minute:6;
			unsigned hour:5;
		} t;
		
		TUint32 winTimeDate;

	} iWinTimeDate;

	TDateTime iEpocTimeDate;
};

#endif // __DATETIMECONVERTER_H__
