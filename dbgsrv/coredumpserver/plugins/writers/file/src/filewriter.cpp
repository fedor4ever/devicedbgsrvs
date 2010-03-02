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
// @file filewriter.cpp
// Implementation of the file writer plugin.
//

/**
 @file
 @internalTechnology
 @released
*/

#include <e32base.h>
#include <e32debug.h>
#include <optionconfig.h>
#include <bautils.h>
#include "filewriter.h"

/**
1st stage construction
@return pointer to the newly created object, user takes ownership of the object.
*/
CFileWriter* CFileWriter::NewL()
    {
	//LOG_MSG("->CFileWriter::NewL()\n");
	CFileWriter* self = CFileWriter::NewLC();
	CleanupStack::Pop();
	return self;
    }

/**
1st stage construction
@return pointer to the newly created object, user takes ownership of the object.
*/
CFileWriter* CFileWriter::NewLC()
    {
	//LOG_MSG("->CFileWriter::NewLC()\n");
	CFileWriter* self = new(ELeave) CFileWriter();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
    }

/**
ctor defaults plugin description descriptor to zero
*/
CFileWriter::CFileWriter() :
	iDescription(0)
{ }

/**
2nd stage construction.
Connects to the file server, reserves config parameter list,
creates config parameter objects and sets them to default values.
*/
void CFileWriter::ConstructL()
    {
	//LOG_MSG("->CFileWriter::ConstructL()\n");
    TInt err;

	_LIT(KPluginDescription, "Symbian Elf Core Dump Writer");
	iDescription = KPluginDescription().AllocL();
    iFs.Connect();
    iFs.ShareAuto(); //share the session with the crash processing thread

	iConfigList.ReserveL( (TInt)EDataSaveLastParam );

	COptionConfig *config;

	_LIT( KFilePathPrompt, "Please supply Full Core Dump Filepath (eg \"C:\\coredump.core\")" );
	config = COptionConfig::NewL(  (TInt)ECoreFilePath,
									KFileWriterUid,								// UID, unused
									COptionConfig::EWriterPlugin,
									COptionConfig::ETFileName,
									KFilePathPrompt,
                                    1,
									KNullDesC,
									0,
									KNullDesC );

	err = iConfigList.Append( config );
    if(err != KErrNone)
        {
	    LOG_MSG2("CFileWriter::ConstructL - unable to append the param! err:%d\n", err);
        delete config;
        User::Leave(err);
        }
    }

/**
dtor frees config parameter list, plugin description descriptor and closes file server session.
*/
CFileWriter::~CFileWriter()
    {
    iConfigList.ResetAndDestroy();
	delete iDescription;
	iFs.Close();
    }

/**
Returns a description of the plugin.
@param aPluginDescription Output parameter that contains the plugin description.
*/
void CFileWriter::GetDescription( TDes & aPluginDescription )
    {
	aPluginDescription = *iDescription;
    }

/**
Called by CDS to ask for the configuration parameters that the writer implements.
@return actual number of implemented config parameters
*/
TInt CFileWriter::GetNumberConfigParametersL()
	{
	//LOG_MSG2("CFileWriter::GetNumberConfigParametersL()\n", iConfigList.Count() );
	return iConfigList.Count();
	}

/**
Called by CDS to ask for configuration parameter.
@param aIndex indicates which parameter to return
@return pointer to COptionConfig object representing the requested config parameter. Caller doesn't take ownership of the object!
@leave KErrBadHandle if index is out of bounds
@see COptionConfig
*/
COptionConfig * CFileWriter::GetConfigParameterL( const TInt aIndex )
	{
	// return the config identified by aIndex
	if( ( aIndex < 0 ) || ( aIndex >= iConfigList.Count() ) )
		{
		User::Leave( KErrBadHandle );
		}

    return iConfigList[aIndex];
	}

