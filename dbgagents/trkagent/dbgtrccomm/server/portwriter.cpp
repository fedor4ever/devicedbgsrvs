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

#include "logging.h"
#include "portwriter.h"



CPortWriter* CPortWriter::NewL(RComm& aPort)
{
	LOG_MSG("CPortWriter::NewL");

	CPortWriter* self = new(ELeave) CPortWriter(aPort);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CPortWriter::CPortWriter(RComm& aPort)
: CActive(CActive::EPriorityStandard),
  iPort(aPort),
  iListener(NULL),
  iAlreadyWriting(EFalse)
{
	LOG_MSG("CPortWriter::CPortWriter");


}

CPortWriter::~CPortWriter()
{
	LOG_MSG("CPortWriter::~CPortWriter");
	Cancel();
}

void CPortWriter::ConstructL()
{
	LOG_MSG("CPortWriter::ConstructL");

	CActiveScheduler::Add(this);
}

void CPortWriter::StartWrite(TDesC8& aMsg, MDataListener* aListener)
{
	LOG_MSG("CPortWriter::StartWrite");
	iAlreadyWriting = ETrue;
	iListener = aListener;
	
	iPort.Write(iStatus, aMsg);
	
	SetActive();	
}

void CPortWriter::RunL()
{
	LOG_MSG("CPortWriter::RunL");

	iAlreadyWriting = EFalse;

	if (iListener)
		iListener->DataWriteComplete(iStatus.Int());
	else
		LOG_MSG("Invalid listener, something wrong");	
}

void CPortWriter::DoCancel()
{
	LOG_MSG("CPortWriter::DoCancel");

	iPort.WriteCancel();
}


