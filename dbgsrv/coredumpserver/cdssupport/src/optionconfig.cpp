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
// Implementation of class COptionConfig configuration parameter interface 
//



/**
 @file
 @see COptionConfig
*/

#include <debuglogging.h>

#include <optionconfig.h>


/**
Allocates and constructs a COptionConfig object.
@param aIndex Internal index to the component that owns the object
@param aUID UID of the component that owns the object
@param aSource Type of component that owns the object
@param aType Type of parameter
@param aPrompt Prompt to present to user 
@param aNumOptions Number of options that the parameter can be set to. Only applies if type is ETMultiEntryEnum.
@param aOptions Comma separated list of options. Applies to ETMultiEntryEnum and ETBool.
@param aVal Integer value. Applies to ETInt, ETUInt, ETBool.
@param aStrValue String value. Applies to ETString, ETFileName, ETMultiEntry, ETBool.
*/
EXPORT_C COptionConfig* COptionConfig::NewL( 
										const TUint32          & aIndex, 
										const TUint32          & aUID, 
										const TParameterSource & aSource, 
										const TOptionType      & aType, 
										const TDesC            & aPrompt, 
										const TUint32          & aNumOptions,
										const TDesC            & aOptions,
										const TInt32           & aVal, 
										const TDesC            & aStrValue)
                                        
	{


	const TInt size = 
		  sizeof( TUint32 )	// iType
		+ sizeof( TUint32 )	// iSource
		+ sizeof( TUint32 )	// iIndex
		+ sizeof( TUint32 )	// iInstance
		+ sizeof( TUint32 )	// iUID

		+ sizeof( TUint32 )	// When externalized, we send the name length, so must include this
		+ 2					// When externalized, the << operator writes 2 bytes for the descriptor size
		+ aPrompt.Size()	// iPrompt

		+ sizeof( TUint32 )	// iNumOptions

		+ sizeof( TUint32 )	// When externalized, we send the name length, so must include this
		+ 2					// When externalized, the << operator writes 2 bytes for the descriptor size
		+ aOptions.Size()	// iOptions

		+ sizeof ( TInt32 )	// iValue

		+ sizeof( TUint32 )	// When externalized, we send the name length, so must include this
		+ 2					// When externalized, the << operator writes 2 bytes for the descriptor size
		+ aStrValue.Size()	// iStrValue

		+ sizeof( TUint32 );// iSize itself

	if( size >= MaxSize() )
		{
		LOG_MSG3( "COptionConfig::NewL() : Attempting to allocate %d bytes but maximum is %d\n", 
			size, MaxSize() );

		User::Leave( KErrTooBig );
		}

	COptionConfig * data = new (ELeave) COptionConfig( aType, aSource, aIndex, aUID, aNumOptions, aVal );
	CleanupStack::PushL( data );

	data->ConstructL( aPrompt, aOptions, aStrValue );
	CleanupStack::Pop(data);
	return (data);

	}


/**
Allocates and constructs a COptionConfig object from a descriptor. 
The descriptor contains an externalised version of a COptionConfig object.
This method is typically used to obtain a COptionConfig object from a 
descriptor returned by the core dump server.

@param aStreamData Descriptor with externalised/streamed object
@see InternalizeL
@see ExternalizeL
*/
EXPORT_C COptionConfig* COptionConfig::NewL( const TDesC8 & aStreamData )
	{

	COptionConfig* data = new (ELeave) COptionConfig();

	CleanupStack::PushL( data );
	
	// Open a read stream for the descriptor
	RDesReadStream stream( aStreamData );

	CleanupClosePushL( stream );

	data->InternalizeL( stream );

	CleanupStack::PopAndDestroy( &stream ); // finished with the stream

	CleanupStack::Pop( data );

	return (data);

	}


/**
Destructor. Deletes descriptors.
*/
EXPORT_C COptionConfig::~COptionConfig()
	{


	if( iPrompt )
		{
		delete iPrompt;
		}

	if( iOptions )
		{
		delete iOptions;
		}

	if( iStrValue )
		{
		delete iStrValue;
		}
	}


