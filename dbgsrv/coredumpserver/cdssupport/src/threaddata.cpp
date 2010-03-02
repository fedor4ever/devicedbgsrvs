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
// Implemetation of CThreadInfo class 
//



/**
 @file
 @see CThreadInfo
*/


#include <rm_debug_api.h>
#include <debuglogging.h> 
#include <threaddata.h>

/**
Allocates and constructs a CThreadInfo object.
@param aId Kernel thread id
@param aName Kernel thread name
@param aProcessId Kernel id of owning process
@param aPriority Kernel thread priority TThreadPriority
@param aSvcStackPtr Thread supervisor stack pointer
@param aSvcStackAddr Thread supervisor mode stack base address
@param aSvcStackSize Thread supervisor mode stack size in bytes
@param aUsrStackAddr Thread user mode stack base address
@param aUsrStackSize Thread user mode stack base size in bytes

@see CThreadInfo::CThreadInfo
*/
EXPORT_C CThreadInfo* CThreadInfo::NewL( 
										const TUint64	& aId,
										const TDesC		& aName, 
										const TUint64	& aProcessId,
										const TUint		& aPriority,
										const TLinAddr	& aSvcStackPtr,
										const TLinAddr	& aSvcStackAddr,
										const TUint		& aSvcStackSize,
										const TLinAddr	& aUsrStackAddr,
										const TUint		& aUsrStackSize )
	{

	// Add up the potential maximum size of the externalized object
	const TUint size = 
		  sizeof( TUint32 )		// iId Low
		+ sizeof( TUint32 )		// iId High

		+ sizeof( TUint32 )		// When externalized, we send the name length, so must include this
		+ 2						// When externalized, the << operator writes 2 bytes for the descriptor size
		+ aName.Size()			// iName Size, in bytes.

		+ sizeof( TUint32 )		// iProcessId Low
		+ sizeof( TUint32 )		// iProcessId High
		+ sizeof( TUint32 )		// iPriority
		+ sizeof( TUint32 )		// iSvcStackPtr
		+ sizeof( TUint32 )		// iSvcStackAddr
		+ sizeof( TUint32 )		// iSvcStackSize
		+ sizeof( TUint32 )		// iUsrStackAddr
		+ sizeof( TUint32 )		// iUsrStackSize
		+ sizeof( TUint32 )		// iObserved

		+ sizeof( TUint32 );	// iSize itself

	if( size >= MaxSize() )
		{
		LOG_MSG3( "CThreadInfo::NewL() : Descriptorized object = 0x%X bytes would exceed the maximum of 0x%X\n", 
			size, MaxSize() );

		User::Leave( KErrTooBig );
		}

	CThreadInfo * data = new (ELeave) CThreadInfo(	aId, 
													aProcessId, 
													aPriority, 
													aSvcStackPtr,
													aSvcStackAddr,
													aSvcStackSize,
													aUsrStackAddr,
													aUsrStackSize );
	CleanupStack::PushL( data );
	
	data->ConstructL( aName );

	CleanupStack::Pop(data);

	return (data);

	}


/**
Allocates and constructs a CThreadInfo object from a descriptor. 
The descriptor contains an externalised version of a CThreadInfo object.
This method is typically used to obtain a CThreadInfo object from a 
descriptor returned by the core dump server.

@param aStreamData Descriptor with externalised/streamed object
@see InternalizeL
@see ExternalizeL
*/
EXPORT_C CThreadInfo* CThreadInfo::NewL( const TDesC8 & aStreamData )
	{

	CThreadInfo* self = new (ELeave) CThreadInfo();
	
	CleanupStack::PushL( self );
	
	// Open a read stream for the descriptor
	RDesReadStream stream( aStreamData );

	CleanupClosePushL( stream );

	self->InternalizeL( stream );

	CleanupStack::PopAndDestroy( &stream ); // finished with the stream

	CleanupStack::Pop( self );

	return ( self );

	}


/**
Destructor. Deletes name if allocated.
*/
EXPORT_C CThreadInfo::~CThreadInfo()
	{

	if( NULL != iName )
		{
		delete iName;
		}
	}

/**
Set the name of the thread by deleting, allocating and then copying the parameter.
@param aName Name of the thread to set to 
@see ConstructL()
*/
EXPORT_C void CThreadInfo::NameL( const TDesC & aName )
	{

	if( aName.Length() > 0 )
		{
		TUint toCopy = aName.Length();
		iName = HBufC::NewL( toCopy );
		TPtr nameDes = iName->Des();

		nameDes.Copy( aName.Ptr(), toCopy );
		nameDes.SetLength( toCopy );
		}
	else
		{
		iName = NULL;
		}
	}