/**
Change a configuration parameter.
@param aIndex Index of parameter to change
@param aValue Unused
@param aDescValue Path and filename to use
@leave KErrBadHandle if index is out of bounds, KErrBadName if file name parameter is not a valid file name or one of the other system wide error codes
*/
void CFileWriter::SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue )
    {
	// Make the changes to our internal structures, and reflect such changes in our behaviour
	if( ( aIndex < 0 ) || ( aIndex > iConfigList.Count() ) )
		{
		LOG_MSG2("CFileWriter::SetConfigParameterL - bad param request %d", aIndex );
		User::Leave( KErrBadHandle );
		}

	COptionConfig & config = *(iConfigList[aIndex]);
	if( aIndex == (TInt)ECoreFilePath )
		{
		if( iFs.IsValidName( aDescValue ) || aDescValue == KNullDesC )
			{
			config.ValueL( aDescValue );
			}
		else
			{
			_LIT( KInvalidFileErrorMsg ,"File Writer Error : Filepath Not Valid");
			User::InfoPrint( KInvalidFileErrorMsg );
			LOG_MSG(" CFileWriter::SetConfigParameterL() : ERROR  from RFs.IsValidName( )" );
			User::Leave( KErrBadName );
			}
		}
	else
		{
		LOG_MSG2("CFileWriter::SetConfigParameterL - invalid aIndex:%d\n", aIndex );
		User::Leave( KErrBadHandle );
		}

	//If the descriptor is not null ensure it is a valid path
	if(aDescValue != KNullDesC)
		{
		TParse parseName;
		TInt err = parseName.Set( aDescValue, NULL, NULL );
		if( KErrNone != err )
			{
			_LIT( KParseFileErrorMsg ,"File Writer Error : Could not Parse Filepath");
			User::InfoPrint( KParseFileErrorMsg );
			LOG_MSG(" CFileWriter::SetConfigParameterL() : ERROR  from TParse.Set( )" );
			User::Leave( err );
			}

		TPtrC drvpath = parseName.DriveAndPath();
		RDir rDir;
		TInt dirErr = rDir.Open( iFs, drvpath, EFileWrite );
		rDir.Close();
		if( KErrNone != dirErr )
			{
			_LIT( KInvalidDirErrorMsg ,"File Writer Error : Directory Does Not Exist");
			User::InfoPrint( KInvalidDirErrorMsg );
			LOG_MSG2(" CFileWriter::SetConfigParameterL() : ERROR  from RDir.Open() returned %d", dirErr );
			User::Leave( KErrBadName );
			}
		}

	//This is initially the filename we intend to, and will try to use
	iFileName = aDescValue;
    }

/**
Called by formatter to start the creation of a dump file.
Can be called several times to create several files.
Note that if the file specified either in the parameter here
or the configuration already exists, the file writer will not 
overwrite it, and instead just append a number in brackets to 
the stem of the filename. e.g. e:\file(1).elf if e:\file.elf
already exists. To determine the actual name used, call GetMediaName

@see GetMediaName()
@param aFileName Formatters can supply a file name to use
@return one of the system error codes
@leave err one of the system wide error codes
*/
void CFileWriter::OpenL( const TDesC& aFileName )
    {
    User::LeaveIfError(Open(aFileName));
    }

/**
Called by formatter to start the creation of a dump file.
Can be called several times to create several files.
Note that if the file specified either in the parameter here
or the configuration already exists, the file writer will not 
overwrite it, and instead just append a number in brackets to 
the stem of the filename. e.g. e:\file(1).elf if e:\file.elf
already exists. To determine the actual name used, call GetMediaName

@see GetMediaName()
@param aFileName Formatters can supply a file name to use
@return one of the system error codes
*/
TInt CFileWriter::Open( const TDesC& aFileName )
	{
    if(aFileName.Length() > 0)
        {
		iFileName = aFileName;
        }
    else
        {
		COptionConfig & config = *(iConfigList[ (TInt)ECoreFilePath ]);
		iFileName = config.ValueAsDesc();
	    }

	LOG_MSG2("->CFileWriter::Open([%S])\n", &TPtr8((TUint8*)iFileName.Ptr(), 2*iFileName.Length(),	2*iFileName.Length() ));

	//If the file exists already, we want to preserve it, and append a (%d) to the name of the new file
	const TUint startAppend = 1;
	TUint currentAppend = startAppend;
	TBuf<KMaxFileName> formatString;

	_LIT(KFormatString, "(%d)");
	_LIT(KFileBackSlash, "\\");

	for(;;)
		{
		//If file doesn't exist, we can use this name
		if(!BaflUtils::FileExists(iFs, iFileName))
			break;

		//otherwise, we sort out the filename
		if(currentAppend == startAppend)
			{
			//first time so it needs the (%d) at the end of the name
			TParse p;
			p.Set(iFileName, NULL, NULL);

			formatString.Append(p.Drive());
			formatString.Append(KFileBackSlash);
			formatString.Append(p.Name());
			formatString.Append(KFormatString);
			formatString.Append(p.Ext());
			}

		iFileName.Format(formatString, currentAppend);
		++currentAppend;
		}

	return iFile.Create(iFs, iFileName, EFileWrite);
    }

/**
This returns a reference to a descriptor containing the name
of the file that the filewriter intends to use at any point 
in time. Note that the filewriter will not determine what file
to use until the last moment and so this is not gauranteed to 
be the actual filename used until one of the Open() methods are
called

@see Open(const TDesC& aFileName)
@see OpenL(const TDesC& aFileName)
@return TDesC& reference to filename
*/
const TDesC& CFileWriter::GetMediaName()
	{
	return iFileName;	
	}

