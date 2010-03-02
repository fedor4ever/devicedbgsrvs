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
// Implementation of the symbian elf options class.
//



/**
 @file
 @internalTechnology
 @released
*/

#include <e32base.h>
#include <e32debug.h>

#include "symbianelfoptions.h"
#include <debuglogging.h>

/**
1st stage construction 
@return pointer to the newly created object, caller takes ownership of the object.
*/
CElfOptions* CElfOptions::NewL( TUint32 aUID )
{
	//LOG_MSG( "CElfOptions::NewL()\n" );
	CElfOptions* self = CElfOptions::NewLC( aUID );
	CleanupStack::Pop();
	return self;
}

/**
1st stage construction 
@return pointer to the newly created object, caller takes ownership of the object.
*/
CElfOptions* CElfOptions::NewLC( TUint32 aUID )
{
	//LOG_MSG( "CElfOptions::NewLC()\n" );

	CElfOptions* self = new(ELeave) CElfOptions();

	CleanupStack::PushL(self);

	self->ConstructL( aUID );
	return self;
}

/**
2nd stage construction
Reserves config parameter list, creates config parameter objects and sets them to default values.
@leave err one of the system wide error codes
*/
void CElfOptions::ConstructL( TUint32 aUID )
{
	//LOG_MSG("->CElfOptions::ConstructL()\n");
    TInt err;

	iConfigList.ReserveL( (TInt)EElfOptionsLast );

	COptionConfig * config;
	TInt index = 0; // Index of config params that we create

	_LIT( KTrueFalseOpt, "True,False" );
	_LIT( KTrueOpt, "True" );
	
	_LIT( KCreateSystemInfo, "System-Wide Crash Data Option:\n  0-System-Wide Data,\n  1-Crashed Process Only,\n  2-Both" );
	config = COptionConfig::NewL(  (TInt)ECreateSystemCrashInfo,
									aUID,						
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETUInt, 
									KCreateSystemInfo, 
                                    1,
									KNullDesC, 
									ESystemWideAndCrashedProc,						
									KNullDesC );
	err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

	
	index ++;


	_LIT( KCreateProcessDataPrompt, 
		"Create Data for all Threads in Crashed Process?" );
//      0        1         2         3         4         5
//      12345678901234567890123456789012345678901234657890

	config = COptionConfig::NewL(  (TInt)ECreateProcessData,
									aUID,						// UID
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETBool, 
									KCreateProcessDataPrompt, 
                                    1,
									KTrueFalseOpt, 
									1,						// Default Value = 1 == True
									KTrueOpt );				// Default Option String = True
	err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

	index ++;

	_LIT( KDataSegmentPrompt, "Create Data Segments?" );
	config = COptionConfig::NewL(  (TInt)ECreateDataSegments,
									aUID,						// UID
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETBool, 
									KDataSegmentPrompt, 
									1,						// Number of options
									KTrueFalseOpt,				// Option string
									1,						// Default Value = 1 == True
									KTrueOpt );				// Default Option String = True
	err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

	index ++;

	_LIT( KCodeSegmentPrompt, "Create Code Segments?" );
	config = COptionConfig::NewL(  (TInt)ECreateCodeSegments,
									aUID,						// UID
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETBool, 
									KCodeSegmentPrompt, 
									1,
									KTrueFalseOpt, 
									1,						// Default Value = 1 == True
									KTrueOpt );				// Default Option String = True
	err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

	index ++;

	_LIT( KThreadSegmentPrompt, "Create Thread Segments?" );
	config = COptionConfig::NewL(  (TInt)ECreateThreadSegments,
									aUID,						// UID
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETBool, 
									KThreadSegmentPrompt, 
                                    1, 
									KTrueFalseOpt, 
									1,						// Default Value = 1 == True
									KTrueOpt );				// Default Option String = True
	err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

	index ++;

	_LIT( KRegisterSegmentPrompt, "Create Register Segments?" );
	config = COptionConfig::NewL(  (TInt)ECreateRegisterSegments,
									aUID,						// UID
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETBool, 
									KRegisterSegmentPrompt, 
                                    1,
									KTrueFalseOpt, 
									1,						// Default Value = 1 == True
									KTrueOpt );				// Default Option String = True
	err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

	index ++;

	_LIT( KCreateExecInfoPrompt, "Create Exec Info?" );
	config = COptionConfig::NewL(  (TInt)ECreateExecSegments,
									aUID,						// UID
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETBool, 
									KCreateExecInfoPrompt, 
                                    1,
									KTrueFalseOpt, 
									1,						// Default Value = 1 == True
									KTrueOpt );				// Default Option String = True
	err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

	index ++;

	_LIT( KCreateProcessSegmentInfo, "Create Process Info?" );
	config = COptionConfig::NewL(  (TInt)ECreateProcessSegments,
									aUID,						// UID
									COptionConfig::EFormatterPlugin,
									COptionConfig::ETBool, 
									KCreateProcessSegmentInfo, 
                                    1,
									KTrueFalseOpt, 
									1,						// Default Value = 1 == True
									KTrueOpt );				// Default Option String = True
	err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

	index ++;
}

