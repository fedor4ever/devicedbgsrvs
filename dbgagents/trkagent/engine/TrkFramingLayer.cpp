/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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


#include "TrkFramingLayer.h"

#ifdef __USE_NEW_DEBUG_API__
#include "trkdispatchlayer2.h"
#else
#include "TrkDispatchLayer.h"
#endif

#include "serframe.h"

_LIT8(kPPP_FLAG, "~");
_LIT8(kPPP_ESCAPE, "}");


//
//
// CTrkFramingLayer implementation
//
//

//
// CTrkFramingLayer constructor
//
CTrkFramingLayer::CTrkFramingLayer(CTrkCommPort* aPort)
	: iPort(aPort),
	  iFramingState(CTrkFramingLayer::eWaiting),
	  iEscape(0),
	  iFCS(0),
	  iSequenceId(0),
	  iOutSequenceId(1)
{
	// check the target endianness
	TUint32	test;
	TUint8 *byte_alias = (TUint8 *)&test;
	
	// Write a specific 4-byte sequence and then read it as
	// a 32-bit word.  Big-endian systems yield one value and
	// little-endian systems yield another.
	
	byte_alias[ 0 ] = 0x12;
	byte_alias[ 1 ] = 0x34;
	byte_alias[ 2 ] = 0x56;
	byte_alias[ 3 ] = 0x78;
	
	if (test == 0x12345678)
		iIsBigEndian = ETrue;
	else
		iIsBigEndian = EFalse;
}

//
// CTrkFramingLayer destructor
//
CTrkFramingLayer::~CTrkFramingLayer()
{
	if (iPort)
	{
		iPort->ClosePort();
		SafeDelete(iPort);
	}
	
	SafeDelete(iLastReply);
}

//
// CTrkFramingLayer::Listen
//
// Start listening for incoming messages from the host debugger
//
void CTrkFramingLayer::Listen(CTrkDispatchLayer *aDispatchLayer)
{
	// Start listening asynchronously for incoming messages
	iDispatchLayer = aDispatchLayer;
	
	iPort->Listen(this);
}

//
// CTrkFramingLayer::StopListening
//
// Stop listening for incoming messages from the host debugger
//
void CTrkFramingLayer::StopListening()
{
	iDispatchLayer = 0;
	iPort->StopListening();
}

//
// CTrkFramingLayer::HandleByte
//
// Handle a single incoming byte with a state machine 
//
void CTrkFramingLayer::HandleByte(TUint8 aByte)
{
	// discard existing escape if needed
	if (iFramingState != eInFrame)
		iEscape = EFalse;

	// handle state machine
	switch (iFramingState)
	{
		case eWaiting:
		{
			// Start of a new packet
			if (PPP_FLAG == aByte)
			{
				iFCS = PPPINITFCS;
				iFramingState = eFound;	// next state
			}
			break;
		}
			
		case eFound:
		{
			if (PPP_FLAG == aByte)  // discard multiple flags
				break;
	
			iFramingState = eInFrame;

			// fall through to eInFrame, receive first char
		}
					
		case eInFrame:
		{
			if (PPP_FLAG == aByte)
			{
				if (iEscape)
				{
					// [..., escape, flag] is bad frame; drop it
					// send a NAK with the error.
					RespondErr(kDSReplyNAK, kDSReplyEscapeError);
					
					// clear the buffer
					DiscardFrame();
				}
				else
				{
					// normal case, good packet
					if (ProcessFrame())
					{
						iDispatchLayer->HandleMsg(iBuffer);
					}
						
					// now start all over again
					DiscardFrame();
				}
				break;
			}
			else if (iEscape)
			{
				aByte ^= PPP_TRANS;		// change character to new code
				iEscape = EFalse;		// no longer escaped character
			}
			else if (PPP_ESCAPE == aByte)
			{
				iEscape = ETrue;		// next character is escaped
				break;
			}

			// make sure our buffer is not full
			if (iBuffer.Length() == iBuffer.MaxLength())
			{
				// overflow.  send out a NAK immediately.  go into an overflow state
				// which will wait for the end of the packet.
				RespondErr(kDSReplyNAK, kDSReplyOverflow);

				iFramingState = eFrameOverflow;
				break;
			}
							
			// append byte to buffer and compute running FCS (checksum)
			iBuffer.Append(aByte);

			iFCS = PPPFCS(iFCS, aByte);
			break;
		}
		
		case eFrameOverflow:
		{
			// overflow occurred.  throw away all characters and wait for a flag
			if (PPP_FLAG == aByte)
			{
				DiscardFrame();
			}
			break;
		}
			
		default:
		{
			User::Panic(_L("Invalid Framing State"), __LINE__);
			break;
		}
	}
}