/**
Called by formatter at the end of the creation of a dump.
Open() and Close() must be paired calls
@pre Must have called Open() or OpenL()
@leave err one of the system wide error codes
*/
void CFileWriter::CloseL()
    {
	User::LeaveIfError(Close());
    }

/**
Called by formatter at the end of the creation of a dump.
Open() and Close() must be paired calls
@return one of the system wide error codes
*/
TInt CFileWriter::Close()
    {
    TInt err = iFile.Flush();
    //LOG_MSG2("->CFileWriter::Close() : returns:%d\n", err);
    iFile.Close();
	return err;
    }

/**
Save binary aData

@param aData Binary data to save
@leave err one of the system wide error codes
*/
void CFileWriter::WriteL( const TDesC8& aData )
    {
    User::LeaveIfError(Write(aData));
    }

/**
Save binary aData

@param aData Binary data to save
@return one of the system wide error codes
*/
TInt CFileWriter::Write( const TDesC8& aData )
    {
    TInt err = iFile.Write(aData);
    if( KErrNone != err )
		{
		LOG_MSG2("CFileWriter::Write() : iFile.Write(aData) returned :%d\n", err);
		}
    return err;
    }

/**
Save aSize bytes of binary data.

@param aData Binary data to save
@param aSize Number of bytes to save
@leave err one of the system wide error codes
*/
void CFileWriter::WriteL( TAny* aData, TUint aSize )
    {
    User::LeaveIfError(Write(aData, aSize));
    }

/**
Save aSize bytes of binary data

@param aData Binary data to save
@param aSize Number of bytes to save
@return one of the system wide error codes
*/
TInt CFileWriter::Write( TAny* aData, TUint aSize )
    {
    //LOG_MSG2("->CFileWriter::Writer(size=%d)\n", aSize);
    if(aData == NULL)
		{
		LOG_MSG2("CFileWriter::Writer(size=%d), aData == NULL\n", aSize);
        return KErrBadHandle;
		}

    TPtrC8 data(static_cast<TUint8*>(aData), aSize);
    return( Write(data) );
    }

/**
Save all of aData to file. Leaves if error.

@param aPos Input : Position to write to, in bytes.
@param aData Input : Data to write to file
@leave err one of the system wide error codes
@see RFile
*/
void CFileWriter::WriteL( TInt aPos, const TDesC8& aData )
    {
    User::LeaveIfError(Write(aPos, aData));
    }

/**
Save all of aData to file.
Note that if a position beyond the end of the file is specified,
then the write operation begins at the end of the file.
Thus aPos must be within the file written so far. For example
writing at position 10 when that position has no data yet will
result in the data being written at the end of the current file.
This is a limitation of RFile.

@param aPos Input : Position to write to, in bytes.
@param aData Input : Data to write to file
@return Returns error from RFile::Write()
@see RFile
*/
TInt CFileWriter::Write( TInt aPos, const TDesC8& aData )
    {
//	LOG_MSG2("  iFile.Write(aPos=%d, aData)\n", aPos );
    TInt err = iFile.Write(aPos, aData);
    if( KErrNone != err )
		{
 		LOG_MSG3("CFileWriter::Write() : iFile.Write(aPos=%d, aData) returned :%d\n", aPos, err );
		}

    return err;
    }

/**
Save aSize bytes of aData at file position aPos.
Note that if a position beyond the end of the file is specified,
then the write operation begins at the end of the file.
Thus aPos must be within the file written so far. For example
writing at position 10 when that position has no data yet will
result in the data being written at the end of the current file.
This is a limitation of RFile.

@param aPos Input : Position to write to, in bytes.
@param aData Input : Data to write to file
@param aSize Input : Number of bytes to write
@leave err one of the system wide error codes
@see RFile
*/
void CFileWriter::WriteL( TInt aPos, TAny* aData, TUint aSize )
    {
    User::LeaveIfError(Write(aPos, aData, aSize));
    }

/**
Save aSize bytes of aData at file position aPos.
Note that if a position beyond the end of the file is specified,
then the write operation begins at the end of the file.
Thus aPos must be within the file written so far. For example
writing at position 10 when that position has no data yet will
result in the data being written at the end of the current file.
This is a limitation of RFile.

@param aPos Input : Position to write to, in bytes.
@param aData Input : Data to write to file
@param aSize Input : Number of bytes to write
@return one of the system wide error codes
@see RFile
*/
TInt CFileWriter::Write( TInt aPos, TAny* aData, TUint aSize )
    {
    if(aData == NULL)
        {
		LOG_MSG3("CFileWriter::Writer(pos=%d, size=%d), aData == NULL\n", aPos, aSize);
        return KErrNoMemory;
        }

    TPtrC8 data(static_cast<TUint8*>(aData), aSize);
    return( Write(aPos, data) );
    }
