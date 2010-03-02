// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include <e32base.h>
#include "testcrashdatasave.h"

CTestDataSave::~CTestDataSave()
{
    iData.Close();
}

CTestDataSave* CTestDataSave::NewL()
{
    CTestDataSave* self = new(ELeave) CTestDataSave();
    CleanupStack::PushL(self);
    //self->ConstructL();
    self->iData.CreateL(0);
    CleanupStack::Pop(self);
    return self;
}

TInt CTestDataSave::Open(const TDesC& aParam )
{
    iData.Zero();
	return KErrNone;
}

TInt CTestDataSave::Close()
{
     iData.ZeroTerminate();
	 return KErrNone;
}

TInt CTestDataSave::Write( const TDesC8& aData)
{
    iData.ReAlloc(iData.Length() + aData.Length());
    iData.Append(aData);
	//Print();
	return KErrNone;
}

TInt CTestDataSave::Write( TAny* aData, TUint aSize)
{
    TPtrC8 data(static_cast<TUint8*>(aData), aSize);
    return ( Write(data) );
}

TInt CTestDataSave::Write( TInt aPos, const TDesC8& aData )
{
	if( 0 == iData.Length() )
		{
		iData.CreateMax( aPos + aData.Length() );
		}
	else if( aPos >= iData.Length() )
		{
		iData.ReAlloc( aPos + aData.Length() );
		iData.SetMax();

		}
	else if( (aPos + aData.Length()) > iData.Length() )
		{
		iData.ReAlloc( aPos - iData.Length() + aData.Length() );
		}

	iData.Replace( aPos, aData.Length(), aData );

	return KErrNone;
}


TInt CTestDataSave::Write( TInt aPos, TAny* aData, TUint aSize )
	{
    TPtrC8 data(static_cast<TUint8*>(aData), aSize);
    return ( Write(aPos, data) );
	}

void CTestDataSave::Print() const
	{
	RBuf8 printBuf;
	printBuf.Create( iData, iData.Length()+1 );
	char* clPrompt = (char*) printBuf.PtrZ();
	RDebug::Printf("  CTestDataSave::Print() length=%d, iData=>>>%s<<<\n", iData.Length(), clPrompt );
	printBuf.Close();
	}
