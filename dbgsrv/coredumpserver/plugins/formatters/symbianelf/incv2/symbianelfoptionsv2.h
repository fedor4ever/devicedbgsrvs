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

#ifndef __ELFOPTIONSV2_H__
#define __ELFOPTIONSV2_H__

#include <e32def.h>
#include <optionconfig.h>

/**
@internalTechnology
@released

It is used by symbian ELF formatter to manage its configuration parameters.
*/
class CElfOptionsV2 : public CBase
{

public:
	static CElfOptionsV2* NewL( TUint32 aUID = 0 );
	static CElfOptionsV2* NewLC( TUint32 aUID = 0 );
	~CElfOptionsV2();

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
	TBool CreateHeapData() const;
	TBool CreateVariantSpecificData() const;

	TInt GetNumberConfigParametersL( );
	COptionConfig * GetConfigParameterL( const TInt aIndex );
	TInt SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue );

private:
	CElfOptionsV2();
	void ConstructL( TUint32 aUID = 0 );

private:

    /** Available values of symbian ELF parameter index */
	enum TElfOptions
		{
		ECreateSystemCrashInfo,		/**< Dump system wide info */
		ECreateProcessData,			/**< Dump whole crashed process info */
		ECreateDataSegments,		/**< Dump data segments */
		ECreateCodeSegments,        /**< Dump code segments */
		ECreateThreadSegments,		/**< Dump thread info segments */
		ECreateRegisterSegments,	/**< Dump register data segments */
		ECreateExecSegments,		/**< Dump executable info segments */
		ECreateProcessSegments,		/**< Dump process info segments */
		ECreateTraceData,			/**< Dump trace data */
		ECreateLockSegments,		/**< Dump lock data */
		ECreateRomInfo,				/**< Dump ROM build info */
		ECreateHeapInfo,			/**< Dump heap info */
		ECreateVariantInfo,			/**< Dump variant specific info */
		EElfOptionsLast				/** Guard */
		};

    /** Symbian ELF formatter configuration parameter list */
	RConfigParameterList	 iConfigList;

    /** Available values of system crash parameter */
	enum TSystemCrashOption
		{
		ESystemWideOnly = 0,			/**< Generate crash data for system, not for crashed process */
		ECrashedProcessOnly = 1,		/**< Generate crash data for crashed process only */
		ESystemWideAndCrashedProc = 2	/**< Generate crash data for crashed process and system wide */
		};
};

#endif // __ELFOPTIONSV2_H__
