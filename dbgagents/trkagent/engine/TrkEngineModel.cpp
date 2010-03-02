/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/


#include <f32file.h>

#include "TrkEngineModel.h"


//
//
// CTrkEngineModel implementation
//
//

//
// CTrkEngine::NewL
//
CTrkEngineModel* CTrkEngineModel::NewL()
{
	CTrkEngineModel* self = new(ELeave) CTrkEngineModel();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;

}

//
// CTrkEngineModel::NewLC
//
CTrkEngineModel* CTrkEngineModel::NewLC()
{
	CTrkEngineModel* self = new(ELeave) CTrkEngineModel();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}
//
// CTrkEngineModel::ConstructL()
//
// Second level constructor
//
void CTrkEngineModel::ConstructL()
{ 
 
    if (!iLoaded)
    {
        TRAP(iFileReadErr, LoadDataL());
    
        if (iFileReadErr != KErrNone)
        {
            //something went wrong in reading the ini file, just set the default values
            iTrkConnData.iCommType = ESerialComm;
            iTrkConnData.iConnType = ETrkSerial;
            iTrkConnData.iPDD = KDefaultPDD;
            iTrkConnData.iLDD = KDefaultLDD;
            iTrkConnData.iCSY = KDefaultCSY;
            iTrkConnData.iPortNumber = KDefaultPortNumber;
            iTrkConnData.iRate = KDefaultRate;
            iTrkConnData.iPlugPlay = ETrue;
            
            iTrkConnData.iDefault = ETrue;
        }
    }
}

//
// CTrkEngineModel constructor
//
CTrkEngineModel::CTrkEngineModel()
:iLineNumber(0),
iLoaded(EFalse),
iFileReadErr(0)
{
	iTrkConnData.iPortNumber = KDefaultUsbPortNumber;
	iTrkConnData.iRate = KDefaultRate;
}


//
// CTrkEngineModel destructor
//
CTrkEngineModel::~CTrkEngineModel()
{

}

static TUint UintFromDecimalStringL(const TDesC8& aStr)
{
	TLex8 lexer(aStr);
	TUint n;
	if (lexer.Val(n, EDecimal) != KErrNone || !lexer.Eos())
	{
		User::Leave(KErrCorrupt);
	}
	return n;
}

