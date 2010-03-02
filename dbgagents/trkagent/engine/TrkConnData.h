/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
#ifndef __TRKCONNDATA_H__
#define __TRKCONNDATA_H__


_LIT8(KNewLine, "\r\n");
_LIT8(KSpace, " ");

_LIT8(KSerialComm,   "[SERIALCOMM]");
_LIT8(KBtSocketComm, "[BTSOCKCOMM]");
_LIT8(KDCCComm, 	 "[DCCIO_COMM]");
_LIT8(KXTIComm, 	 "[XTIIO_COMM]");
_LIT8(KDbgTrcComm,   "[DBGTRCCOMM]");

_LIT8(KPDD, "PDD");
_LIT8(KLDD, "LDD");
_LIT8(KCSY, "CSY");
_LIT8(KPort, "PORT");
_LIT8(KRate, "RATE");
_LIT8(KPlugPlay, "PlugPlay");

//
// Default comm values
//
#ifdef __WINS__
	_LIT(KDefaultPDD, "ECDRV");
#else
	_LIT(KDefaultPDD, "EUART%d");
#endif
	_LIT(KDefaultLDD, "ECOMM");
	_LIT(KDefaultCSY, "ECUART");

_LIT(KPort0, "0");
_LIT(KPort1, "1");

#define KDefaultPortNumber		0
#define KDefaultUsbPortNumber	1
#define KDefaultRate			115200


//default IR settings
_LIT8(KDefaultSerialPDD, "EUART");
_LIT8(KDefaultSerialLDD, "ECOMM");
_LIT8(KDefaultSerialCSY, "ECUART");
_LIT8(KDefaultSerialPort, "0");

//default USB settings
_LIT8(KDefaultUsbPDD, "NONE");
_LIT8(KDefaultUsbLDD, "EUSBC");
_LIT8(KDefaultUsbCSY, "ECACM");
_LIT8(KDefaultUsbPort, "1");

//default IR settings
_LIT8(KDefaultIrPDD, "EUART");
_LIT8(KDefaultIrLDD, "ECOMM");
_LIT8(KDefaultIrCSY, "IRCOMM");
_LIT8(KDefaultIrPort, "0");

//default BT settings
_LIT8(KDefaultBtPort, "1");

//default DCC settings
_LIT8(KTrkDCCLDD, "trkdccdriver");
#define KDefaultDCCPort 42

//default Xti settings
_LIT8(KTrkXTILDD, "trkxtidriver");


#ifdef __UIQ_BUILD__
_LIT(KInitFilePath, "C:\\Media Files\\document\\");
#else
_LIT(KInitFilePath, "C:\\");
#endif
_LIT(KInitFileName, "trk.ini");

_LIT(KStatusConnecting,"Status: Connecting ...");
_LIT(KStatusConnected, "Status: Connected");
_LIT(KStatusDisconnected, "Status: Not connected");
_LIT(KStatusError, "Status: Not connected");

_LIT(KCmdLineConnTypeUsb, "-usb");
_LIT(KCmdLineConnTypeXti, "-xti");


//This represents the underlying comms transport that is used for
//accessing the actual connection.
enum TTrkCommType
{
	ESerialComm = 0,
	EDbgTrcComm,
	EBtSocket,
	ETcpSocket,
	EUsb,
	EDcc,
	EXti,
	ETrkCommInvalid
};

//This represents the actual connection type that is shown to the user through the GUI
enum TTrkConnType
{
	ETrkUsbDbgTrc = 0,
	ETrkBt,
	ETrkXti,
	ETrkDcc,
	ETrkSerial,
	ETrkUsb,
	ETrkIr,
	ETrkConnInvalid
};

enum TTrkConnStatus
{
    ETrkNotConnected = 1,
    ETrkConnecting,
    ETrkConnected,
    ETrkConnectionError
};
//
// Forward declarations
//

class TTrkConnData
{
public:
	TTrkCommType iCommType;
	TTrkConnType iConnType;
	
	TBuf<KMaxFileName> iPDD;
	TBuf<KMaxFileName> iLDD;
	TBuf<KMaxFileName> iCSY;
	TUint  iPortNumber;
	TUint  iRate;
	TBool iDefault;
	TUint iPlugPlay;
};

#endif //__TRKCONNDATA_H__