/**
First phase contructor. Sets the size to 0, descriptors to NULL.
@see COptionConfig::NewL()
*/
COptionConfig::COptionConfig(const TOptionType				& aType,  
									const TParameterSource	& aSource, 
									const TUint32			& aIndex, 
									const TUint32			& aUID,
									const TUint32			& aNumOptions,
									const TInt32			& aValue ) :
	iType			( aType ),
	iSource			( aSource ),
	iIndex			( aIndex ), 
	iUID			( aUID ), 
	iPrompt			( NULL ),
	iNumOptions		( aNumOptions ),
	iOptions		( NULL ),
	iValue			( aValue ),
	iStrValue		( NULL ),
	iSize			( 0 ),
	iInstance		( 0 ) 

	{

	
	/*
	LOG_MSG2( "COptionConfig::COptionConfig() : iSize=%d\n", iSize );
	LOG_MSG4( "  iType=%d, iSource=%d, iIndex=%d", iType, iSource, iIndex );
	LOG_MSG4( "  iUID=0x%X, iNumOpts=%d, iValue=%d\n", iUID, iNumOptions, iValue );
	*/
	
	}



void COptionConfig::ConstructStringL( const TDesC & aSource, HBufC ** aDest )
	{

	TInt sourceLength = aSource.Length();

	if( sourceLength > 0 )
		{

		TInt newMaxLength = Align4( KCDSMaxConfigParamStr );
		if( *aDest )
			{
			delete *aDest;
			}

		*aDest = HBufC::NewL( newMaxLength );
		CleanupStack::PushL( *aDest );

		TPtr destDes = (*aDest)->Des();

		destDes.FillZ( newMaxLength );

		if( sourceLength >= KCDSMaxConfigParamStr )
			{
			sourceLength = KCDSMaxConfigParamStr - 1; // We want to leave the null at the end always
			}

		destDes.Copy( aSource.Ptr(), sourceLength );

		destDes.SetLength( sourceLength );
		CleanupStack::Pop( *aDest );

		}
	else
		{
		*aDest = NULL;
		}
	 }



/**
Second phase constructor initialises the descriptors
*/
void COptionConfig::ConstructL( const TDesC & aPrompt, 
								const TDesC & aOptions,
								const TDesC & aStrValue )
	{

	ConstructStringL( aPrompt, &iPrompt );
	ConstructStringL( aOptions, &iOptions );
	ConstructStringL( aStrValue, &iStrValue );

	}


/**
Initialise this object with the contents of RReadStream aStream.
The descriptor contains an externalised version of an object.
This method is typically used to obtain a COptionConfig object from 
the core dump server.
Any modifications to this method should be synchronised with ExternalizeL(). 
Also note that the methods used from RReadStream (>> or ReadUint32L) 
can behave differently, especially for descriptors.
@param aStream Stream with streamed object
@see ExternalizeL
@see RReadStream
@pre Call ExternaliseL to obtain the stream containing an externalised 
version of this object.
*/
EXPORT_C void COptionConfig::InternalizeL( RReadStream & aStream )
	{


	iType = (TOptionType) aStream.ReadUint32L(); 

	iSource = (TParameterSource) aStream.ReadUint32L(); 

	iIndex = aStream.ReadUint32L(); 

	iInstance = aStream.ReadUint32L(); 

	iUID = aStream.ReadUint32L(); 

	if( NULL != iPrompt )
		{
		delete iPrompt;
		iPrompt = NULL;
		}

	TUint32 promptSize = aStream.ReadUint32L(); 

	if ( promptSize > 0 )
		{
		//iPrompt = HBufC::NewL( aStream, KCDSMaxConfigParamStr ); 
		iPrompt = HBufC::NewL( aStream, promptSize ); 
		}
	else
		{
		iPrompt  = NULL;
		}

	iNumOptions = aStream.ReadUint32L(); 

	if( NULL != iOptions )
		{
		delete iOptions;
		iOptions = NULL;
		}

	TUint32 optionSize = aStream.ReadUint32L(); 

	if ( optionSize > 0 )
		{
		//iOptions = HBufC::NewL( aStream, KCDSMaxConfigParamStr ); 
		iOptions = HBufC::NewL( aStream, optionSize ); 
		}
	else
		{
		iOptions = NULL;
		}


	iValue = aStream.ReadInt32L();

	if( NULL != iStrValue )
		{
		delete iStrValue;
		iStrValue = NULL;
		}

	TUint32 strValueSize = aStream.ReadUint32L(); 

	if ( strValueSize > 0 )
		{
		//iStrValue = HBufC::NewL( aStream, KCDSMaxConfigParamStr ); 
		iStrValue = HBufC::NewL( aStream, strValueSize ); 
		}
	else
		{
		iStrValue = NULL;
		}

	iSize = aStream.ReadUint32L() ;

	}


