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
// trkdummyrecognizer.cpp
//

// system includes
#include <apmrec.h>
#include <apmstd.h>
#include <eikenv.h>
// local includes
#include "trkdummyrecognizer.h"

// constants
const TInt KNumDataTypesSupported = 1;
const TInt KMaxBufferLength = 4;
const TInt KDummyRecognizerValue = 0x200159DC;
const TUid KUidDummyRecognizer = {KDummyRecognizerValue};
_LIT8(KDataTypeDummy, "text/vnd.nokia.trklauncherrec");

//
// CTrkDummyRecognizer
//
CTrkDummyRecognizer::CTrkDummyRecognizer()
:CApaDataRecognizerType(KUidDummyRecognizer, CApaDataRecognizerType::ENormal)
{
	iCountDataTypes = KNumDataTypesSupported;
}

TUint CTrkDummyRecognizer::PerferredBufSize()
{
	return KMaxBufferLength;
}

TDataType CTrkDummyRecognizer::SupportedDataTypeL(TInt /*aIndex*/) const
{
	return TDataType(KDataTypeDummy);
}
