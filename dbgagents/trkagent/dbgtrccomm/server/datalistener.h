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


#ifndef __DATALISTENER_H__
#define __DATALISTENER_H__


#define MAX_BUF_SIZE 0x880 


class MDataListener
{
    public:
        
        virtual void DataAvailable(TDesC8& aMsgData, TUint aLength) = 0;
        
        virtual void DataWriteComplete(TInt aErrCode) = 0;
        
      
};

#endif //__DATALISTENER_H__
