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
// Class holds all user configuration data
//



/**
 @file
 @internalTechnology
 @released
*/

#ifndef __ELFOPTIONS_H__
#define __ELFOPTIONS_H__

#include <e32def.h>
#include <optionconfig.h>

/**
@internalTechnology
@released

It is used by symbian ELF formatter to manage its configuration parameters.
*/
class CElfOptions : public CBase
{

public:
	static CElfOptions* NewL( TUint32 aUID = 0 );
	static CElfOptions* NewLC( TUint32 aUID = 0 );
	~CElfOptions();

	TBool CreateProcessData() const;

	TBool CreateDataSegments() const;
	TBool CreateCodeSegments() const;
	TBool CreateThreadSegments() const;
	TBool CreateRegisterSegments() const;
	TBool CreateExecSegments() const;
	TBool CreateProcessSegments() const;
	TBool CreateSystemCrashInfo() const;
	TBool CreateSystemWideOnly() const;
	TInt TraceDataToCapture() const;
	TBool CreateLockDataSegments() const;
	TBool CreateRomBuildInfo() const;
	
	
	TInt GetNumberConfigParametersL( );
	COptionConfig * GetConfigParameterL( const TInt aIndex );
	TInt SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue );
	
private:
	CElfOptions();
	void ConstructL( TUint32 aUID = 0 );
	
private:

    /** Available values of symbian ELF parameter index */
	enum TElfOptions
		{
        /** dump system wide info */
		ECreateSystemCrashInfo,
        /** dump whole crashed process info */
		ECreateProcessData,
        /** dump data segments */
		ECreateDataSegments,
        /** dump code segments */
		ECreateCodeSegments,
        /** dump thread info segments */
		ECreateThreadSegments,
        /** dump register data segments */
		ECreateRegisterSegments,
        /** dump executable info segments */
		ECreateExecSegments,
        /** dump process info segments */
		ECreateProcessSegments,
		/** dump trace data */
		ECreateTraceData,
		/** dump lock data */
		ECreateLockSegments,
		/** dump ROM build info */
		ECreateRomInfo,		
        /** guard */
		EElfOptionsLast
		};	

    /** Symbian ELF formatter configuration parameter list */
	RConfigParameterList	 iConfigList;

    /** Available values of system crash parameter */
	enum TSystemCrashOption
		{
		/**
		Generate crash data for system, not for crashed process
		*/
		ESystemWideOnly = 0,			
		/**
		Generate crash data for crashed process only
		*/
		ECrashedProcessOnly = 1,
		/**
		Generate crash data for crashed process and system wide
		*/
		ESystemWideAndCrashedProc = 2
		};
};


#endif // __ELFOPTIONS_H__