//
// CTrkEngineModel LoadDataL
//
void CTrkEngineModel::LoadDataL()
{
	RFs fs;
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);

	TFindFile findFile(fs);
	
	User::LeaveIfError(findFile.FindByDir(KInitFileName, KInitFilePath));

	iInitFilePath = findFile.File();
	
	RFile file;
	User::LeaveIfError(file.Open(fs, iInitFilePath, EFileStreamText|EFileRead));

	iInitFilePath.ZeroTerminate();

	CleanupClosePushL(file);

	TInt size;
	User::LeaveIfError(file.Size(size));
	
	HBufC8 *content = HBufC8::NewLC(size);
	TPtr8 p(content->Des());
	
	User::LeaveIfError(file.Read(p));
	
	TLex8 lexer(*content);

	iLineNumber = 1;
	// skip any white space or blank lines
	TChar ch = lexer.Peek();
	while (!lexer.Eos() && (ch.IsSpace() || (ch == '\r') || (ch == '\n')))
	{
		lexer.Inc();
		ch = lexer.Peek();
	}
	
	if (lexer.Eos())
	{
		User::Leave(KErrCorrupt);
	}
	
	// mark the token
	lexer.Mark();
	
	lexer.Inc(KSerialComm().Length());

	// the first thing in the init file has to be the comm type ([SERIALCOMM] or [SOCKETCOMM]) or [BTSOCKCOMM]
	if (lexer.TokenLength() != KSerialComm().Length())
	{
		User::Leave(KErrCorrupt);
	}

	TPtrC8 commType = lexer.MarkedToken();

	if (!commType.Compare(KSerialComm()))
	{
		iTrkConnData.iCommType = ESerialComm; //set to serial as the default comm type
	}
	else if (!commType.Compare(KDbgTrcComm()))
	{
		iTrkConnData.iCommType = EDbgTrcComm;
		iTrkConnData.iConnType = ETrkUsbDbgTrc;	
	}
	else if (!commType.Compare(KBtSocketComm()))
	{
		iTrkConnData.iCommType = EBtSocket;
		iTrkConnData.iConnType = ETrkBt;	
	}
	else if (!commType.Compare(KDCCComm()))
	{
		iTrkConnData.iCommType = EDcc;
		iTrkConnData.iConnType = ETrkDcc;	
	}
	else if (!commType.Compare(KXTIComm()))
	{
		iTrkConnData.iCommType = EXti;
		iTrkConnData.iConnType = ETrkXti;	
	}	
	else
	{
		User::Leave(KErrCorrupt);
	}
	
	// skip white space and end of lines
	ch = lexer.Peek();
	while (!lexer.Eos() && (ch.IsSpace() || (ch == '\r') || (ch == '\n')))
	{
		if (ch == '\n')
		{
			iLineNumber++;
		}

		lexer.Inc();
		ch = lexer.Peek();
	}

	// now read in the values
	while (!lexer.Eos())
	{
		// mark the start of the token
		lexer.Mark();

		// read up to the next white space
		ch = lexer.Peek();
		while (!lexer.Eos() && !ch.IsSpace())
		{
			lexer.Inc();
			ch = lexer.Peek();
		}
		
		if (lexer.Eos())
		{
			User::Leave(KErrCorrupt);
		}

		// grab the token	
		TPtrC8 token = lexer.MarkedToken();
		
		// mark the start of the value
		lexer.SkipSpaceAndMark();

		// read up to the next white space or end of line
		ch = lexer.Peek();
		while (!lexer.Eos() && !ch.IsSpace())
		{			
			if ((ch == '\r') || (ch == '\n'))
			{
				User::Leave(KErrCorrupt);
			}
			lexer.Inc();
			ch = lexer.Peek();
		}
		
		// grab the value
		TPtrC8 value = lexer.MarkedToken();
		
		if (token == KPDD)
		{
			iTrkConnData.iPDD.Copy(value);
		}
		else if (token == KLDD)
		{
			iTrkConnData.iLDD.Copy(value);
		}
		else if (token == KCSY)
		{
			iTrkConnData.iCSY.Copy(value);
		}
		else if (token == KPort)
		{
			iTrkConnData.iPortNumber = UintFromDecimalStringL(value);
		}
		else if (token == KRate)
		{
			iTrkConnData.iRate = UintFromDecimalStringL(value);
		}
	    else if (token == KPlugPlay)
	    {
	    	iTrkConnData.iPlugPlay = UintFromDecimalStringL(value);
		}
		else
		{
			User::Leave(KErrCorrupt);
		}

		// skip white space and end of lines
		ch = lexer.Peek();
		while (!lexer.Eos() && (ch.IsSpace() || (ch == '\r') || (ch == '\n')))
		{
			if (ch == '\n')
			{
				iLineNumber++;
			}

			lexer.Inc();
			ch = lexer.Peek();
		}
	}
		
	
	//if the comm type is serial, the actual conn type could be USB or IR
	if (iTrkConnData.iCommType == ESerialComm)
	{		
		TBuf8<32> csyModule;
		csyModule.Copy(iTrkConnData.iCSY);
		csyModule.UpperCase();
		
		if (csyModule == KDefaultUsbCSY)
		{
			iTrkConnData.iConnType = ETrkUsb;	
		}
		else if(csyModule == KDefaultIrCSY)
		{
			iTrkConnData.iConnType = ETrkIr;	
		}
		else
		{
			iTrkConnData.iConnType = ETrkSerial;	
		}
	}
		
	
	iTrkConnData.iDefault = EFalse;
	
	iLoaded = ETrue; //read only once from the file system.
	
	CleanupStack::PopAndDestroy(content);
	CleanupStack::PopAndDestroy(); // file
	CleanupStack::PopAndDestroy(); // fs
}