/**
First phase contructor. Sets the size to 0, name to NULL.
@see CThreadInfo::NewL()
*/
CThreadInfo::CThreadInfo(	const TUint64	& aId,
							const TUint64	& aProcessId,
							const TUint		& aPriority,
							const TLinAddr	& aSvcStackPtr,
							const TLinAddr	& aSvcStackAddr,
							const TUint		& aSvcStackSize,
							const TLinAddr	& aUsrStackAddr,
							const TUint		& aUsrStackSize ) :
	iId			 ( aId ),
	iName		 ( NULL ),
	iProcessId	 ( aProcessId ),
	iPriority	 ( aPriority ), 
	iSvcStackPtr ( aSvcStackPtr ),
	iSvcStackAddr( aSvcStackAddr ),
	iSvcStackSize( aSvcStackSize ),
	iUsrStackAddr( aUsrStackAddr ),
	iUsrStackSize( aUsrStackSize ),
	iObserved    ( EFalse ),
	iSize        ( 0 ), 
	iLastCpuId	 ( -1 )
	{
	
	}


/**
Second phase constructor initialises the name of the thread.
@param aName Thread name
@see NameL()
*/
void CThreadInfo::ConstructL(  const TDesC & aName )
	{
	NameL( aName );
	}



/**
Initialise this object with the contents of RReadStream aStream.
The descriptor contains an externalised version of an object.
This method is typically used to obtain a CThreadInfo object from 
the core dump server.
Any modifications to this method should be synchronised with ExternalizeL().
Also note that the methods used from RReadStream (>> or ReadUint32L) 
can behave differently, especially for descriptors.
@param aStream Stream with streamed object
@see ExternalizeL
@see RReadStream
@pre Call Externalise to obtain the stream containing an externalised 
version of this object.
*/
EXPORT_C void CThreadInfo::InternalizeL( RReadStream & aStream )
	{

	TUint32 idLow = aStream.ReadUint32L(); 

	TUint32 idHigh = aStream.ReadUint32L(); 
	iId = MAKE_TUINT64( idHigh, idLow );

	if( NULL != iName )
		{
		//LOG_MSG( " iName != NULL\n" );
		delete iName;
		iName = NULL;
		}	

	TUint32 nameLength = aStream.ReadUint32L(); 

	if ( nameLength > 0 )
		{
		iName = HBufC::NewL( aStream, nameLength ); 
		}
	else
		{
		iName  = NULL;
		}

	TUint32 pidLow = aStream.ReadUint32L(); 
	TUint32 pidHigh = aStream.ReadUint32L(); 
	iProcessId = MAKE_TUINT64( pidHigh, pidLow );

	iPriority     = aStream.ReadUint32L(); 
	iSvcStackPtr  = aStream.ReadUint32L() ;
	iSvcStackAddr = aStream.ReadUint32L() ;
	iSvcStackSize = aStream.ReadUint32L() ;
	iUsrStackAddr = aStream.ReadUint32L() ;
	iUsrStackSize = aStream.ReadUint32L() ;
	iObserved     = static_cast<TBool>(aStream.ReadUint32L());
	iLastCpuId = aStream.ReadUint32L();
	iHeapBase = aStream.ReadUint32L();
	iHeapSize = aStream.ReadUint32L();
	iSize = aStream.ReadUint32L() ;
	}





/**
Make a streamed representation of this object to RWriteStream aStream.

This method is typically by the core dump server when contructing a list of 
CThreadInfo for a client.
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
EXPORT_C void CThreadInfo::ExternalizeL( RWriteStream & aStream, CBufFlat* buf )
	{

	const TUint32 idLow = I64LOW( iId ); 
	const TUint32 idHigh = I64HIGH( iId ); 

	// Take the size of the buffer before we add anything to it.
	TUint startBufSize = buf->Size();

	aStream.WriteUint32L( idLow ); 
	aStream.WriteUint32L( idHigh ); 

	TUint nameLength = 0;

	if ( NULL != iName )
		{
		nameLength = iName->Des().Length();
		if( nameLength > 0 )
			{
			// Write the number of character elements in the name. 
			aStream.WriteUint32L( nameLength ); 
			aStream << iName->Des();
			}
		}

	if( nameLength == 0 )
		{
		aStream.WriteUint32L( 0 ); 
		}

	const TUint32 pIdLow = I64LOW( iProcessId ); 
	const TUint32 pIdHigh = I64HIGH( iProcessId ); 
	aStream.WriteUint32L( pIdLow ); 
	aStream.WriteUint32L( pIdHigh );
	
	aStream.WriteUint32L( iPriority     ); 

	aStream.WriteUint32L( iSvcStackPtr  ) ;
	aStream.WriteUint32L( iSvcStackAddr ) ;
	aStream.WriteUint32L( iSvcStackSize ) ;
	aStream.WriteUint32L( iUsrStackAddr );
	aStream.WriteUint32L( iUsrStackSize );
	aStream.WriteUint32L( static_cast<TUint32>(iObserved) );
	aStream.WriteInt32L(iLastCpuId);
	aStream.WriteUint32L( iHeapBase );
	aStream.WriteUint32L( iHeapSize );

	// The real exteranlized size is the size of the buffer up to here plus the 
	// 4 bytes for the size itself
	iSize = buf->Size() - startBufSize + 4;
	aStream.WriteUint32L( iSize );

	}

/**
Obtain the kernel thread id.
*/
EXPORT_C const TUint64 & CThreadInfo::Id() const 
	 { 
	 return ( iId ); 
	 }

