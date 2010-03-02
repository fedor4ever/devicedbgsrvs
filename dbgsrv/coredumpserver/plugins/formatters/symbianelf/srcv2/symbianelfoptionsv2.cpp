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
// symbianelfoptions.cpp
// Implementation of the symbian elf options class.
//

/**
 @file
 @internalTechnology
 @released
*/

#include <e32base.h>
#include <e32debug.h>

#include "symbianelfoptionsv2.h"
#include <debuglogging.h>

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CElfOptionsV2* CElfOptionsV2::NewL( TUint32 aUID )
{
	//LOG_MSG( "CElfOptionsV2::NewL()\n" );
	CElfOptionsV2* self = CElfOptionsV2::NewLC( aUID );
	CleanupStack::Pop();
	return self;
}

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CElfOptionsV2* CElfOptionsV2::NewLC( TUint32 aUID )
{
	//LOG_MSG( "CElfOptionsV2::NewLC()\n" );

	CElfOptionsV2* self = new(ELeave) CElfOptionsV2();

	CleanupStack::PushL(self);

	self->ConstructL( aUID );
	return self;
}

/**
2nd stage construction
Reserves config parameter list, creates config parameter objects and sets them to default values.
@leave err one of the system wide error codes
*/
void CElfOptionsV2::ConstructL( TUint32 aUID )
{
	//LOG_MSG("->CElfOptionsV2::ConstructL()\n");
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

    index++;

    _LIT( KCreateTraceData, "How much Trace data to capture (Kb)" );
    config = COptionConfig::NewL( (TInt)ECreateTraceData,
    							   aUID,
    							   COptionConfig::EFormatterPlugin,
								   COptionConfig::ETInt,
								   KCreateTraceData,
								   1,
								   KNullDesC,
								   100,
								   KNullDesC );
    err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

	index ++;

    _LIT( KCreateLockData, "Capture System Lock Data ?" );
    config = COptionConfig::NewL( (TInt)ECreateLockSegments,
    							   aUID,
    							   COptionConfig::EFormatterPlugin,
    							   COptionConfig::ETBool,
								   KCreateLockData,
								   1,
								   KTrueFalseOpt,
								   1,
								   KTrueOpt );
    err = iConfigList.Append( config );
    if(err != KErrNone)
    	{
        delete config;
        User::Leave(err);
    	}

    index ++;

    _LIT( KCreateRomBuildInfo, "Capture ROM build data ?" );
    config = COptionConfig::NewL( (TInt)ECreateRomInfo,
    							   aUID,
    							   COptionConfig::EFormatterPlugin,
								   COptionConfig::ETInt,
								   KCreateRomBuildInfo,
								   1,
								   KNullDesC,
								   100,
								   KNullDesC );
    err = iConfigList.Append( config );
    if(err != KErrNone)
    {
        delete config;
        User::Leave(err);
    }

    index ++;

    _LIT( KCreateHeapData, "Capture Heap Data ?" );
    config = COptionConfig::NewL( (TInt)ECreateHeapInfo,
    							   aUID,
    							   COptionConfig::EFormatterPlugin,
    							   COptionConfig::ETBool,
    							   KCreateHeapData,
								   1,
								   KTrueFalseOpt,
								   1,
								   KTrueOpt );
    err = iConfigList.Append( config );
    if(err != KErrNone)
    	{
        delete config;
        User::Leave(err);
    	}

    index ++;

 	_LIT( KCreateVariantData, "Create Variant Specific Data ?" );
    config = COptionConfig::NewL( (TInt)ECreateVariantInfo,
    							   aUID,
    							   COptionConfig::EFormatterPlugin,
    							   COptionConfig::ETBool,
    							   KCreateVariantData,
								   1,
								   KTrueFalseOpt,
								   1,
								   KTrueOpt );
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

CElfOptionsV2::CElfOptionsV2()
{

}

/**
dtor frees config parameter list
*/
CElfOptionsV2::~CElfOptionsV2()
{
    iConfigList.ResetAndDestroy();
}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateProcessData() const
	{
	return iConfigList[ (TInt)ECreateProcessData ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateDataSegments() const
	{
	if( CreateSystemWideOnly() )
		return EFalse;

	return iConfigList[ (TInt)ECreateDataSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateCodeSegments() const
	{
	if( CreateSystemWideOnly() )
		return EFalse;

	return iConfigList[ (TInt)ECreateCodeSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateThreadSegments() const
	{
	return iConfigList[ (TInt)ECreateThreadSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateRegisterSegments() const
	{
	if( CreateSystemWideOnly() )
		return EFalse;

	return iConfigList[ (TInt)ECreateRegisterSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateExecSegments() const
	{
	if( CreateSystemWideOnly() )
		return EFalse;

	return iConfigList[ (TInt)ECreateExecSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateProcessSegments() const
	{
    return iConfigList[ (TInt)ECreateProcessSegments ]->ValueAsBool();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateSystemWideOnly() const
	{
	return ( ESystemWideOnly == iConfigList[ (TInt)ECreateSystemCrashInfo ]->Value() );
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateSystemCrashInfo() const
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
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TInt CElfOptionsV2::TraceDataToCapture() const
	{
	return iConfigList[ (TInt)ECreateTraceData ]->Value();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateLockDataSegments() const
	{
	return iConfigList[ (TInt)ECreateLockSegments ]->Value();
	}

/**
Called to obtain the value of the parameter
@return actual value of the parameter
*/
TBool CElfOptionsV2::CreateRomBuildInfo() const
	{
	return iConfigList[ (TInt)ECreateRomInfo ]->Value();
	}

/**
Called to obtain the value of the heap data option
@return Whether or not to dump heap data
*/
TBool CElfOptionsV2::CreateHeapData() const
	{
	LOG_MSG2("CElfOptionsV2::CreateHeapData ret %d", iConfigList[(TInt)ECreateHeapInfo]->Value());
	return iConfigList[(TInt)ECreateHeapInfo]->Value();
	}

/**
Called to obtain the value of the variant specific data option
@return Whether or not to dump variant data
*/
TBool CElfOptionsV2::CreateVariantSpecificData() const
	{
	LOG_MSG2("CElfOptionsV2::CreateVariantSpecificData ret %d", iConfigList[(TInt)ECreateVariantInfo]->Value());
	return iConfigList[(TInt)ECreateVariantInfo]->Value();
	}

/**
Called by CDS to ask for the configuration parameters that the formatter implements.
@return actual number of implemented config parameters
*/
TInt CElfOptionsV2::GetNumberConfigParametersL( )
	{
	//LOG_MSG("->CElfOptionsV2::GetNumberConfigParametersL() : returning %d", iConfigList.Count() );
	return iConfigList.Count();
	}

/**
Called by CDS to ask for configuration parameter.
@param aIndex indicates which parameter to return
@return pointer to COptionConfig object representing the requested config parameter. Caller doesn't take ownership of the object!
@leave KErrBadHandle if index is out of bounds
@see COptionConfig
*/
COptionConfig * CElfOptionsV2::GetConfigParameterL( const TInt aIndex )
	{
	LOG_MSG2("CElfOptionsV2::GetConfigParameterL Getting index = %d", aIndex);
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
TInt CElfOptionsV2::SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue )
	{
	LOG_MSG3("->CElfOptionsV2::SetConfigParameterL()\n  DesC = %S aIndex = %d", &aDescValue, aIndex);

	// Make the changes to our internal structures, and reflect such changes in our behaviour

	if( ( aIndex < 0 ) || ( aIndex > iConfigList.Count() ) )
		{
		LOG_MSG2( "CElfOptionsV2::SetConfigParameterL(): ERROR !* : aIndex=%d is out of bounds\n", aIndex );
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
		case ECreateHeapInfo:
		case ECreateVariantInfo:
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
				LOG_MSG2( "CElfOptionsV2::SetConfigParameterL(): ERROR !* : invalid value %d\n", aValue );
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
			break;
		case ECreateTraceData:
			{
			config->Value(aValue);
			}
			break;
		default:
			break;
		}//switch

	LOG_MSG("CElfOptionsV2::SetConfigParameterL finished");
	return KErrNone;
	}