//
// CTrkEngineModel SaveDataL
//
void CTrkEngineModel::SaveDataL()
{
	RFs fs;
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);

//	RFile file;
//	User::LeaveIfError(file.Open(fs, iInitFilePath, EFileStreamText|EFileRead|EFileWrite));
//	CleanupClosePushL(file);
	RFile file;
	
	if (!iInitFilePath.Size())
	{
		iInitFilePath = KInitFilePath;
		iInitFilePath.Append(KInitFileName);
	}
	
	//check to this if the file is in ROM, 
	//If so, save it in C drive as Trk looks for the ini file in C drive first
	if (iInitFilePath[0] == 'z' || iInitFilePath[0] == 'Z')
	{
		iInitFilePath[0] = 'C';
	}

	TInt err = file.Replace(fs, iInitFilePath, EFileStreamText|EFileWrite);
	if (err != KErrNone)
	{
		User::LeaveIfError(file.Create(fs, iInitFilePath, EFileStreamText|EFileWrite));
	}
	CleanupClosePushL(file);

	TBuf8<1024> fileBuf;
	TBuf<25> buf;
	if (iTrkConnData.iCommType == ESerialComm)
	{	
		fileBuf.Copy(KSerialComm);
		fileBuf.Append(KNewLine);
			
		//write PDD info		
		fileBuf.Append(KPDD);
		fileBuf.Append(KSpace);
		fileBuf.Append(iTrkConnData.iPDD);
		fileBuf.Append(KNewLine);

		//write LDD
		fileBuf.Append(KLDD);
		fileBuf.Append(KSpace);
		fileBuf.Append(iTrkConnData.iLDD);
		fileBuf.Append(KNewLine);
				
		//write CSY
		fileBuf.Append(KCSY);
		fileBuf.Append(KSpace);
		fileBuf.Append(iTrkConnData.iCSY);
		fileBuf.Append(KNewLine);

		//write port number
		_LIT(KPortSetting, "PORT %d\r\n");
		buf.Format(KPortSetting, iTrkConnData.iPortNumber); 	
		fileBuf.Append(buf);
	
		//write baud rate
		_LIT(KRateSetting, "RATE %d");
		buf.Format(KRateSetting, iTrkConnData.iRate); 	
		fileBuf.Append(buf);
		fileBuf.Append(KNewLine);
		//fileData8.Copy(fileData16);
	}
	else if (iTrkConnData.iCommType == EDbgTrcComm)
	{
		fileBuf.Copy(KDbgTrcComm);
		fileBuf.Append(KNewLine);

		//write port number
		_LIT(KPortSetting, "PORT %d\r\n");
		buf.Format(KPortSetting, iTrkConnData.iPortNumber); 	
		fileBuf.Append(buf);
	
		//write baud rate
		_LIT(KRateSetting, "RATE %d");
		buf.Format(KRateSetting, iTrkConnData.iRate); 	
		fileBuf.Append(buf);
		fileBuf.Append(KNewLine);
	}
	else if (iTrkConnData.iCommType == EBtSocket)
	{
		fileBuf.Copy(KBtSocketComm);
		fileBuf.Append(KNewLine);

		//write port number
		_LIT(KPortSetting, "PORT %d\r\n");
		buf.Format(KPortSetting, iTrkConnData.iPortNumber); 	
		fileBuf.Append(buf);
	
		//write baud rate
		_LIT(KRateSetting, "RATE %d");
		buf.Format(KRateSetting, iTrkConnData.iRate); 	
		fileBuf.Append(buf);
		fileBuf.Append(KNewLine);		
	}
	else if(iTrkConnData.iCommType == EDcc)
	{
		fileBuf.Copy(KDCCComm);
		fileBuf.Append(KNewLine);

		//write LDD
		fileBuf.Append(KLDD);
		fileBuf.Append(KSpace);
		fileBuf.Append(KTrkDCCLDD);	
		fileBuf.Append(KNewLine);

		//write port number
		_LIT(KPortSetting, "PORT %d\r\n");
		buf.Format(KPortSetting, KDefaultDCCPort); 	
		fileBuf.Append(buf);
		fileBuf.Append(KNewLine);		
	}
	else if(iTrkConnData.iCommType == EXti)
	{
		fileBuf.Copy(KXTIComm);
		fileBuf.Append(KNewLine);

		//write LDD
		fileBuf.Append(KLDD);
		fileBuf.Append(KSpace);
		fileBuf.Append(KTrkXTILDD);	
		fileBuf.Append(KNewLine);
	}
	else
	{
		User::Leave(KErrCorrupt);
	}
	
    _LIT(KPlugPlaySetting, "PlugPlay %d");
    buf.Format(KPlugPlaySetting, iTrkConnData.iPlugPlay);   
    fileBuf.Append(buf);
	
	file.Write(fileBuf);
	file.Flush();
	
	CleanupStack::PopAndDestroy(); // file
	CleanupStack::PopAndDestroy(); // fs
}

