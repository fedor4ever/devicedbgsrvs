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


#include <e32cons.h>
#include <e32base.h>

#include "logging.h"
#include "portreader.h"
#include "portwriter.h"
#include "dbgtrcportmgr.h"

//default USB settings
_LIT(KDefaultUsbPDD, "NONE");
_LIT(KDefaultUsbLDD, "EUSBC");
_LIT(KDefaultUsbCSY, "ECACM");
#define KDefaultUsbPort 1

// uncomment the line below for testing with serial port on H4 or H2 board
//#define SERIAL

//
// Static helper functions
//
static void BuildPortName(const TDesC& aModuleName, TUint aUnit, TDes& aCompleteName)
{
	aCompleteName.Zero();
	aCompleteName.Append(aModuleName);
	aCompleteName.AppendFill(':', 2);
	aCompleteName.AppendNum(aUnit);
}

static TBps TBpsRateFromDbgTrcBaudRate(TBaudRates aRate)
{
	switch (aRate)
	{
		case EBaud50:      return EBps50;
		case EBaud75:      return EBps75;
		case EBaud110:     return EBps110;
		case EBaud134:     return EBps134;
		case EBaud150:     return EBps150;
		case EBaud300:     return EBps300;
		case EBaud600:     return EBps600;
		case EBaud1200:    return EBps1200;
		case EBaud1800:    return EBps1800;
		case EBaud2000:    return EBps2000;
		case EBaud2400:    return EBps2400;
		case EBaud3600:    return EBps3600;
		case EBaud4800:    return EBps4800;
		case EBaud7200:    return EBps7200;
		case EBaud9600:    return EBps9600;
		case EBaud19200:   return EBps19200;
		case EBaud38400:   return EBps38400;
		case EBaud57600:   return EBps57600;
		case EBaud115200:  return EBps115200;
		case EBaud230400:  return EBps230400;
		case EBaud460800:  return EBps460800;
		case EBaud576000:  return EBps576000;
		case EBaud1152000: return EBps1152000;
		case EBaud4000000: return EBps4000000;
		default:  		   return EBpsAutobaud;
	}
}


