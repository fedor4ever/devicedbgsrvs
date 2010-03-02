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
// Implements an ECOM Plugin for the purpose of testing the Core Dump Server
//



/**
 @file
 @internalTechnology
 @released
 @see CCoreDumpFormatter
*/


#ifndef CORE_DUMP_TEST_PLUGIN
#define CORE_DUMP_TEST_PLUGIN


// Abstract ECOM API definition
#include <formatterapi.h>


class CCrashDataSave;
class CCrashDataSource;
class TCrashInfo;

/** 
CTestFormatter test class. Only used for internal testing
*/
class CTestFormatter : public CCoreDumpFormatter
{
public:
	static CTestFormatter* NewL();
	static CTestFormatter* NewLC();
	virtual ~CTestFormatter();

public:

	virtual void ConfigureDataSaveL( CCrashDataSave * aDataSave );

	virtual void ConfigureDataSourceL( CCrashDataSource * aDataSource );
	
	virtual void GetDescription( TDes & aPluginDescription );

	virtual void CrashEventL( TCrashInfo* aCrashInfo);

	virtual TInt GetNumberConfigParametersL( );

	virtual COptionConfig * GetConfigParameterL( const TInt aIndex );

	virtual void SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue );

	TInt SetToUseFlashDataSource();
	
public:

private:
	void ConstructL();
	CTestFormatter();
	
private:
	
	//CCrashDataSource* iDataSource;

};

#endif // 