/**
Obtain the kernel thread name.
*/
EXPORT_C const TDesC & CThreadInfo::Name() const 
	{ 
	return ( *iName ); 
	}

/**
Obtain the kernel owning process id.
*/
EXPORT_C const TUint64 & CThreadInfo::ProcessId() const 
	 { 
	 return ( iProcessId );
	 }

/**
Obtain the kernel priority.
*/
EXPORT_C TUint CThreadInfo::Priority() const 
	 { 
	 return ( iPriority ); 
	 }

/**
Obtain the kernel supervisor mode stack pointer.
*/
EXPORT_C TUint CThreadInfo::SvcStackPtr() const 
	 { 
	 return ( iSvcStackPtr ); 
	 }

/**
Obtain the kernel supervisor mode stack base address.
*/
EXPORT_C TUint CThreadInfo::SvcStackAddr() const 
	 { 
	 return ( iSvcStackAddr ); 
	 }

/**
Obtain the kernel supervisor mode stack size in bytes.
*/
EXPORT_C TUint CThreadInfo::SvcStackSize() const 
	 { 
	 return ( iSvcStackSize ); 
	 }

/**
Obtain the kernel user mode stack base address.
*/
EXPORT_C TUint CThreadInfo::UsrStackAddr() const 
	 { 
	 return ( iUsrStackAddr ); 
	 }

/**
Obtain the kernel user mode stack size in bytes.
*/
EXPORT_C TUint CThreadInfo::UsrStackSize() const 
	 { 
	 return ( iUsrStackSize ); 
	 }


/**
Returns ETrue if the thread is being explicitly observed for crashes by the Core Dump Server.
If only the owning process is being observed, this method returns EFalse.
*/
EXPORT_C TBool CThreadInfo::Observed() const 
	 { 
	 return ( iObserved ); 
	 }

/**
Set whether this thread is being observed for crashes by the Core Dump Server.
*/
EXPORT_C void CThreadInfo::Observed(TBool aFlag) 
	 { 
	 iObserved = aFlag; 
	 }

CThreadInfo::CThreadInfo()
	{
	}


/**
Get the maximum size allowed for this object. This is needed as the object is passed  
across the Client Server interface.
*/
EXPORT_C TInt CThreadInfo::MaxSize()
	{
	
	const TInt maxSize = 512;
	return maxSize;
	}


/**
Gets the size of the object when externalized. The sizeofs used to calculate this 
must match the operators used in ExternalizeL and InternalizeL.
Special attention must be paid to the name. If the object has not been 
externalized yet then this method returns the maximum that it could take.
The name descriptor is compressed when externalized, so it is not its Size().
Furthermore the << operator adds two bytes to the stream when externalizing 
a descriptor
*/
EXPORT_C TInt CThreadInfo::Size() const
	{

	if( iSize != 0 )
		{
		return iSize;
		}

	// This is the maximum size of the object when externalized.
	// It is a maximum since externalizing 16 bit descriptors compresses the data,
	// and thus the real externalized size is only known at the end of the Externalize() call.
	// The difference is the iName->Size()


	TUint extNameSize = 0;
	if( iName )
		{
		extNameSize = 2					// When externalized, the << operator writes 2 bytes for the descriptor size
					+ iName->Size();	// iName itself, in bytes. Worst case
		}

	const TUint size = 
		  sizeof( TUint32 )		// iId Low
		+ sizeof( TUint32 )		// iId High
		+ sizeof( TUint32 )		// When externalized, we send the name length, so must include this
		+ extNameSize
		+ sizeof( TUint32 )		// iProcessId Low
		+ sizeof( TUint32 )		// iProcessId High
		+ sizeof( TUint32 )		// iPriority
		+ sizeof( TUint32 )		// iSvcStackPtr
		+ sizeof( TUint32 )		// iSvcStackAddr
		+ sizeof( TUint32 )		// iSvcStackSize
		+ sizeof( TUint32 )		// iUsrStackAddr
		+ sizeof( TUint32 )  	// iUsrStackSize
		+ sizeof( TUint32 )  	// iObserved
		+ sizeof( TInt32 )  	// iLastCpuId
		+ sizeof( TUint32 )  	// iHeapBase
		+ sizeof( TUint32 )  	// iHeapSize
		+ sizeof( TUint32 );	// iSize When externalized, we send the real externalized size of the buffer
	
	return size;
	}


EXPORT_C void  CThreadInfo::SetLastCpuId(TInt32 aLastCpuId)
	{
	iLastCpuId = aLastCpuId;
	}

EXPORT_C TInt32  CThreadInfo::LastCpuId() const
	{
	return iLastCpuId;
	}

EXPORT_C void  CThreadInfo::SetHeapBase(TUint32 aHeapBase)
	{
	iHeapBase = aHeapBase;
	}

EXPORT_C TUint32  CThreadInfo::HeapBase() const
	{
	return iHeapBase;
	}

EXPORT_C void  CThreadInfo::SetHeapSize(TUint32 aHeapSize)
	{
	iHeapSize = aHeapSize;
	}

EXPORT_C TUint32  CThreadInfo::HeapSize() const
	{
	return iHeapSize;
	}