/**
Make a streamed representation of this object to a RWriteStream.

This method is typically by the core dump server when contructing a list of 
COptionConfig for a client.
Any modifications to this method should be synchronised with InternalizeL(). 
Also note that the methods used from RWriteStream (>> or WriteUint32L) can behave differently,
especially for descriptors.
@param aStream Stream to stream object onto
@param buf Buffer onto the same stream, used to obtain correct size of externalised object
@see InternalizeL
@see RReadStream
@see RWriteStream
@post The stream contains an externalised version of this object.
*/
EXPORT_C void COptionConfig::ExternalizeL( RWriteStream & aStream, CBufFlat* buf )
	{


	// Take the size of the buffer before we add anything to it.
	TUint startBufSize = buf->Size();

	aStream.WriteUint32L( iType ); 

	aStream.WriteUint32L( iSource ); 

	aStream.WriteUint32L( iIndex );

	aStream.WriteUint32L( iInstance );

	aStream.WriteUint32L( iUID );

	TInt promptSize = 0;
	if ( iPrompt != NULL )
		{
		promptSize = iPrompt->Des().Length();
		if( promptSize > 0 )
			{
			aStream.WriteUint32L( promptSize ); 
			aStream << iPrompt->Des();
			}
		}

	if( promptSize == 0 )
		{
		aStream.WriteUint32L( 0 ); 
		}

	aStream.WriteUint32L( iNumOptions );

	TInt optionSize = 0;
	if ( iOptions != NULL )
		{
		optionSize = iOptions->Des().Length();
		if( optionSize > 0 )
			{
			aStream.WriteUint32L( optionSize ); 
			aStream << iOptions->Des();
			}
		}

	if( optionSize == 0 )
		{
		aStream.WriteUint32L( 0 ); 
		}

	aStream.WriteInt32L( iValue );

	TInt valueSize = 0;
	if ( iStrValue != NULL )
		{
		valueSize = iStrValue->Des().Length();
		if( valueSize > 0 )
			{
			aStream.WriteUint32L( valueSize ); 
			aStream << iStrValue->Des();
			}
		}

	if( valueSize == 0 )
		{
		aStream.WriteUint32L( 0 ); 
		}

	// The real exteranlized size is the size of the buffer up to here plus the 
	// 4 bytes for the size itself
	iSize = buf->Size() - startBufSize + 4;
	aStream.WriteUint32L( iSize );

	}



/**
Obtain the type of parameter.
@see TOptionType
*/
EXPORT_C COptionConfig::TOptionType COptionConfig::Type() const 
	 { 
	 return ( iType ); 
	 }

/**
Obtain the source component type of the parameter.
@see TParameterSource
*/
EXPORT_C COptionConfig::TParameterSource COptionConfig::Source() const 
	 { 
	 return ( iSource ); 
	 }

/**
Obtain the internal index to the component that owns the object.
*/
EXPORT_C TUint32 COptionConfig::Index() const 
	 { 
	 return ( iIndex ); 
	 }

/**
Obtain the internal index to the component that owns the object.
*/
EXPORT_C TUint32 COptionConfig::Instance() const 
	 { 
	 return ( iInstance ); 
	 }

/**
Set the internal index to the component that owns the object.
*/
EXPORT_C void COptionConfig::Instance(TInt32 aInstance)
	 { 
	 iInstance = aInstance; 
	 }

/**
Obtain the UID of the component that owns the object.
*/
EXPORT_C TUint32 COptionConfig::Uid( ) const 
	{ 
	return iUID; 
	}


/**
Obtain the prompt to present to the user.
*/
EXPORT_C const TDesC & COptionConfig::Prompt() const 
	{ 
	if( iPrompt )
		{
		return ( *iPrompt ); 
		}
	else
		{
		return KNullDesC;
		}
	}


/**
Obtain the number of options that the parameter can be set to. 
Only applies if type is ETMultiEntryEnum.
*/
EXPORT_C TUint32 COptionConfig::NumOptions( ) const 
	{ 
	return iNumOptions; 
	}


