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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <e32base.h>
#include <utf.h>
#include <swi\launcher.h>
#include "uihandler.h"

int main(int argc, char *argv[], char **/*envp*/)
	{
	
	TInt err = KErrNone;
	
	if(argc!=2)
		{
		printf("Usage:\n");
		printf("test_uninstall [UID in hexadecimal]\n");
		printf("Example:\n");
		printf("test_uninstall 10203040\n");
		exit(KErrNone);
		}
	
	TBuf16<10> desc;
	
	TPtrC8 src((const TUint8*)argv[1]);
	
	CnvUtfConverter::ConvertToUnicodeFromUtf8(desc, src);
	
	TLex conv(desc);
	
	TUint32 id;
	
	err = conv.Val(id,EHex);
	
	if(err==KErrNone)
		{
		TUid uid = TUid::Uid(id);
		
		CUIHandler* uiHandler = CUIHandler::NewL();

		err = Swi::Launcher::Uninstall(*uiHandler, uid);
		
		delete uiHandler;
		}
	
	printf("Return Code: %d\n",err);
	
	exit(err);
	}

