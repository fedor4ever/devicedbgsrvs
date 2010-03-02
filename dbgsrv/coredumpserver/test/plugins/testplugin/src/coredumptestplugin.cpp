// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include "coredumptestplugin.h"

#include <e32debug.h>
#include <debuglogging.h>

// Two phase construction of implementation
CTestFormatter* CTestFormatter::NewL()
	{
	LOG_MSG( "CTestFormatter::NewL()\n" );
	CTestFormatter* self = CTestFormatter::NewLC();
	LOG_MSG( "CTestFormatter::NewL() <- CTestFormatter::NewLC()\n" );

	LOG_MSG( "  NewL() -> Pop()\n" );
	CleanupStack::Pop();
	LOG_MSG( "  NewL() <- Pop()\n" );
	return self;
	}


CTestFormatter* CTestFormatter::NewLC()
	{
	LOG_MSG( "CTestFormatter::NewLC() -> new(ELeave)CTestFormatter\n" );
	CTestFormatter* self = new(ELeave)CTestFormatter();
	CleanupStack::PushL(self);

	self->ConstructL();
	LOG_MSG( "CTestFormatter::NewL() <- self->ConstructL()\n" );

	return self;
	}


CTestFormatter::CTestFormatter()
	{
	}

void CTestFormatter::ConstructL()
	{
	}

CTestFormatter::~CTestFormatter()
	{
	LOG_MSG( "CTestFormatter::~CTestFormatter()\n" );

	}

// Interface methods
void CTestFormatter::ConfigureDataSaveL( CCrashDataSave * aDataSave )
	{
	LOG_MSG( "CTestFormatter::ConfigureDataSaveL()\n" );
    User::LeaveIfNull( aDataSave );
	}

	
void CTestFormatter::ConfigureDataSourceL( CCrashDataSource * aDataSource )
	{
	LOG_MSG( "CTestFormatter::ConfigureDataSource()\n" );
    User::LeaveIfNull( aDataSource );
	}


TInt CTestFormatter::GetNumberConfigParametersL( )
	{
	return ( 0 );
	}


COptionConfig * CTestFormatter::GetConfigParameterL( const TInt aIndex )
	{
	// return the config identified by aIndex
	return ( (COptionConfig *) NULL );
	}


void CTestFormatter::SetConfigParameterL( const TInt aIndex, 
										  const TInt32 & aValue, 
										  const TDesC & aDescValue )
	{
	LOG_MSG( "CTestFormatter::SetConfigParameterL()\n" );

	TInt remIntWarn = aIndex;
	TInt32 remInt32Warn = aValue;
	remIntWarn = remInt32Warn;
	remInt32Warn = remIntWarn;
	TInt remInt2Warn = aDescValue.Length();
	}


void CTestFormatter::GetDescription( TDes & aPluginDescription )
	{
	LOG_MSG( "CTestFormatter::GetDescription()\n" );

	_LIT( KPluginDescription, "CTestFormatter::GetDescription" );
	aPluginDescription = *(KPluginDescription().AllocL());
	}


void CTestFormatter::CrashEventL( TCrashInfo* aCrashInfo) 
	{
	LOG_MSG( " CTestFormatter::CrashEvent()\n" );

	User::LeaveIfNull( aCrashInfo );
	}

TInt CTestFormatter::SetToUseFlashDataSource()
	{
	return KErrNone;
	}


	
