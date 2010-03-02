/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies). 
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <e32base.h>

int main(int argc, char *argv[], char **/*envp*/)
	{
	
	TInt64 handleNumber;
	TInt err = KErrNone;
	
	if(argc!=2)
		{
		printf("Usage:\n");
		printf("test_kill [pid]\n");
		exit(KErrNone); 
		}
	
	TBuf8<10> desc;
	TInt size = strlen(argv[1]);
	desc.Copy((TUint8 *)argv[1],size);
	
	TLex8 conv(desc);
	
	err = conv.Val(handleNumber);
	
	if(err==KErrNone)
		{
		RProcess newProcess;
		err = newProcess.Open(TProcessId(handleNumber));
		if(err==KErrNone)
			{
			newProcess.Kill(1);
			}
		newProcess.Close();	
		}
	
	printf("Return Code: %d\n",err);
	
	return err;
	}

