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
// streamelement.h
// Implemetation of class CStreamElementBase
//



/**
 @file
 @see CStreamElementBase
*/

#include <streamelement.h>


EXPORT_C HBufC8* CStreamElementBase::MarshalDataL()
	{
		const TInt KExpandSize = 128; // "Granularity" of dynamic buffer	

		/*
		Create a dynamic flat buffer to hold this object's member data	
		Can only use CBufFlat due to Store supporting CBufFlat and CBufSeg
		See Symbian OS guide » System libraries » Using Store » Streaming » Templated stream operators
		*/
		CBufFlat* buf = CBufFlat::NewL( KExpandSize );

		CleanupStack::PushL( buf );

		RBufWriteStream stream( *buf ); // Stream over the buffer

		CleanupClosePushL( stream );

		ExternalizeL( stream, buf );

		CleanupStack::PopAndDestroy( &stream );
		
		// Create a heap descriptor from the buffer
		HBufC8 * des = HBufC8::NewL( buf->Size() );
		TPtr8 ptr( des->Des() );

		buf->Read( 0, ptr, buf->Size() );
		
		CleanupStack::PopAndDestroy( buf ); // Finished with the buffer
		return ( des );
	}