/**
Obtain the comma separated list of options. Applies to ETMultiEntryEnum and ETBool.
*/
EXPORT_C const TDesC & COptionConfig::Options() const 
	{ 
	if( iOptions )
		{
		return ( *iOptions ); 
		}
	else
		{
		return KNullDesC;
		}
	}


/** Obtain the integer value of the parameter. */
EXPORT_C TInt32 COptionConfig::Value() const 
	 { 
	 return ( iValue ); 
	 }


/** Obtain the value of the parameter as a Boolean. */
EXPORT_C TBool COptionConfig::ValueAsBool() const
	{
	if( 0 == iValue )
		{
		return EFalse;
		}
	else
		{
		return ETrue;
		}
	}


/** Obtain the value of the parameter as a descriptor. Does not apply to ETInt or ETUInt. */
EXPORT_C const TDesC & COptionConfig::ValueAsDesc() const 
	{ 

	if( iStrValue )
		{
		return ( *iStrValue ); 
		}
	else
		{
		return KNullDesC;
		}
	 }

/** Set the integer value of the parameter. */
EXPORT_C void COptionConfig::Value( const TInt32 aValue ) 
	{ 
	iValue = aValue; 
	}

/** Set the descriptor value of the parameter. */
EXPORT_C void COptionConfig::ValueL( const TDesC & aValue ) 
	{ 

	TInt valSize = aValue.Size();

	TInt newSize = Size() + valSize ;

	if( iStrValue )
		{
		newSize -= iStrValue->Size();
		}

	if( aValue.Length() > KCDSMaxConfigParamStr )
		{
		LOG_MSG( "if( aValue.Length() > KCDSMaxConfigParamStr ) -> User::Leave( KErrTooBig )\n" );
		User::Leave( KErrTooBig );
		}

	if( newSize >= MaxSize() )
		{
		LOG_MSG( "if( newSize >= MaxSize() ) -> User::Leave( KErrTooBig )\n" );
		User::Leave( KErrTooBig );
		}


	if( valSize == 0 )
		{
		iStrValue = NULL;
		}
	else
		{
		ConstructStringL( aValue, & iStrValue );
		}

	iSize = newSize;

	}



EXPORT_C COptionConfig::COptionConfig()
	{
	}


/**
Gets the size of the object when externalized. The sizeofs used to calculate this 
must match the operators used in ExternalizeL and InternalizeL.
Special attention must be paid to the name. If the object has not been 
externalized yet then this method returns the maximum that it could take.
The name descriptor is compressed when externalized, so it is not its Size().
Furthermore the << operator adds two bytes to the stream when externalizing 
a descriptor.
*/
EXPORT_C TInt COptionConfig::Size() const
	{

	if( iSize != 0 )
		{
		return iSize;
		}

	TUint extDecSize = 0;
	if( iPrompt )
		{
		extDecSize += 2					// When externalized, the << operator writes 2 bytes for the descriptor size
					+ iPrompt->Size();	// iPrompt itself, in bytes. Worst case
		}

	if( iOptions )
		{
		extDecSize += 2					// When externalized, the << operator writes 2 bytes for the descriptor size
					+ iOptions->Size();	// iOptions itself, in bytes. Worst case
		}

	if( iStrValue )
		{
		extDecSize += 2					// When externalized, the << operator writes 2 bytes for the descriptor size
					+ iStrValue->Size();	// iStrValue itself, in bytes. Worst case
		}


	const TInt size = 
		  sizeof( TUint32 )	// iType
		+ sizeof( TUint32 )	// iSource
		+ sizeof( TUint32 )	// iIndex
		+ sizeof( TUint32 )	// iInstance
		+ sizeof( TUint32 )	// iUID
		+ sizeof( TUint32 )	// iPrompt size
		+ extDecSize		// all the descriptors and their 2 bytes due to <<
		+ sizeof( TUint32 )	// iNumOptions
		+ sizeof( TUint32 )	// iOptions size
		+ sizeof( TInt32 )	// iValue 
		+ sizeof( TUint32 )	// iStrValue size
		+ sizeof( TUint32 );	// iSize itself

	return size;
	}


 /**
 Get the maximum size allowed for this object. This is needed as the object is passed  
 across the Client Server interface.
 */
 EXPORT_C TInt COptionConfig::MaxSize()
	{
	
	const TInt maxSize = 1024;
	return maxSize;
	}

