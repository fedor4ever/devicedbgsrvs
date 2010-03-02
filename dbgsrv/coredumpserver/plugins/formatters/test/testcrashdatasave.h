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

/**
 @file
 @internalTechnology
 @released
*/

#ifndef TEST_DATA_SAVE_H
#define TEST_DATA_SAVE_H
#include <e32std.h>
#include <crashdatasave.h>

class CTestDataSave : public CCrashDataSave
{
    public:
        void OpenL( const TDesC& aParam ){ Open( aParam ); };
        TInt Open( const TDesC& aParam );
        void CloseL( ){ Close(); };
        TInt Close( );

		// Save aData - binary
		void WriteL( const TDesC8& aData ){ Write( aData ); };
		TInt Write( const TDesC8& aData );

		// Save aData - raw
		void WriteL( TAny* aData, TUint aSize ) { Write( aData, aSize ); };
		TInt Write( TAny* aData, TUint aSize );

		// Save aData at the specific position - binary
		// need implementation
		void WriteL( TInt aPos, const TDesC8& aData ){ Write( aPos, aData); };
		TInt Write( TInt aPos, const TDesC8& aData );

		// Save aData at the specific position - raw
		void WriteL( TInt aPos, TAny* aData, TUint aSize ){ Write( aPos, aData, aSize ); };
		TInt Write( TInt aPos, TAny* aData, TUint aSize );

        void GetDescription( TDes & aPluginDescription )
        	{
			(void)aPluginDescription;
        	};

        TInt GetNumberConfigParametersL( ) {return 0;};
        COptionConfig * GetConfigParameterL( const TInt aIndex )
        	{
			(void)aIndex;
			return NULL;
			};
	    void SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue )
			{
			(void)aIndex;
			(void)aValue;
			(void)aDescValue;
			};

		 const TDesC& GetMediaName() { return KNullDesC; };

    public:
        static CTestDataSave* NewL();
        virtual ~CTestDataSave();

        RBuf8 iData;

		void Print() const;

};
#endif
