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

#ifndef __OstProtDefs_H__
#define __OstProtDefs_H__

#define OST_HEADER_LENGTH 4

#define VERS_FIELD_INDEX 0
#define PROTID_FIELD_INDEX 1
#define LENGTH_FIELD_INDEX 2

#define OST_PROTOCOL_VERSION 0x01 // first internal Nokia verion is 0.1
#define OST_VERSION_PROTOCOL_VERSION 0x00

// Constants
const TInt KOstProtMajorVN = 1;
const TInt KOstProtMinorVN = 0;



enum TOstProtIds
{
	//Custom check
	EOstProtOstSystem = 0x00,
	EOstProtTrcActivation = 0x01,
	EOstProtTrk = 0x90,	
	EOstProtTraceCore = 0x91
		
};

#endif