//
// CTrkFramingLayer::RespondOkL
//
// Send an ACK to the host debugger
//
void CTrkFramingLayer::RespondOkL(const TDesC8& aMsg)
{
	// compose the standard ACK message
	TBuf8<3> reply;
	reply.Zero();
	
	reply.Append(kDSReplyACK);
	reply.Append(iSequenceId);
	reply.Append(kDSReplyNoError);

	// now append the rest of the response if necessary
	HBufC8 *ack = HBufC8::NewLC(aMsg.Length() + 3);
	ack->Des().Copy(aMsg.Ptr(), aMsg.Length());

	TPtr8 ptr(ack->Des());
	ptr.Insert(0, reply);

	// send it out
	TUint ackTries = 3;
	TInt leaveCode = KErrGeneral;
	
	do
	{
		TRAP(leaveCode, SendMsgL(*ack));
		ackTries--;
	} while ((leaveCode != KErrNone) && ackTries > 0);
	
	// now increment the sequence id
	IncrementSequenceId();

	// save this in case we need to resend it
	SafeDelete(iLastReply);
	iLastReply = ack->Des().AllocL();
	
	CleanupStack::PopAndDestroy(ack);
}

//
// CTrkFramingLayer::RespondErr
//
// Send an NAK to the host debugger
//
void CTrkFramingLayer::RespondErr(TUint8 aType, TUint8 aErr)
{
	// compose the standard NAK message
	TBuf8<3> ack;
	TUint ackTries = 3;
	TInt leaveCode = KErrGeneral;
	
	ack.Zero();
	ack.Append(aType);	
	ack.Append(iSequenceId);
	ack.Append(aErr);

	// send it out
	do
	{
		TRAP(leaveCode, SendMsgL(ack));
		ackTries--;
	} while ((leaveCode != KErrNone) && ackTries > 0);

	if (kDSReplySequenceMissing != aErr)
	{
		// now increment the sequence id
		IncrementSequenceId();
	}
	
	// save this in case we need to resend it
	SafeDelete(iLastReply);
	iLastReply = ack.Alloc();
	
	if (!iLastReply)
		User::Panic(_L("Failed to allocate reply buffer"), __LINE__);
}

//
// CTrkFramingLayer::InformEventL
//
// Queue up an event on the target
//
void CTrkFramingLayer::InformEventL(const TDesC8& aMsg)
{
	// compose the message
	TBuf8<1> id;
	id.Zero();
	id.Append(iOutSequenceId);
	IncrementOutSequenceId();

	HBufC8 *msg = HBufC8::NewLC(aMsg.Length() + 1);
	msg->Des().Copy(aMsg.Ptr(), aMsg.Length());

	TPtr8 ptr(msg->Des());
	ptr.Insert(1, id);

	// now send it out
	TUint ackTries = 3;
	TInt leaveCode = KErrGeneral;

	do
	{
		TRAP(leaveCode, SendMsgL(*msg));
		ackTries--;
	} while ((leaveCode != KErrNone) && ackTries > 0);

	CleanupStack::PopAndDestroy(msg);
	
	if (leaveCode != KErrNone)
		User::Leave(leaveCode);
}

//
// CTrkFramingLayer::SendRawMsgL
//
// Frames a message and sends it back to the host debugger
//
void CTrkFramingLayer::SendRawMsgL(const TDesC8& aMsg)
{
	iPort->SendDataL(aMsg);
}


