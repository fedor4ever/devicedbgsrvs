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


#include "DateTimeConverter.h"

//
//
// TDateTimeConverter implementation
//
//

//
// TDateTimeConverter constructor
//
// Accepts Win32 date/time format and converts it to Symbian OS format
//
TDateTimeConverter::TDateTimeConverter(TUint32 aWinTimeDate)
{
	iWinTimeDate.winTimeDate = aWinTimeDate;
	iEpocTimeDate.Set((TInt)(iWinTimeDate.t.year + 1980), (TMonth)(iWinTimeDate.t.month - 1),
					  (TInt)(iWinTimeDate.t.day - 1), (TInt)iWinTimeDate.t.hour,
					  (TInt)iWinTimeDate.t.minute, (TInt)(iWinTimeDate.t.second * 2), 0);
}

//
// TDateTimeConverter constructor
//
// Accepts Win32 date/time format and converts it to Symbian OS format
//
TDateTimeConverter::TDateTimeConverter(TDateTime aEpocTimeDate)
	: iEpocTimeDate(aEpocTimeDate)
{
	iWinTimeDate.t.hour = aEpocTimeDate.Hour();
	iWinTimeDate.t.minute = aEpocTimeDate.Minute();
	iWinTimeDate.t.second = (aEpocTimeDate.Second() / 2);
	iWinTimeDate.t.year = aEpocTimeDate.Year() - 1980;
	iWinTimeDate.t.month = aEpocTimeDate.Month() + 1;
	iWinTimeDate.t.day = aEpocTimeDate.Day() + 1;
}

//
// TDateTimeConverter::GetWinTimeDate
//
// Returns the date/time in Win32 format
//
TUint32 TDateTimeConverter::GetWinTimeDate()
{
	return iWinTimeDate.winTimeDate;
}

//
// TDateTimeConverter::GetEpocTimeDate
//
// Returns the date/time in Symbian OS format
//
TDateTime TDateTimeConverter::GetEpocTimeDate()
{
	return iEpocTimeDate;
}
