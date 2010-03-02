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
// trkdummyrecognizer.h
//

#ifndef TRKDUMMYRECOGNIZER_H
#define TRKDUMMYRECOGNIZER_H

#include <apmrec.h>

class CTrkDummyRecognizer : public CApaDataRecognizerType
{
public:
	CTrkDummyRecognizer();

public:
	// from CApaDataRecognizerType
	TUint PerferredBufSize();
	TDataType SupportedDataTypeL(TInt aIndex) const;
	//swagata
	#ifdef __UI_FRAMEWORKS_V2__
	static CApaDataRecognizerType* NewL();
	#endif //__UI_FRAMEWORKS_V2__

};

#endif // TRKDUMMYRECOGNIZER_H
