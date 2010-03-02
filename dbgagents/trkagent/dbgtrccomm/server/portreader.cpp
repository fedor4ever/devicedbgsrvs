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

#include <e32cons.h>
#include <e32base.h>

#include "logging.h"
#include "portreader.h"



CPortReader* CPortReader::NewL(RComm& aPort)
{
	LOG_MSG("CPortReader::NewL");

	CPortReader* self = new(ELeave) CPortReader(aPort);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CPortReader::CPortReader(RComm& aPort) 
: CActive(CActive::EPriorityStandard),
  iPort(aPort),
  iAlreadyReading(EFalse)
{
	LOG_MSG("CPortReader::CPortReader");

}

CPortReader::~CPortReader()
{
	LOG_MSG("CPortReader::~CPortReader");

	Cancel();

}

void CPortReader::ConstructL()
{
	LOG_MSG("CPortReader::ConstructL");
	
	CActiveScheduler::Add(this);
}

void CPortReader::StartRead(MDataListener* aListener)
{
	LOG_MSG("CPortReader::StartRead");

	if (iAlreadyReading)
		return;
	
	iListener = aListener;
	
	IssueReadRequest();
	iAlreadyReading = ETrue;
}

void CPortReader::IssueReadRequest()
{
	LOG_MSG("CPortReader::IssueReadRequest");

	iNextReadChar = 0;
	iReceivedChars.Zero();
	iPort.ReadOneOrMore(iStatus, iReceivedChars);

	SetActive();
}

void CPortReader::RunL()
{
	LOG_MSG("CPortReader::RunL");

	// pass the data onto listener
	if (iStatus.Int() == KErrNone && iListener)		
		iListener->DataAvailable(iReceivedChars, iReceivedChars.Length());

	// continue waiting for data
	IssueReadRequest();	
}

void CPortReader::DoCancel()
{	
	LOG_MSG("CPortReader::DoCancel");

	iPort.ReadCancel();
	
	iAlreadyReading = EFalse;
}