CDbgTrcPortMgr*	CDbgTrcPortMgr::NewL()
{
	LOG_MSG("CDbgTrcPortMgr::NewL");

	CDbgTrcPortMgr* self = new(ELeave) CDbgTrcPortMgr;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CDbgTrcPortMgr::~CDbgTrcPortMgr()
{
	LOG_MSG("CDbgTrcPortMgr::~CDbgTrcPortMgr");

	SafeDelete(iReader);
	SafeDelete(iWriter);
}

void CDbgTrcPortMgr::ConstructL()
{
	LOG_MSG("CDbgTrcPortMgr::ConstructL");

	iReader = NULL;
	iWriter = NULL;
	
#ifdef SERIAL // for testing with serial on H4 or H2 board.
	iPDD.Copy(_L("EUART1"));
	iLDD.Copy(_L("ECOMM"));
	iCSY.Copy(_L("ECUART"));
	iPortNumber = 3;
#else
	//set the default ACM port settings.	
	iPDD.Copy(KDefaultUsbPDD);
	iLDD.Copy(KDefaultUsbLDD);
	iCSY.Copy(KDefaultUsbCSY);
	iPortNumber = KDefaultUsbPort;	
#endif

	iRate = EBaud115200;
	
	iConnected = EFalse;
	iActiveConnections = 0;
	iServerStarted = EFalse;
	
	iAcmConfig.iBaudRate = iRate;
	iAcmConfig.iPortNumber = iPortNumber;
}

void CDbgTrcPortMgr::GetPortConfig(TDes8& aDes)
{
	LOG_MSG("CDbgTrcPortMgr::GetPortConfig");

	TPtrC8 cfg((const TUint8*)&iAcmConfig, sizeof(iAcmConfig));
	aDes.Copy(cfg);	
}

TInt CDbgTrcPortMgr::SetPortConfig(const TDesC8& aDes)
{
	LOG_MSG("CDbgTrcPortMgr::SetPortConfig");

	if (iConnected)
		return KErrInUse;
		
	if (aDes.Length()<(TInt)sizeof(TAcmConfigV01))
		return KErrGeneral;
	TAcmConfigV01 config(*(TAcmConfigV01*)aDes.Ptr());
	
	iPortNumber = config.iPortNumber;
	iRate = config.iBaudRate;
	
	iAcmConfig.iBaudRate = iRate;
	iAcmConfig.iPortNumber = iPortNumber;
	
	return KErrNone;
}


//
// CDbgTrcPortMgr::OpenPortL
//
// Open the ACM port
//
TInt CDbgTrcPortMgr::OpenPortL()
{
	LOG_MSG("CDbgTrcPortMgr::OpenPort");

	TInt error = KErrNone;
	
	if (iConnected)
	{
		iActiveConnections++;
		return error; //return KErrNone if the connection is already opened by another client.
	}

	if (!iServerStarted)
	{
		StartC32();
		error = iServer.Connect();
		if (error == KErrNone)
		{
			iServerStarted = ETrue;
			LOG_MSG("C32 Server started");
		}
	}											  
		
	if (iServerStarted)
	{
		error = iServer.LoadCommModule(iCSY);
		if (error == KErrNone)
		{
			LOG_MSG("CSY Loaded:");

			TInt ports = 0;
			error = iServer.NumPorts(ports);
			
			if (error == KErrNone)
			{
				LOG_MSG2("Number of ports: %d", ports);

				// make sure the unit number is in range
				TSerialInfo serialInfo;

				error = KErrNotFound;
				
				TBuf<32> csyModule;
				csyModule = iCSY;
				csyModule.LowerCase();
				for (TInt i=0; i<ports && error; i++)
				{
					TBuf<32> module;
					TInt err1 = iServer.GetPortInfo(i, module, serialInfo);
					
					// come out of the loop and error out if we are not able
					// to get the port info.
					if (err1 != KErrNone) 
						break;
										   
					module.LowerCase();
					LOG_MSG("Module name: ");

					if (!module.Compare(csyModule))
					{
						error = KErrNone;
						LOG_MSG("CSY Module matched");
					}
				}
				
				if (error == KErrNone)
				{		
					LOG_MSG("CSY Module found");			
					if (iPDD.Compare(KDefaultUsbPDD))
						error = User::LoadPhysicalDevice(iPDD);
					
					if (error == KErrNone || error == KErrAlreadyExists)
					{
						error = User::LoadLogicalDevice(iLDD);
						if (error == KErrNone || error == KErrAlreadyExists)
						{
							error = KErrNone;
							TBuf<KMaxPortName+4> portName;
							BuildPortName(serialInfo.iName, iPortNumber, portName);
							
							error = iPort.Open(iServer, portName, ECommExclusive, ECommRoleDTE);
							if (error != KErrNone)
							{
								LOG_MSG("Unable to open port in DTE mode");
								
								error = iPort.Open(iServer, portName, ECommExclusive, ECommRoleDCE);												  
							}
							
							if (error == KErrNone)
							{
								LOG_MSG("Port opened");
								
								// Configure physical and logical characteristics
								TCommConfig config;
								iPort.Config(config);
								config().iRate = TBpsRateFromDbgTrcBaudRate(iRate);
								config().iParity = EParityNone;
								config().iDataBits = EData8;
								config().iStopBits = EStop1;
								config().iFifo = EFifoEnable;

								config().iHandshake = 0;	// no flow control at all

								error = iPort.SetConfig(config);
								
								if (error == KErrNone)
								{
									LOG_MSG("Port configuration set");

									//iPort.SetReceiveBufferLength(MAX_BUF_SIZE);									
									iConnected = ETrue;
									//reset the rx and tx buffers just in case 
									//if there is any stale data lying around from the previous debug sessions
									iPort.ResetBuffers(); 
									
									// now increment the active connections counter
									iActiveConnections++;
									
									// now create the port reader and writer.
									iReader = CPortReader::NewL(iPort);
									iWriter = CPortWriter::NewL(iPort);																
								}							
							}
						}
					}
				}
			}
		}
	}
	
	if (iServerStarted && !iConnected)  
	{
		iServer.Close();
		iServerStarted = EFalse;
	}	

	return error;
}


//
// CDbgTrcPortMgr::ClosePort
//
// Close the ACM port
//
TInt CDbgTrcPortMgr::ClosePort()
{	
	// now decrement the active connections counter
	if (iActiveConnections > 0)	
		iActiveConnections--; 

	if (iActiveConnections == 0) // close the actual port only when the number of active connections is 0
	{
		SafeDelete(iReader);
		SafeDelete(iWriter);

		if (iConnected)
		{
			iPort.Close();
			iConnected = EFalse;
		}

		if (iServerStarted)
		{
			iServer.Close();
			iServerStarted = EFalse;
		}		
	}
	
	return KErrNone;
}

//
// CDbgTrcPortMgr::GetPortReader
//
//
CPortReader* CDbgTrcPortMgr::GetPortReader()
{
	if (iConnected)
		return iReader;
	
	return NULL;
}

//
// CDbgTrcPortMgr::GetPortReader
//
//
CPortWriter* CDbgTrcPortMgr::GetPortWriter()
{
	if (iConnected)
		return iWriter;
	
	return NULL;
}