//
// CTrkFramingLayer::SendMsgL
//
// Frames a message and sends it back to the host debugger
//
void CTrkFramingLayer::SendMsgL(const TDesC8& aMsg)
{
	FCSType fcs;
	FCSType acc;

	// allocate room for this message with ample space for escape sequences and framing
	HBufC8 *msg = HBufC8::NewLC(aMsg.Length() + 100);

	TPtr8 ptr(msg->Des());

	// walk through buffer, computing frame check sequence
	// (checksum).
	
	fcs = PPPINITFCS;
	
	for (TInt i=0; i<aMsg.Length(); i++)
		fcs = PPPFCS(fcs, aMsg[i]);

	fcs = fcs ^ PPPCOMPFCS;		// finish by complementing
	
	// send bytes out the comm channel:
	//	[0x7E (frame flag)] [...data...] [FCS:8 or FCS:16 or FCS:32] [0x7E (frame flag)]
	 
	// FLAG
	ptr.Append(kPPP_FLAG);
					
	// DATA
	TBuf8<1> byte;
	
	for (TInt i=0; i<aMsg.Length(); i++)
	{
		byte.Zero();
		byte.Append(aMsg[i]);
		
		// escape if necessary
		if (byte[0] == PPP_FLAG || byte[0] == PPP_ESCAPE)
		{
			ptr.Append(kPPP_ESCAPE);
			byte[0] ^= PPP_TRANS;
		}
		
		ptr.Append(byte);
	}
	
	// FCS always goes out in little endian (LSB, ... , MSB) order	
	acc = fcs;		// accumulator
	
	TInt fcsSize = sizeof(FCSType);
	
	for (TInt i=0; i<fcsSize; i++)
	{
		byte.Zero();
		byte.Append(acc & 0xFF);

		acc >>= 8;

		// escape if necessary
		if (byte[0] == PPP_FLAG || byte[0] == PPP_ESCAPE)
		{
			ptr.Append(kPPP_ESCAPE);
			byte[0] ^= PPP_TRANS;
		}
		
		ptr.Append(byte);
	}
	
	// FLAG	
	ptr.Append(kPPP_FLAG);
	
	iPort->SendDataL(*msg);
	
	CleanupStack::PopAndDestroy(msg);
}

//
// CTrkFramingLayer::DiscardFrame
//
// Trash the existing message and reset the state machine 
//
void CTrkFramingLayer::DiscardFrame()
{
	iBuffer.Zero();
	iFramingState = eWaiting;
}

//
// CTrkFramingLayer::ProcessFrame
//
// Checks a complete message for correct format
//
TBool CTrkFramingLayer::ProcessFrame()
{
	// the buffer contains data in the following form:
	//		[data] [FCS8]
	//	or:
	//		[data] [FCS16]
	//	or:
	//		[data] [FCS32]
	//	
	//	where data is arbitrary length and FCSxx is 1, 2 or 4 bytes

	TInt minSize = 2 + sizeof(FCSType);
	
	if (iBuffer.Length() < minSize)
	{
		// data received is too short
		RespondErr(kDSReplyNAK, kDSReplyPacketSizeError);
		DiscardFrame();
		return EFalse;
	}
	
	// check for FCS
	if (PPPGOODFCS != iFCS)
	{
		// bad checksum (FCS)		
		RespondErr(kDSReplyNAK, kDSReplyBadFCS);
		DiscardFrame();
		return EFalse;
	}
	
	// make sure the sequence id is correct, then increment
	if (iBuffer[TRK_MSG_SID_INDEX] == 0)
	{
		// special case - resets both sequence ids
		iSequenceId = 0;
		iOutSequenceId = 1;		
	}
	else
	{
		// see which id we're dealing with
		if ((iBuffer[0] != kDSReplyACK) && (iBuffer[0] != kDSReplyNAK))
		{
			// a command from the host 
			if (iBuffer[TRK_MSG_SID_INDEX] == iSequenceId)
			{
				// got the expected sequence id - do nothing
			}
			else if (iBuffer[TRK_MSG_SID_INDEX] == (iSequenceId - 1))
			{
				// we need to resend the last reply
				TUint ackTries = 3;
				TInt leaveCode = KErrGeneral;

				do
				{
					TRAP(leaveCode, SendMsgL(*iLastReply));
					ackTries--;
				} while ((leaveCode != KErrNone) && ackTries > 0);

				return EFalse;
			}
			else
			{
				// bad sequence id
				RespondErr(kDSReplyNAK, kDSReplySequenceMissing);
				DiscardFrame();
				return EFalse;
			}
		}
	}

	// trim back packet, eliminating FCS
	iBuffer.Delete(iBuffer.Length()-1, 1);

	// remove sequence id
	iBuffer.Delete(1, 1);
	
	return ETrue;
}