//
// CTrkEngineModel GetConnData
//
TInt CTrkEngineModel::GetConnData(TTrkConnData& aTrkConnData)
{
   
	aTrkConnData = iTrkConnData;
	return iFileReadErr;
}

//
// CTrkEngineModel GetDefaultConnData
//
TInt CTrkEngineModel::GetDefaultConnData(TTrkConnType aConnType, TTrkConnData& aTrkConnData)
{
	switch (aConnType)
	{
		case ETrkUsbDbgTrc:
		{		
			aTrkConnData.iCommType = EDbgTrcComm;
			aTrkConnData.iConnType = ETrkUsbDbgTrc;
			aTrkConnData.iPDD.Copy(KDefaultUsbPDD);
			aTrkConnData.iLDD.Copy(KDefaultUsbLDD);
			aTrkConnData.iCSY.Copy(KDefaultUsbCSY);
			aTrkConnData.iPortNumber = KDefaultUsbPortNumber;
			aTrkConnData.iRate = KDefaultRate;		
			aTrkConnData.iPlugPlay = iTrkConnData.iPlugPlay;
			aTrkConnData.iDefault = ETrue;	
		
			return KErrNone;
		}
		case ETrkXti:
		{
            aTrkConnData.iCommType = EXti;
            aTrkConnData.iConnType = ETrkXti;
            aTrkConnData.iPDD.Copy(KNullDesC);
	        aTrkConnData.iLDD.Copy(KTrkXTILDD);
	        aTrkConnData.iCSY.Copy(KNullDesC);
	       // aTrkConnData.iPortNumber = KDefaultUsbPortNumber;
	       // aTrkConnData.iRate = KDefaultRate;      
	        aTrkConnData.iPlugPlay = ETrue; 
			aTrkConnData.iDefault = ETrue;	
			aTrkConnData.iPlugPlay = iTrkConnData.iPlugPlay;
		
			return KErrNone;
		}
		default:
		{
			return KErrNotFound;	
		}
	}	
}

//
// CTrkEngineModel SaveDataL
//
TInt CTrkEngineModel::SetConnData(TTrkConnData& aTrkConnData)
{
	iTrkConnData = aTrkConnData;
	
	TRAPD(err, SaveDataL());
	
	return err;
}

//
// CTrkEngineModel::GetPlugPlaySettingValue()
//

TBool CTrkEngineModel::GetPlugPlaySettingValue()
{
    if(iTrkConnData.iPlugPlay)
        return ETrue;
    else
        return EFalse;
}