/**
ctor nothing really
*/

CElfOptions::CElfOptions()
{
	
}

/**
dtor frees config parameter list
*/
CElfOptions::~CElfOptions()
{
    iConfigList.ResetAndDestroy();
}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptions::CreateProcessData() const
	{
	return iConfigList[ (TInt)ECreateProcessData ]->ValueAsBool();
	}
	
/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptions::CreateDataSegments() const
	{
	if( CreateSystemWideOnly() )
		return EFalse;

	return iConfigList[ (TInt)ECreateDataSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptions::CreateCodeSegments() const
	{
	if( CreateSystemWideOnly() )
		return EFalse;

	return iConfigList[ (TInt)ECreateCodeSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptions::CreateThreadSegments() const
	{
	return iConfigList[ (TInt)ECreateThreadSegments ]->ValueAsBool();
	}
	
/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptions::CreateRegisterSegments() const
	{
	if( CreateSystemWideOnly() )
		return EFalse;

	return iConfigList[ (TInt)ECreateRegisterSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptions::CreateExecSegments() const
	{
	if( CreateSystemWideOnly() )
		return EFalse;

	return iConfigList[ (TInt)ECreateExecSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptions::CreateProcessSegments() const
	{
    return iConfigList[ (TInt)ECreateProcessSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptions::CreateSystemWideOnly() const
	{
	return ( ESystemWideOnly == iConfigList[ (TInt)ECreateSystemCrashInfo ]->Value() );
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptions::CreateSystemCrashInfo() const
	{

	switch( iConfigList[ (TInt)ECreateSystemCrashInfo ]->Value() )
		{
		case ESystemWideOnly:
		case ESystemWideAndCrashedProc:
			return ETrue;
		case ECrashedProcessOnly:
		default:
			return EFalse;
		}
	}

/**
Called by CDS to ask for the configuration parameters that the formatter implements.
@return actual number of implemented config parameters
*/
TInt CElfOptions::GetNumberConfigParametersL( )
	{
	//LOG_MSG("->CElfOptions::GetNumberConfigParametersL() : returning %d", iConfigList.Count() );
	return iConfigList.Count();
	}

/**
Called by CDS to ask for configuration parameter.
@param aIndex indicates which parameter to return
@return pointer to COptionConfig object representing the requested config parameter. Caller doesn't take ownership of the object!
@leave KErrBadHandle if index is out of bounds
@see COptionConfig
*/
COptionConfig * CElfOptions::GetConfigParameterL( const TInt aIndex )
	{

	if( ( aIndex < 0 ) || ( aIndex >= iConfigList.Count() ) )
		{
		User::Leave( KErrBadHandle );
		}

    return iConfigList[aIndex];

	}

/**
Called by CDS with response to prompt from user. Plugin interprets 
the response (eg create file, check permissions, pre-allocate memory etc).

@param aIndex Index of parameter to change
@param aValue Unused 
@param aDescValue Path and filename to use
@return KErrNone
@leave KErrBadHandle if index is out of bounds or one of the other system wide error codes
*/
TInt CElfOptions::SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue )
	{
	//LOG_MSG2("->CElfOptions::SetConfigParameterL()\n");
    
	// Make the changes to our internal structures, and reflect such changes in our behaviour

	if( ( aIndex < 0 ) || ( aIndex > iConfigList.Count() ) )
		{
		LOG_MSG2( "CElfOptions::SetConfigParameterL(): ERROR !* : aIndex=%d is out of bounds\n", aIndex );
		User::Leave( KErrBadHandle );
		}

	COptionConfig * config = iConfigList[aIndex];

	switch( config->Index() )
		{
		case ECreateProcessData:
		case ECreateDataSegments:
		case ECreateCodeSegments:
		case ECreateThreadSegments:
		case ECreateRegisterSegments:
		case ECreateExecSegments:
		case ECreateProcessSegments:
			if( 0 == aValue )
				{
				config->Value( EFalse );
				}
			else if( 1 == aValue )
				{
				config->Value( ETrue );
				}
			else
				{
				User::Leave( KErrArgument );
				}
			break;

		case ECreateSystemCrashInfo:
			{
			if( ESystemWideOnly == aValue )
				{
				config->Value( ESystemWideOnly );
				}
			else if( ECrashedProcessOnly == aValue )
				{
				config->Value( ECrashedProcessOnly );
				}
			else if( ESystemWideAndCrashedProc == aValue )
				{
				config->Value( ESystemWideAndCrashedProc );
				}
			else
				{
				User::Leave( KErrArgument );
				}
			}
		}//switch

	return KErrNone;
	}
